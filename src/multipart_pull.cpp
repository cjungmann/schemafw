// -*- compile-command: "g++ -std=c++11 -Wno-pmf-conversions -Wall -Werror -Weffc++ -pedantic -ggdb -DINCLUDE_MULTIPART_PULL_MAIN `mysql_config --cflags` -o multipart_pull multipart_pull.cpp `mysql_config --libs`" -*-

#include <ctype.h>
#include <unistd.h>       // fork()
#include <sys/wait.h>     // wait();
#include <fcntl.h>        // open() constant values

#include "multipart_pull.hpp"

uint16_t Multipart_Pull::s_END_FIELD = 2573; // "\r\n"
uint16_t Multipart_Pull::s_END_FORM = 11565; // "--"

/** @brief Constructor */
Multipart_Pull::Multipart_Pull(IStreamer &s)
   : m_workarea(),
     m_end_workarea(nullptr),
     m_str(s),
     m_boundary(nullptr),
     m_boundary_end(nullptr),
     m_boundary_length(-1),
     m_boundary_ptr(nullptr),
     m_end_boundary_chunk(nullptr),
     m_match_breaker(-1),
     m_buffer(nullptr),
     m_end_of_field(true),
     m_end_of_form(false),
     m_field_cdispo(nullptr),
     m_field_name(nullptr),
     m_field_fname(nullptr),
     m_field_ctype(nullptr)
{
   m_end_workarea = m_workarea + sizeof(m_workarea) - 10;
   // int sz = sizeof(m_workarea);
   // m_end_workarea = m_workarea + sz;
   read_initial_boundary_and_prepare_buffers();
   reset_field_heads();
}

// const mimetypes_map * const Multipart_Pull::s_mtypes_map[] = {
//    {0, "application/vnd.ms-excel"},
//    {1, "application/vnd.oasis.opendocument.spreadsheet"},
//    nullptr
// };

/**
 * @brief Save the boundary string to the workarea and setup to use
 *        the rest as a buffer.
 *
 * The first line of a multipart_form is the boundary string.  We read
 * it and save it to the beginning of the workarea, then keep a pointer
 * to the character immediately following to be the memory for buffering
 * the rest of the reads.
 */
void Multipart_Pull::read_initial_boundary_and_prepare_buffers(void)
{
   int c;
   char *pstr = m_workarea;
   m_boundary = m_boundary_ptr = m_workarea;

   // Limiter to leave room for \0 at end of boundary string and
   // one character afterwards:
   char *limit = m_end_workarea - 2;

   // After the initial boundary string, all boundary strings are
   // preceeded by a /r/n:
   *pstr++ = '\r';
   *pstr++ = '\n';

   while (EOF!=(c=m_str.getc()) && pstr < limit)
   {
      if (c=='\r')
      {
         // save useful pointers:
         m_boundary_end = pstr;
         *pstr++ = '\0';
         m_buffer = pstr;

         // dispose of newline:
         assert((c=m_str.getc())=='\n');
         break;
      }
      else
         *pstr++ = c;
   }

   // // Debug code to confirm that command line and web server get same data
   // ifputs("Attempting to open initizlize_mpp.txt.\n", stderr);
   // int fh = open("/tmp/initialize_mpp.txt",
   //               O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
   // if (fh)
   // {
   //    ifputs("Opened initialize_mpp.txt.\n", stderr);
   //    char msg_boundary[] = "boundary = ";
   //    write(fh, msg_boundary, strlen(msg_boundary));
   //    if (errno)
   //       ifputs(strerror(errno), stderr);
   //    write(fh, m_workarea, pstr-m_workarea);
   //    close(fh);
   // }
   
   m_boundary_length = m_buffer - m_boundary;
}

/**
 * @brief Thread start routine for get_csv_file_handle (and perhaps others)
 *
 * @param data Cast this to tf_payload in order to access a Multipart_Pull object
 *             and a file handle with which to write the current field's contents.
 *
 * This function simply reads from a Multipart_Pull object and writes it to
 * the file handle.  In get_csv_file_handle, the file handle part of @p data.
 */
