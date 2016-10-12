#include "advisor.hpp"
#include <ctype.h>
#include <assert.h>
#include <string.h>  // for strerror()

/**
 * @page Advisor_Notes Advisor Notes
 *
 * Advisor uses AFile_Stream as its file handler, which in turn
 * uses a FILE* handle either passed to it or opened using fopen()
 * with a filename.
 *
 * The issue, that may not be significant, is that fopen() makes
 * a heap allocation (568 bytes today (2015-06-25)).
 *
 * This is actually not necessary: Advisor uses a limited line
 * length that could be used with a lower-level file functions
 * open() and read().  I won't need write().  I could probably
 * just do my own buffering since I don't need to write.
 *
 * @todo Create a new class AFile_Handle : public I_Afile
 * that uses lower-level open() and read() commands forom the
 * <fcntl.h> library.
 */

/**
 * @brief If pos<0, rewind to start of file, otherwise set pointer to pos.
 */
void I_AFile::set_position(long int pos)
{
   if (pos<0)
      this->rewind();
   else
      _set_position(pos);
}

/**
 * @brief Saves file position just before read in order to restore later.
 *
 * This methods saves the file position just before reading so Advisor
 * can restore the file position and reread the line so the state of
 * the Advisor object with the same tag loaded.
 */
char *I_AFile::read_line(char *buff, int buffsize)
{
   m_cur_line_pos = _get_position();
   return _read_line(buff, buffsize);
}


AFile_Stream::~AFile_Stream()
{
   if (m_pfile)
   {
      fclose(m_pfile);
      m_pfile = nullptr;
   }
}


bool AFile_Stream::is_open(void)               { return m_pfile!=NULL; }
void AFile_Stream::rewind(void)                { ::rewind(m_pfile); }


long int AFile_Stream::_get_position(void)     { return ftell(m_pfile); }
void AFile_Stream::_set_position(long int pos) { fseek(m_pfile, pos, SEEK_SET); }
char *AFile_Stream::_read_line(char *buff, int buffsize)
{
   return fgets(buff, buffsize, m_pfile);
}


/**
 * @brief Open file, with error detection and exception throwing.
 *
 * The purpose of this function is to take this processing out of the
 * template function to minimize its size.
 */
int BaseBuffer::open_file(const char* path)
{
   int handle = open(path, O_RDONLY);
   if (handle<0)
   {
      fprintf(stderr, "Unable to open \"%s\": %s\n", path, strerror(errno));
      throw std::runtime_error("failed to open file");
   }
   return handle;
}

/**
 * @brief Uses fstat() to get file size to avoid disturbing the file pointer.
 *
 * The value this function returns helps the class determine whether more bytes
 * are available for reading, preventing unproductive reads that invalidate the
 * buffer contents.
 *
 * This could have been private because it's only used in the class, but there's
 * no safety issue with it, so I left it public.
 */
off_t BaseBuffer::file_size(int handle)
{
   struct stat st;
   if (fstat(handle, &st))
   {
      fputs("Buffer::file_size fstat fails: ", stderr);
      fputs(strerror(errno), stderr);
      fputc('\n', stderr);
      throw std::runtime_error("file-sizing error");
   }
   return st.st_size;
}

/** Begin using buffer, set in_use flag and get file size. */
void BaseBuffer::start(int handle)
{
   m_buffer_file_size = file_size(handle);
   m_in_use = true;
   read_from_file(handle);
}

/**
 * @brief Copies the next line, including the newline, into @p buff,
 *        up to @p buffsize characters.
 *
 * This function copies characters from the class buffer into @p buff,
 * only reading from the disk file if the end of the line isn't found
 * in the current buffer.
 *
 * The function is meant to imitate fgets(), so it likewise copies the
 * newline, if found, and adds a terminating '\0' after the copied
 * string.
 */