void * Multipart_Pull::pthread_create_start_routine(void *data)
{
   fhandle_payload *pl = static_cast<fhandle_payload*>(data);
   Multipart_Pull  &mpp = pl->mpp;
   int             handle = pl->thread_write();
   size_t     r = 1;
   int        i;
   char       c;

   while (EOF!=(i=mpp.getc()))
   {
      if (r==1)
      {
         c = static_cast<char>(i);
         r=write(handle, &c, 1);
         if (r!=1)
            fprintf(stderr,
                    "Wrote %lu character, expected to write 1 (%s)\n",
                    r, strerror(errno));
      }
   }

   close(handle);
   mpp.flag_field_complete();

   return nullptr;
}



/**
 * @brief Reset all field head pointers to nullptr and reset m_buffer pointer.
 */
void Multipart_Pull::reset_field_heads(void)
{
   m_field_cdispo =
      m_field_name =
      m_field_fname =
      m_field_ctype = nullptr;
   
   m_boundary_ptr = m_boundary;
   m_end_boundary_chunk = nullptr;
   m_match_breaker = -1;
   
   m_buffer = const_cast<char*>(m_boundary_end+1);
}

/**
 * @brief Read field header line(s) to extract certain bits of data.
 *
 * This function reads the header lines, extracting just a few fields.
 *
 * The values are saved in the workarea buffer after the boundary string.
 * Each value, including the boundary string, is terminated with a \0.
 * The values are saved in named member pointer variables, so we won't save
 * the name, only the value.
 *
 * When parsing the header strings, the first value separator is the colon (_:_)
 * that separates the header tag from its value.  For headers that have more
 * than one value (_Content-Disposition_ in particular), subsequent values are
 * separated by an equals-sign (_=_).  When processing a new line, the
 * `value-separator` variable is reset to be the header tag separator.  Once a
 * header tag has been found, the `value-separator` variable is set to '=',
 * until the next line, where it will again be set to ':'.
 *
 * Headers are read until there are two consecutive \r\n pairs, indicating the
 * end of the headers and the beginning of the field data.
 *
 * @note Portability notice: I'm using `strcasecmp` to do a case-insensitive
 * comparison of header tags.  This is a linux-only function.  Apparently
 * Windows has a similar `stricmp` function.  See @ref Portability.
 *
 * @note Debugging child processes is not hard, but not obvious, either.
 * Refer to @ref SchemaFW_Debugging_Hints for reminders.  On the same page
 * are reminders about setting watch points, which was also useful in
 * debugging this function.
 */
void Multipart_Pull::read_headers(void)
{
   char *p = m_buffer;

   // The first character must potentially be saveable
   // because at the end of a value, we don't know when
   // the next value begins until we encounter it, so
   // the initial cur value must be saved to the beginning
   // of the buffer.
   int cur;
   do
   {
      cur = m_str.getc();
   } while (cur=='\n' || cur=='\r');

   // On a new line, the value separator is a colon that separates
   // a header name from the header value(s)
   char value_separator = ':';
   
   while (cur!='\r')
   {
      // Get the name first;
      // Get character to the colon to find the name:
      do
         *p++ = cur;
      while ( p<m_end_workarea && (cur=m_str.getc())!=EOF && cur!=value_separator);
         
      if (cur==value_separator)
      {
         // replace the colon/= with a \0
         *p = '\0';

         // Single point to change string comparison function used
         // for identifying tag names:
         const auto &tagcmp = strcasecmp;

         // match the name to save the value that follows:
         
         // Note that for header tags, the value_separator
         // changes to '=' for internal values.
         if (value_separator==':')
         {
            if (0==tagcmp(m_buffer, "Content-Disposition"))
            {
               m_field_cdispo = m_buffer;
               value_separator = '=';
            }
            else if (0==tagcmp(m_buffer,"Content-Type"))
            {
               m_field_ctype = m_buffer;
               value_separator = '=';
            }
         }
         else
         {
            if (0==tagcmp(m_buffer,"name"))
               m_field_name = m_buffer;
            else if (0==tagcmp(m_buffer,"filename"))
               m_field_fname = m_buffer;
         }
         
         // Reset the pointer to overwrite the name, which
         // we no longer need since we're saving the value that
         // follows to a named char*:
         p = m_buffer;

         // throw-away spaces:
         while ((cur=m_str.getc())==' ')
            ;
      }

      // get the value next

      // Get character to the semicolon or ^M to find the value:
      do
         *p++ = cur;
      while (p<m_end_workarea && (cur=m_str.getc())!=EOF && cur!=';' && cur!='\r');

      if (cur!=EOF)
      {
         // If a return, a new line will need to look for a header 
         if (cur=='\r')
            value_separator = ':';

         // throw-away whitespace:
         while ((cur=m_str.getc())==' ' || cur=='\n')
            ;
         
         // terminate value string and move m_buffer
         // to the next position for either the next
         // value or to be the general buffer for reading
         // the field content:
         *p++ = '\0';
         m_buffer = p;
      }
   }

   if (cur=='\r')
      assert('\n'==(cur=m_str.getc()));
}

/**
 * @brief Get the next character from the form.
 *
 * @return Next character in the current field.  EOF if no more field characters.
 *         EOF + Multipart_Pull::end_of_form() == true if no more characters in form.
 *
 * This function gets the next character in the current field.  When we've
 * reached the end of the field, it returns EOF.
 *
 * The end of the field may signal the end of the form.  Call end_of_form() to
 * check the document status.
 *
 * Multipart-forms use a boundary string to mark field boundaries.  This function
 * is careful to discard the characters of a full boundary string match, but does
 * return a partially-matched boundary string. The code considers a full match
 * to be the full boundary string preceeded and followed by a `\r\n`.  I do not
 * handle a pathological full boundary string match followed by something
 * other than a `\r`. Refer to the source code comments for more explanation and
 * see [RFC2046 Section 5.1](https://tools.ietf.org/html/rfc2046#section-5.1) to
 * validate the assumption that the boundary string is not included in
 * the content.
 *
 * @note When getc() reads/identifies as boundary string, it sets m_end_of_field
 * to true, and if the boundary indicates it, m_end_of_form to true as well.
 * Call 
 * by loading the headers of the next field, if any.  Plan your code so that
 * you will no longer need the header values after getting an EOF.
 */