char* BaseBuffer::read_line(int handle, char *buff, int buffsize)
{
   // Early termination if we've reached the end-of-file:
   if (eof())
      return nullptr;
   
   char *ptr = buff -1;           // start at minus-one so the loop can start
                                  // with an increment, and ptr always points
                                  // at the last-copied character.  Then, when
                                  // the loop ends, it will be appropriate to
                                  // do a final increment for the \0.
   
   char *end = buff + buffsize-1; // leave room for a terminating \0.

   // Continue while both buffer pointers are within their ranges.
   while (ptr<end && m_buffer_ptr<m_buffer_end)
   {
      // Prefix increment for ptr so it points at the last-copied
      // character, postfix increment for m_buffer_ptr so it points
      // to the next character.  This is so we can efficiently compare
      // both values in case of a \n\r or \r\n newline character pair.
      *++ptr = *m_buffer_ptr++;

      // If the buffer pointer is now invalid, get next characters, if any
      // so it points to the next character after *ptr.
      if (m_buffer_ptr >= m_buffer_end)
      {
         // If buffer exhausted, get more characters.
         // If no more characters, increment and break
         // to terminate string an return as final line.
         if (!read_from_file(handle))
            break;
      }

      // Check for one- or two-character newline, consuming the second newline
      // character if found.  In both cases, use the postfix increment for
      // m_buffer_ptr so it points past the newline char(s) when done, but use
      // a prefix increment for ptr because we earlier delayed the increment
      // when copying so as to make available both characters for detecting
      // the full newline. When we leave the loop, ptr should point to the last
      // character copied.
      if (*ptr=='\r')
      {
         if (*m_buffer_ptr=='\n')
            *++ptr = *m_buffer_ptr++;
         break;
      }
      else if (*ptr=='\n')
      {
         if (*m_buffer_ptr=='\r')
            *++ptr = *m_buffer_ptr++;
         break;
      }
   }

   *++ptr = '\0';
   return buff;
}

/**
 * @brief Returns the object to a state where next bytes to be read are from
 *        file position @p pos.
 *
 * Allows SpecsReader to retrieve data from the specs file once it's determined
 * that it's necessary.  
 *
 * For efficiency, this function first checks if the requested data is already
 * in the buffer.  If so, it simply sets buffer_ptr to the appropriate position.
 * Subsequent calls to read_line will consume the bytes available and call
 * BaseBuffer::read_from_file(int) when the buffer is exhausted.
 */
void BaseBuffer::set_position(int handle, off_t pos)
{
   if (pos >= m_buffer_file_size)
      throw std::runtime_error("attempt to read past end-of-file");
   
   if (file_position_in_buffer(pos))
   {
      set_buffer_ptr_to_file_position(pos);
   }
   else
   {
      off_t newpos = lseek(handle, pos, SEEK_SET);
      if (newpos<0)
      {
         fputs("Buffer::set_position lseek error: ", stderr);
         fputs(strerror(errno), stderr);
         fputc('\n', stderr);
         throw std::runtime_error("seek error");
      }
      else
         // This function will set buffer pointers appropriately
         read_from_file(handle);
   }
}

/**
 * @brief To be called when finished with the buffer.
 *
 * This function is primarily to return a static instance of the class
 * to an unused state in order for a subsequent call.
 */
void BaseBuffer::finish(void)
{
   m_buffer_end = m_buffer_ptr = nullptr;
   m_buffer_file_pos = m_buffer_file_size = 0;
   m_in_use = false;
}



/**
 * @brief Reads up to <bufflen> bytes into the buffer, updating pointers.
 *
 * This private function is used to access the disk file when appropriate.
 * The other functions of this class will determine if and when this is
 * necessary.
 *
 * @param handle A handle to an open file returned from open()
 * @return true if more characters are available, false if reached end-of-file.
 */