int Multipart_Pull::getc(void)
{
   if (m_end_of_field)
      return EOF;
   
   int c;

   // This next section runs if previous calls to getc() have
   // made a partial boundary string match.  getc() will return
   // the next matched boundary string character at each subsequent
   // call, until it reaches the last matched character.  The next
   // call to getc() (after completion of the partial match) will
   // return the value of `m_match_breaker`, which was saved when
   // the character was read and signaled the boundary string
   // match was incomplete.
   //
   // The function will return immediately after handling either
   // of the two following conditions.  As a result of the early
   // return, the following `c = m_str.getc()` will not run, and
   // input stream will remain undisturbed until the partial
   // match and the match-breaker characters have been returned.
   // function will 
   if (m_boundary_ptr < m_end_boundary_chunk)
      return *m_boundary_ptr++;
   else if (m_match_breaker>-1)
   {
      // Save return value before we clear its source:
      c = m_match_breaker;

      // Clear boundary-match variables so next call to getc()
      // starts again getting characters from m_str.
      m_boundary_ptr = m_boundary;
      m_end_boundary_chunk = nullptr;
      m_match_breaker = -1;

      return c;
   }

   c = m_str.getc();

   // If we suspect a boundary, collect characters until
   // we confirm it's a boundary or we realize it's not.
   // Only when we know can we return characters, and only
   // if it's not a boundary.
   if (c==*m_boundary_ptr)
   {
      // Read as much of the boundary string as matches, including
      // the character that signals it's not the boundary string
      ++m_boundary_ptr;
      while (m_boundary_ptr<m_boundary_end && *m_boundary_ptr==(c=m_str.getc()))
         ++m_boundary_ptr;

      // If we didn't match the entire boundary string, the matched chars 
      // will need to be sent now and in subsequent calls to getc() until
      // we have sent all the matched characters.  Keep track of where the
      // last character was read so we know when to stop.
      //
      // After saving the end of the chunk, we reset m_boundary_ptr to the
      // beginning of the boundary string.  Subsequent calls will move this
      // pointer until it reachs m_end_of_chunk.
      if (m_boundary_ptr<m_boundary_end)
      {
         m_match_breaker = c;
         m_end_boundary_chunk = m_boundary_ptr;
         m_boundary_ptr = m_boundary;
         return *m_boundary_ptr++;
      }
      // If the pointer is at the boundary end, we fully matched
      // the boundary string.  We're at the end of the current field.
      else
      {
         m_end_of_field = true;
            
         // Reset the m_boundary_ptr to indicate there are
         // no pre-getc-ed characters to read out.
         m_boundary_ptr = m_boundary;
         
         // The field is finished, clear field head values:
         reset_field_heads();

         // The next two characters determine if we're simply
         // at the end of the field or at the end of the form.
         uint16_t buff;
         *reinterpret_cast<char*>(&buff) = m_str.getc();
         reinterpret_cast<char*>(&buff)[1] = m_str.getc();

         if (buff==s_END_FORM)
         {
            m_end_of_form = true;
         }
         else if (buff!=s_END_FIELD)
         {
            // Full boundary string match followed by something other than
            // \r\n or --.
            //
            // This should never happen, the boundary string is guaranteed
            // not to be included the content, so matching the entire boundary
            // string must either end the field or the form.
            //
            // I suppose if I was more ambitious or not so far behind releasing
            // this, I might put some defensive code here to treat this as a
            // partial match.  That would be complicated and would break a few
            // assumptions that allows the current process to work.  Since it's
            // not supposed to occur, I'm not going to bother unless I see
            // the error message sometime.
            fprintf(stderr,
                    "Unexpected post-boundary characters (uint16_t) %d\n",
                    buff);

            m_end_of_form = true;
         }

         // If at end of form, read all remaining characters in the
         // stream so m_str.eof()==true as a cross-process signal that
         // we've reached the end of the form.
         if (m_end_of_form)
         {
            while (EOF!=(c=m_str.getc()))
               ;
         }

         return EOF;
      }
   }  // end of `if (c==*m_boundary_ptr)`

   // If no boundary matching, just return the character:
   return c;
}

/**
 * @brief Get a file handle to read the converted contents of the current field.
 *
 * Use this function to read the CSV results of a spreadsheet conversion.
 *
 * The caller of this function should read from the returned file handle
 * until the read function returns 0 to indicate it has reached the end
 * of the data.  Then the file handle should be closed.
 *
 * Refer to the snippet on the Multipart_Pull.
*/
int Multipart_Pull::get_csv_file_handle(fhandle_payload &pl)
{
   // Open both pipes:
   int result;
   if ((result=pipe(pl.pipe_in)))
   {
      fprintf(stderr, "multipart_pull stdin pipe failed: %s\n",
              strerror(errno));

      return -1;
   }
   else if ((result=pipe(pl.pipe_out)))
   {
      // If second pipe failed, close first pipe before leaving:
      close(pl.pipe_in[0]);
      close(pl.pipe_in[1]);
      
      fprintf(stderr, "multipart_pull stdout pipe failed: %s\n", strerror(errno));

      return -1;
   }

   pid_t pid = fork();
   // If fork failed, there are no processes to close,
   // so we can just close the pipes, log the failure,
   // and return.
   if (pid==-1)
   {
      // Close pipes
      close(pl.pipe_in[0]);
      close(pl.pipe_in[1]);
      close(pl.pipe_out[0]);
      close(pl.pipe_out[1]);

      fprintf(stderr, "multipart_pull fork failed: %s.\n", strerror(errno));
      return -1;
   }

   // If we're here, we have two pipes and a successful fork.
   // We have to be aware of the multiple processes when we
   // report errors and abort the function.

   if (pid==0)    // child process
   {
      // close unneeded pipes:
      close(pl.pipe_in[1]);   // write-end of stdin pipe used by new thread
      close(pl.pipe_out[0]);  // read-end of stdout pipe used by parent process

      // Replace the process's stdin and stdout with the pipes:

      close(STDIN_FILENO);
      if (-1==(result=dup2(pl.pipe_in[0], STDIN_FILENO)))
         fprintf(stderr,
                 "multipart_pull stdin dup2 failed: %s\n",
                 strerror(errno));

      // if stdin replaced, do stdout:
      if (result!=-1)
      {
         close(STDOUT_FILENO);
         if (-1==(result=dup2(pl.pipe_out[1], STDOUT_FILENO)))
            fprintf(stderr,
                    "multipart_pull stdin dup2 failed: %s\n",
                    strerror(errno));
      }

      // If either of the dups failed, close our end of the pipes
      // and exit with a failure code
      if (result==-1)
      {
         close(pl.pipe_in[0]);
         close(pl.pipe_out[1]);
         _exit(EXIT_FAILURE);
      }

      // throw away error messages from ssconvert:
      if (true)
      {
         int fh = open("/dev/null", O_APPEND, O_WRONLY);
         if (fh==-1)
            fprintf(stderr, "Open /dev/null failed %s\n", strerror(errno));
         else
         {
            close(STDERR_FILENO);
            dup2(fh, STDERR_FILENO);
            close(fh);
         }
      }

      // Start converter:
      execl("/usr/bin/ssconvert", "ssconvert",
            "-T", "Gnumeric_stf:stf_assistant",
            "-O", "quote=\"'\" quoting-mode=always",
            "fd://0",
            "fd://1",
            nullptr);
   }

   // Parent process
   
   // close unnecessary pipes:
   close(pl.pipe_in[0]);    // read-end used as stdin for ssconvert in child process
   close(pl.pipe_out[1]);   // write-end used as stdout for ssconvert in child process

   // Create thread to file write end into child process:
   pthread_t pthread;
   pthread_attr_t *p_pta = nullptr;
   if (pthread_create(&pthread,
                      p_pta,
                      pthread_create_start_routine,
                      static_cast<void*>(&pl)))
   {
      // Closing write end of pipe will result in an EOF
      // when the child process attempts to read the pipe,
      // which will end the child process.
      close(pl.pipe_in[1]);

      // Close the stdout pipe that we no longer need:
      close(pl.pipe_out[0]);

      // Log the error,
      fprintf(stderr,"Thread create failed: %s\n", strerror(errno));

      // Return sign of failure:
      return -1;
   }
   else
   {
      // size_t r;
      // char buff[80];
      // while (0>=(r=read(pl.pipe_out[0], buff, sizeof(buff))))
      //    write(STDOUT_FILENO, buff, r);

      // return -1;
      
      // Thread running, simply return the read end of the stdout pipe.
      // When the calling function receives an EOF, the child process
      // will have completed.  Closing the handle will end the pipe
      // and complete the cleanup.
      return pl.pipe_out[0];
   }
}


void Multipart_Pull::flag_field_complete(void)
{
   m_end_of_field = true;
}