bool BaseBuffer::read_from_file(int handle)
{
   m_buffer_end = m_buffer_ptr = m_buffer;
   m_buffer_file_pos = lseek(handle, 0, SEEK_CUR);
   
   size_t count = ::read(handle, m_buffer, m_buffer_size);
   if (count<0)
   {
      if (errno==EINTR)
      {
         fputs("***Buffer::read interrupted error: how to test and recover?\n",
               stderr);
      }
      else if (errno)
      {
         fputs("Buffer::read::read error: ", stderr);
         fputs(strerror(errno), stderr);
         fputc('\n', stderr);
      }
      else
         fputs("Buffer::read, unidentified read error.\n", stderr);
      
      throw std::runtime_error("read error");
   }
   else
      m_buffer_end += count;

   return count>0;
}



/** Static instance should suffice for all... */
Buffer<1024> static_buffer;
BaseBuffer* AFile_Handle::s_buffer = &static_buffer;

AFile_Handle::~AFile_Handle() { }

void AFile_Handle::build_new_buffer(int handle, IGeneric_Callback<AFile_Handle> &callback)
{
   Buffer<1024> buff;
   AFile_Handle afh(handle, &buff);
   callback(afh);
}


//const char Advisor::s_continued_tag[2] = { 31, 0 };
const char Advisor::s_continued_tag[2] = { '*', 0 };

Advisor::Advisor(I_AFile &afile)
   : m_file(afile), m_buffer(),
     m_line_continuing(false),
     m_cur_tag(nullptr), m_cur_value(nullptr),
     m_len_tag(0), m_len_value(0),
     m_cur_level(0)
{
   get_next_line();
}


/**
 * String of line terminating characters for scan_buffer_for_ends_and_level()
 * 
 * The colon is a terminating character until the first colon is found.
 * Once the colon is found, cur_termchars is incremented so strchr doesn't
 * consider the colon for a match.
 */
const static char line_termination_chars[] = ":#\r\n";

/**
 * @brief Marks beginning and end of content in the buffer, also handling
 *        continuing lines.
 *
 * Scans m_buffer to find the beginning of content (first non-whitespace)
 * and the end of the content (last non-whitespace).  If a comment token (#)
 * is found, the line is terminated at the last non-whitespce character
 * preceding the terminator.
 *
 * When this function is done, m_cur_line points to the beginning of the
 * content and m_cur_level indicates the number of whitespace characters
 * preceded the content.  m_cur_level==-1 if there is no content on the
 * current line (and m_cur_line will likewise by NULL).
 *
 * **Continuing Lines**  as of 2016-10-10
 *
 * Advisor now detects the intention to continue a line, indicated by
 * terminating a line with a backslash ('|').  The line following a
 * continuing line will have an artificial, untypeable name, and the
 * entire contents of the line (less trimmed leading- and trailing- spaces)
 * will be the value of the continued line.
 
 */
void Advisor::scan_buffer_for_ends_and_level(void)
{
   char *last_non_white = nullptr;
   char *curpos = m_buffer;
   m_cur_tag = m_cur_value = nullptr;
   m_len_tag = m_len_value = 0;
   m_cur_level = 0;

   // keep pointers to working parts to avoid conditionals
   const char *cur_termchars = line_termination_chars;
   const char **ptr_cur_string = &m_cur_tag;
   int  *ptr_cur_len = &m_len_tag;

   auto start_value_processing = [this, &cur_termchars, &ptr_cur_string,
                                  &ptr_cur_len, &last_non_white, &curpos]()
      {
         ++cur_termchars;
         ptr_cur_string = &m_cur_value;
         ptr_cur_len = &m_len_value;
         last_non_white = nullptr;
      };

   if (m_line_continuing)
   {
      m_line_continuing = false;

      // Fake having already parsed the tag:
      m_cur_tag = s_continued_tag;
      m_len_tag = 1;
      m_cur_level = 256;

      start_value_processing();
   }

   const char *terminator;
      
   while (*curpos)
   {
      if ((terminator=strchr(cur_termchars, *curpos)))
      {
         // A terminator only matters if characters have been saved,
         // that is, if last_non_white has a value.
         if (last_non_white)
         {
            *(last_non_white+1) = '\0';
            *ptr_cur_len = last_non_white - *ptr_cur_string + 1;
         }

         // if a colon, reset variables and continue:
         // Note: use *terminator because the *curpos is likely to
         // have been overwritten with a \0:
         if (*terminator==':')
         {
            start_value_processing();
            // increment char pointer, then bypass remainer of loop:
            ++curpos;
         }
         else  // if line-terminating character:
            break;
      }

      if (isspace(*curpos))
      {
         // count spaces before the first character
         if (!m_cur_tag)
            ++m_cur_level;
      }
      else
      {
         // save last_non_white to later trim trailing whitespace:
         last_non_white = curpos;
         
         // If it's the first non-white character, save its
         // position as the start of the current string:
         if (!*ptr_cur_string)
            *ptr_cur_string = curpos;
      }
      ++curpos;
   }

   // If terminating character is '\0', save length of current string.
   if (*ptr_cur_string && !*ptr_cur_len)
   {
      // Note that we add 1 here, unlike above when we don't want to
      // include the terminating character.
      *ptr_cur_len = last_non_white - *ptr_cur_string + 1;
   }

   if (!m_cur_tag || *m_cur_tag=='\0')
      m_cur_level = -1;

   if (last_non_white && *last_non_white == '\\')
      m_line_continuing = true;
}