/**
 * @brief Read the next field's header lines and reset the m_end_of_field flag.
 *
 * The proper working of this function, and the Multipart_Pull object as well,
 * depends on the file pointer being positioned exactly at the beginning of the
 * field headers before calling this function.  To ensure this, __it is the
 * reponsibility of each field-reader to read exactly to the end of the
 * boundary string and no more__.
 *
 * The reason for this is because the purpose of Multipart_Pull is to be
 * called from a child process.  There is no convenient way to communicate
 * between the child and parent processes, so the parent needs to trust that
 * the child will leave the file pointer in an appropriate position to
 * continue reading fields.
 *
 * This function will read and ignore any remainer of the current field
 * before loading the new field.  The Schema for which this class was
 * created, is free to ignore fields that are not import fields by simply
 * calling this function again when is_file_upload() returns false.
 */
bool Multipart_Pull::next_field(void)
{
   // Finish reading the current field, if not already done:
   if (!m_end_of_field)
   {
      int c;
      while (EOF!=(c=getc()))
         ;
   }

   // At the end of the field, reset field heads before reading
   // the field meta data from its headers:
   reset_field_heads();
   
   if (m_end_of_form)
      return false;
   // Check if a child process read to the end of the stream:
   else if (m_str.eof())
   {
      m_end_of_form = true;
      return false;
   }
   
   read_headers();
   m_end_of_field = false;

   return true;
}






#ifdef INCLUDE_MULTIPART_PULL_MAIN

#include <stdio.h>    // printf

#include <sys/types.h> // open()
#include <sys/stat.h>  // open()

#include "vclasses.hpp"
#include "prandstr.cpp"


/**
 * @brief Demonstrate the use of Multipart_Pull, especially for importing
 *        a spreadsheet.
 *
 * This function is a test function for the object as well as a demonstration
 * snippet for the documentation.  Because there are at least two ways to get
 * a Multipart_Pull object, this demo function has left that task to a calling
 * function.
 */
//@ [Demo_MP_Pull]
void demo_mp_pull(Multipart_Pull &mpp)
{
   printf("          Form Boundary: %s\n", mpp.boundary());
   
   while (!mpp.end_of_form())
   {
      // Load the meta-data of the next field
      if (mpp.next_field())
      {
         // Demonstrate meta-data access:
         printf("\n***          Field Name: %s\n", mpp.field_name());
         printf("    Content Disposition: %s\n", mpp.field_content_disposition());
         if (mpp.field_file_name())
            printf("     Uploaded File Name: %s\n", mpp.field_file_name());
         if (mpp.field_content_type())
            printf("     Uploaded File Type: %s\n", mpp.field_content_type());

         // Special handling for file upload:
         if (mpp.is_file_upload())
         {
            // Allocate the memory needed to run get_csv_file_handle()
            // in the current stack frame so it remains in scope while
            // reading from the returned file handle.
            Multipart_Pull::fhandle_payload pl(mpp);

            // Call the function to get a file handle to the contents
            // of the spreadsheet converted to CSV:
            int fh = mpp.get_csv_file_handle(pl);

            if (fh!=-1)
            {
               printf("Converted File Contents:\n\n");
               
               char iobuff[256];
               size_t bytes;

               // Keep reading until read returns 0 to indicate
               // that we've reached the end of the field contents.
               while (0!=(bytes=read(fh, iobuff, sizeof(iobuff))))
                  write(STDOUT_FILENO, iobuff, bytes);

               // Close the file handle to clean up (especially in FASTCGI mode)
               // to clean up
               close(fh);
            }
         }
         else
         {
            // Display the field value for non-file-upload:
            printf("            Field Value: \"");
            
            int c;
            while (EOF!=(c=mpp.getc()))
               fputc(c, stdout);
            fputc('"', stdout);
            fputc('\n', stdout);
         }
      }
   }
}
//@ [Demo_MP_Pull]


int main(int argc, char **argv)
{
   FILE *f = fopen("multipart_form.txt", "rb");
   if (f)
   {
      StrmStreamer str(f);

      Multipart_Pull mpp(str);
      demo_mp_pull(mpp);
      
      fclose(f);
   }
   else
      printf("Unable to open multipart_form.txt.\n");
}


#endif