bool Advisor::get_next_line(void)
{
   char *line;
   
   // Gets the next non-empty line from the file until
   // the end of the file.
//   while ((line=fgets(m_buffer, sizeof(m_buffer), m_file)))
   while ((line=m_file.read_line(m_buffer, s_bufflen )))
   {
      scan_buffer_for_ends_and_level();

      if (m_cur_level>=0)
         return true;
   }

   // If we get to the end of the file, m_cur_level goes negative again:
   m_cur_level = -1;
   return false;
}

/**
 * @brief Set position and read the next line into the buffer.
 */
void Advisor::restore_state(long int pos)
{
   m_file.set_position(pos);
   get_next_line();
}

/**
 * @brief Get the first child under the current line.
 *
 * Get the first following line whose level is greater than the current line.
 *
 * If the level of the first following line is less than or equal to the
 * current level, the position is reset to the starting line and returns false.
 * Otherwise, the file is left alone (ready to get the value or to get the next
 * line) and this method returns true.
 */
bool Advisor::first_child(void)
{
   int parent_level = level();
   long int pos =  m_file.get_position();

   if (get_next_line() && level()>parent_level)
      return true;
   else
   {
      restore_state(pos);
      return false;
   }
}

/**
 * @brief Get next line whose level matches the current line.
 *
 * Returns true if an appropriate line is found, false othewise.
 */
bool Advisor::next_sibling(void)
{
   int sib_level = level();
   while(get_next_line())
   {
      if (level()==sib_level)
         return true;
      else if (level() < sib_level)
         return false;
   }
   return false;
}


/**
 * @brief Position file pointer to named section
 *
 * This function positions the file pointer to the line just
 * following a section title line named *name*, where *name*
 * begins at column *level*
 */
bool Advisor::seek_tag(const char *name, int level, bool start_at_top)
{
   assert(!(start_at_top && level>0));
   
   if (start_at_top && m_file.is_open())
      this->rewind();

   for (; !end(); get_next_line())
   {
      if (this->level()==level)
      {
         if (is_equal_to(name))
            return true;
      }
      
      // If found another section, it's not in the current section: abort.
      if (this->level() < level)
         break;
   }

   return false;
}

bool Advisor::seek_child(const char *name)
{
   long int pos = get_position();
   if (first_child())
   {
      do
      {
         if (is_equal_to(name))
            return true;
      }
      while (next_sibling());
   }

   restore_state(pos);
   return false;
}


Advisor_iterator::Advisor_iterator(Advisor &adv)
   : m_advisor(adv), m_top(), m_section_level(-1)
{
   save_position();
   m_section_level = adv.level();
   adv.get_next_line();
}


