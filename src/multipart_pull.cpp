// -*- compile-command: "g++ -std=c++11 -Wno-pmf-conversions -Wall -Werror -Weffc++ -pedantic -ggdb -DINCLUDE_MAIN `mysql_config --cflags` -U NDEBUG -o multipart_pull multipart_pull.cpp `mysql_config --libs`" -*-

#include <ctype.h>
#include <unistd.h>       // fork()
#include <sys/wait.h>     // wait();
#include <fcntl.h>        // open() constant values

#include "multipart_pull.hpp"
#include "linebuffer.hpp"
#include "istdio.hpp"

uint16_t Multipart_Pull::s_END_FIELD = 2573; // "\r\n"
uint16_t Multipart_Pull::s_END_FORM = 11565; // "--"
const char Multipart_Pull::s_multipart_str[] = "multipart/form-data; ";
const int Multipart_Pull::s_len_multipart_str = strlen(s_multipart_str);
const char Multipart_Pull::s_boundary_str[] = "boundary=";
const int Multipart_Pull::s_len_boundary_str = strlen(s_boundary_str);

/** @brief Constructor */
Multipart_Pull::Multipart_Pull(IStreamer &s)
   : m_workarea(),
     m_errormsg(),
     m_end_workarea(nullptr),
     m_str(s),
     m_boundary(nullptr),
     m_boundary_end(nullptr),
     m_boundary_length(-1),
     m_boundary_ptr(nullptr),
     m_end_boundary_chunk(nullptr),
     m_match_breaker(-1),
     m_buffer(nullptr),
     m_field_name(nullptr),
     m_field_fname(nullptr),
     m_field_ctype(nullptr),
     m_field_incomplete(false),
     m_form_complete(false)
{
   // Leave 10 characters of slop:
   m_end_workarea = m_workarea + sizeof(m_workarea) - 10;
   *m_errormsg = '\0';

#ifdef INCLUDE_MAIN   
   initialize_from_file();
#endif

   read_initial_boundary_and_prepare_buffers();
   reset_field_heads();
}

Multipart_Pull::~Multipart_Pull()
{
}


/**
 * @brief Reads first line from submission to set form meta data.
 *
 * Confirms appropriate header format while saving the boundary string
 * and saving a pointer to the beginning of unused buffer space.
 */
#ifdef INCLUDE_MAIN
void Multipart_Pull::initialize_from_file(void)
{
   char *ptr = m_workarea;
   char *limit = m_end_workarea - 2;

   /********************************************************************/
   // Returns false if it reaches EOF without finding /r/n.
   auto read_and_discard_file_preamble = [this, &ptr, &limit](void) -> bool
   {
      int c;
      int nlcount = 0;
      // Read first line into the workarea:
      while(EOF!=(c=m_str.getc()) && ptr<limit)
      {
         *ptr = static_cast<char>(c);
      
         // Detect second EOL:
         if (*ptr=='\n' && *(ptr-1)=='\r')
            if (nlcount++)  // postfix++ evaluates to 0 on first pass
               return true;

         ++ptr;
      }

      return false;
   };

   /********************************************************/
   // Returns true if everything is as expected and m_workarea
   // starts with the boundary string.
   auto confirm_multipart_preamble = [this, &ptr](void) -> bool
   {
      char *p = strstr(m_workarea, s_multipart_str);
      if (p)
      {
         p += s_len_multipart_str;
         p = strstr(p, s_boundary_str);
         if (p)
            return true;
      }

      return false;
   };

   // Beginning of function execution:
   if (!read_and_discard_file_preamble() ||
       !confirm_multipart_preamble() )
      throw std::runtime_error("Submitted form not encoded as multipart/form-data.");
}
#endif // INCLUDE_MAIN

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
< * it and save it to the beginning of the workarea, then keep a pointer
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

   // Start the comparison buffer with /r/n because after the initial boundary
   // string, all boundary strings start with /r/n:
   *pstr++ = '\r';
   *pstr++ = '\n';

   while (EOF!=(c=m_str.getc()) && pstr < limit)
   {
      // Get chars up to \r\n, replacing the \r with \0, but leaving
      // the file pointer to the beginning of a field description.
      if (c=='\n')
      {
         if (*(pstr-1)=='\r')
         {
            *(pstr-1) = '\0';
            m_boundary_end = pstr-1;
            m_buffer = pstr;
         }
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
 * @brief Reset all field head pointers to nullptr and reset m_buffer pointer.
 */
void Multipart_Pull::reset_field_heads(void)
{
   m_field_name =
      m_field_fname =
      m_field_ctype = nullptr;

   m_boundary_ptr = m_boundary;
   m_end_boundary_chunk = nullptr;
   m_match_breaker = -1;

   m_buffer = const_cast<char*>(m_boundary_end+1);
}


void Multipart_Pull::read_headers(void)
{
   char *ptr = m_buffer; // Progress pointer
   char *pline;          // pointer to current line
   char *hname;          // header name
   char *hvalue;         // header value (to be further parsed)

   /**********************************************/
   auto discard_spaces = [](const char *p) -> char*
   {
      // Don't forget to increment the pointer after
      // replacing a character with '\0':
      assert(*p);
      
      while (*p && isspace(*p))
         ++p;
      return const_cast<char*>(p);
   };

   /*******************************************/
   auto unquote_value = [](char *value) -> char*
   {
      char *p = value;
      while (*++p)
         ;
      
      if (*value=='"' && *(p-1)=='"')
      {
         *(p-1) = '\0';
         ++value;
      } 
      
      return value;
   };

   /*****************************************************/
   auto read_next_header_line = [this, &ptr, &pline](void) -> bool
   {
      int c;
      pline = ptr;
      while (ptr < m_end_workarea)
      {
         if ((c=m_str.getc())==EOF)
            break;
         else if ((*ptr=static_cast<char>(c))=='\n')
            break;

         ++ptr;
      }
      
      if (*ptr=='\n' && *(ptr-1)=='\r')
      {
         *(ptr-1) = '\0';
         
         // Leave closure pointer to character following \r\n:
         ++ptr;

         if (ptr-pline == 2)
            return false;

         
         return true;
      }
      else
         return false;
   };

   /********************************************************************/
   auto parse_header_line = [this,
                             &pline,
                             &hname,
                             &hvalue,
                             &discard_spaces](void) -> bool
   {
      hname = hvalue = nullptr;
      
      char *p = pline;
      
      while (*p && *p!=':')
         ++p;

      if (*p)
      {
         hname = pline;
         *p = '\0';

         p = discard_spaces(p+1);

         if (*p)
            hvalue = p;

         return true;
      }
      return false;
   };

   /**************************************************************************/
   auto set_disposition_value = [this, &unquote_value](const char *name,
                                                       char *value) -> void
   {
      if (0==strcasecmp("name", name))
         m_field_name = unquote_value(value);
      else if (0==strcasecmp("filename", name))
         m_field_fname = unquote_value(value);
   };

   /********************************************************/
   // Returns true if more settings to retrieve.
   auto get_next_disposition_value = [this, &discard_spaces, &set_disposition_value](char*& p) -> bool
   {
      char *name = p;
      char *value= nullptr;

      while (*p && *p!='=')
         ++p;

      if (*p)
      {
         *p = '\0';
         value = ++p;

         while (*p && *p!=';')
            ++p;

         // Terminate last item in disposition list:
         if (*p)
         {
            *p = '\0';
            
            // move to next setting:
            p = discard_spaces(p+1);
         }

         set_disposition_value(name, value);
      }

      return *p!=0;
   };

   /********************************************/
   auto parse_disposition = [this,
                             &hvalue,
                             &discard_spaces,
                             &get_next_disposition_value](void)
   {
      char *dtype = nullptr;
      
      char *p = hvalue;
      while (*p && *p!=';')
         ++p;
      
      if (*p)
      {
         dtype = hvalue;
         *p = '\0';

         p = discard_spaces(p+1);
      }

      if (dtype && 0==strcasecmp(dtype, "form-data"))
      {
         while (get_next_disposition_value(p))
            ;
      }
   };

   while (read_next_header_line())
   {
      if (parse_header_line())
      {
         if (0==strcasecmp(hname, "Content-Type"))
            m_field_ctype = hvalue;
         else if (0==strcasecmp(hname, "Content-Disposition"))
            parse_disposition();
      }
   }
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
   if (!get_field_incomplete())
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
         set_field_incomplete(false);
            
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
            set_form_complete(true);
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

            set_form_complete(true);
         }

         // If at end of form, read all remaining characters in the
         // stream so m_str.eof()==true as a cross-process signal that
         // we've reached the end of the form.
         if (get_form_complete())
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
 * @brief Thread initialization function for transmitting to stdin.
 *
 * Although this function accesses shared memory, especially in
 * set_field_incomplete(),  I am avoiding using mutexes by having the
 * calling thread only accessing member variable m_field_complete
 * when this thread is complete.
 */
void* Multipart_Pull::start_stdin_thread(void *data)
{
   Multipart_Pull &mpp = static_cast<thread_data*>(data)->mpp;
   int            &fh = static_cast<thread_data*>(data)->fhandle;
   int            i;
   ssize_t        cr;

   while (EOF!=(i=mpp.getc()))
   {
      if ((cr=write(fh, static_cast<void*>(&i), 1))!=1)
      {
         fprintf(stderr,
                 "start_stdin_thread write error (%s) wrote %ld chars\n",
                 strerror(errno),
                 cr);
      }
   }

   // Close pipe to signal ssconvert that the tranmission is complete:
   close(fh);
   fh = -1;

   uintptr_t rval = (i==EOF) ? 1 : 0;
   return reinterpret_cast<void*>(rval);
}

/**
 * @brief Thread initialization function for processing stderr.
 */
void* Multipart_Pull::start_stderr_thread(void *data)
{
   Multipart_Pull &mpp = static_cast<thread_data*>(data)->mpp;
   int            &fh = static_cast<thread_data*>(data)->fhandle;

   auto get_line = [&mpp](const char *line)
   {
      // Left-trim spaces
      while (*line && *line==20)
         ++line;
      
      if (*line && !strstr(line, "GLib-GObject"))
      {
         // Only write the output if the line begins with "E ",
         // at least until we experience other fatal errors that
         // begin some other way:
         if (*line=='E' && *(line+1)==' ')
         {
            line+=2;
            strncpy(mpp.m_errormsg, line, sizeof(m_errormsg));
         }
      }
   };

   // Start line buffer using the lambda callback function:
   linebuffer::BufferFile(get_line, fh);

   return reinterpret_cast<void*>(0);
}

/**
 * @brief Create processes and threads to use ssconvert to process the submitted file.
 *
 * @param callback Lambda function that prepares MySQL_LoadData object with a
 *                 filehandle, runs the MySQL_LoadData::import() function.
 *
 * Initially starts a child process for _ssconvert_, then creates two new threads,
 * one for stdin, the other for stderr (to filter for important error messages),
 * then, in the main thread, passes stdout to an instance of MySQL_LoadData to be
 * used by the MySQL_LoadData::mih_read() when called by MySQL_LoadData::import();
 *
 * This function is also a coding experiment, using several lambda functions to
 * provide context and explanation to small blocks of code that otherwise would
 * be all in one long loop.  The advantage of this method is that it keeps related
 * functions grouped together, but it causes an inconvenient separation of the
 * closure variables from the section at the end of the function that drives it.
 * It _may_ be a performance benefit because some of the lambda functions may be
 * implemented inline, and it won't be necessary to calculate an offset to member
 * variables and functions in order to run.
 *
 * If nothing else, it's an example of certain coding strategy.
 */
void Multipart_Pull::t_send_for_csv_filehandle(const IGeneric_Callback<int>& callback)
{
   int l_pipe_in[2];
   int l_pipe_out[2];
   int l_pipe_err[2];

   int* pipe_in = l_pipe_in;
   int* pipe_out = l_pipe_out;
   int* pipe_err = l_pipe_err;

   int* pipes[] = {pipe_in, pipe_out, pipe_err, nullptr};
   int result;

   /***********************************/
   auto close_clear = [](int& p) -> void
   {
      if (p>-1)
      {
         close(p);
         p = -1;
      }
   };

   /*****************************************************/
   auto close_pipes = [&pipes, &close_clear](void) -> void
   {
      for (int** p = pipes; *p; ++p)
      {
         close_clear(**p);
         close_clear(*++*p);
      }
   };


   /***** Here starts the actual running of the function: *****/
   
   // Open all the pipes
   for (int** p = pipes; *p; ++p)
   {
      if ((result=(pipe(*p))))
      {
          fprintf(stderr, "multipart_pull pipe creation failed: %s\n",
                  strerror(errno));
          close_pipes();
          break;
      }
   }

   pid_t pid = fork();
   if (pid==-1)
   {
      close_pipes();
      fprintf(stderr, "multipart_pull fork failed: %s.\n", strerror(errno));
   }
   else if (pid==0)
   /*** CHILD PROCESS ***/
   {
      int arr[] = { pipe_in[0], STDIN_FILENO,
                    pipe_out[1], STDOUT_FILENO,
                    pipe_err[1], STDERR_FILENO,
                    -1 };
      
      int count=1;
      for (int* i=arr; *i>=0; i+=2, ++count)
      {
         if (-1==dup2(*i, *(i+1)))
         {
            fprintf(stderr, "%d: dup2(%d,%d) failed: (%s).\n",
                    count, *i, *(i+1), strerror(errno));
            close_pipes();
            exit(1);
         }
      }
      
      close_clear(pipe_in[1]);
      close_clear(pipe_out[0]);
      close_clear(pipe_err[0]);

      // Start converter:
      execl("/usr/bin/ssconvert", "ssconvert",
            "-T", "Gnumeric_stf:stf_assistant",
            "-O", "quote=\"'\" quoting-mode=always",
            "fd://0",
            "fd://1",
            nullptr);
   }
   else
   /*** PARENT PROCESS ***/
   {
      // Pipe housekeeping, close unused ends of pipes:
      close_clear(pipe_in[0]);
      close_clear(pipe_out[1]);
      close_clear(pipe_err[1]);

      // Create thread to filter error messages:
      thread_data td_err = { *this, pipe_err[0] };
      pthread_t pt_err;
      if (pthread_create(&pt_err,
                         nullptr,
                         start_stderr_thread,
                         static_cast<void*>(&td_err) ))
      {
         throw std::runtime_error("Import stderr pipe failed.");
      }

      // Create thread to pipe the contents of the multipart form to ssconvert:
      thread_data td_in = { *this, pipe_in[1] };
      pthread_t pt_in;
      if (pthread_create(&pt_in,
                         nullptr,
                         start_stdin_thread,
                         static_cast<void*>(&td_in) ))
      {
         throw std::runtime_error("Import stdin pipe failed.");
      }

      // Main thread invokes callback function to send results of ssconvert to MySQL:
      callback(pipe_out[0]);
      
      // Close pipe to signal ssconvert that the file is complete:
      close_clear(pipe_out[0]);
      
      int wstatus;
      waitpid(pid, &wstatus, 0);

      // Creating an pointer variable makes casting for pthread_join() easier:
      void *preturn;
      
      // Wait for the completion of the stdin thread, clearing the field_incomplete
      // flag if it returns successfully:
      pthread_join(pt_in, &preturn);
      uintptr_t stdin_return = reinterpret_cast<uintptr_t>(preturn);
      close_clear(pipe_in[1]);

      if (stdin_return)
         set_field_incomplete(false);
      
      // Wait for completion of the stderr thread, logging an error if appropriate.
      pthread_join(pt_err, &preturn);
      close_clear(pipe_err[0]);
   }
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
   if (get_field_incomplete())
   {
      int c;
      while (EOF!=(c=getc()))
         ;
   }

   set_field_incomplete(false);

   // At the end of the field, reset field heads before reading
   // the field meta data from its headers:
   reset_field_heads();
   
   if (get_form_complete())
      return false;
   // Check if a child process read to the end of the stream:
   else if (m_str.eof())
   {
      set_form_complete(true);
      return false;
   }
   
   read_headers();

   set_field_incomplete(true);
   return true;
}


#ifdef INCLUDE_MAIN
#undef INCLUDE_MAIN

#include <stdio.h>    // printf
#include <stdlib.h>   // atoi

#include <sys/types.h> // open()
#include <sys/stat.h>  // open()

#include "vclasses.hpp"
#include "mysql_loaddata.hpp"

#include "prandstr.cpp"
#include "linebuffer.cpp"
#include "istdio.cpp"
#include "mysql_loaddata.cpp"

void load_to_table(Multipart_Pull &mpp,
                   const char *database_to_use,
                   const char *import_table_to_use,
                   int session_id)
{
   MYSQL mysql;

   auto f = [&mysql, &import_table_to_use, session_id](int fh)
      {
         MySQL_LoadData msld(&mysql, fh, session_id, import_table_to_use);
         msld.import();
      };

   mysql_init(&mysql);
   mysql_options(&mysql,MYSQL_READ_DEFAULT_FILE,"/etc/mysql/my.cnf");
//   mysql_options(&mysql,MYSQL_READ_DEFAULT_FILE,"~/.my.cnf");
   mysql_options(&mysql,MYSQL_READ_DEFAULT_GROUP,"client");
   mysql_options(&mysql, MYSQL_OPT_LOCAL_INFILE, nullptr);


   const char *host = nullptr;
   const char *user = nullptr;
   const char *pword = nullptr;
   const char *db = database_to_use;
   int        port = 0;
   const char *socket = nullptr;
   unsigned long client_flag = 0;

   MYSQL *handle = mysql_real_connect(&mysql,
                                      host, user, pword, db,
                                      port, socket, client_flag);

   if (handle)
   {
      MySQL_LoadData::disable_local_infile(&mysql);
      
      printf("Host: %s\nUser: %s\n", mysql.host, mysql.user);

      if (import_table_to_use)
         mpp.send_for_csv_filehandle(f);
   }
   else
      printf("Failed to connect (%s).\n", mysql_error(&mysql));
   
   mysql_close(&mysql);
   mysql_library_end();
   
}

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
void demo_mp_pull(Multipart_Pull &mpp,
                  const char *database_to_use,
                  const char *import_table_to_use,
                  int session_id)
{
   printf("          Form Boundary: %s\n", mpp.boundary());

   while (!mpp.get_form_complete())
   {
      // Load the meta-data of the next field
      if (mpp.next_field())
      {
         // Demonstrate meta-data access:
         printf("\n             Field Name: %s\n", mpp.field_name());
         if (mpp.field_file_name())
            printf("     Uploaded File Name: %s\n", mpp.field_file_name());
         if (mpp.field_content_type())
            printf("     Uploaded File Type: %s\n", mpp.field_content_type());

         // Special handling for file upload:
         if (mpp.is_file_upload())
         {
            if (database_to_use && import_table_to_use)
               load_to_table(mpp,database_to_use,import_table_to_use,session_id);
            else
            {
               auto f = [](int fh)
                  {
                     char iobuff[256];
                     size_t bytes = 0;

                     // Keep reading until read returns 0 to indicate
                     // that we've reached the end of the field contents.
                     while (0!=(bytes=read(fh, iobuff, sizeof(iobuff))))
                        write(STDOUT_FILENO, iobuff, bytes);

                     // Close the file handle to clean up (especially in FASTCGI mode)
                     // to clean up
                     close(fh);
                  };

               fputs("            Field Value, converted to CSV:\n", stdout);
               
               mpp.send_for_csv_filehandle(f);
               fputc('\n', stdout);
            }
         }
         else
         {
            // Display the field value for non-file-upload:
            fputs("            Field Value: \"", stdout);
            
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
   const char *post_file_to_open = nullptr;
   const char *database_to_use = nullptr;
   const char *insert_table_to_use = nullptr;
   int        session_id = 1000;

   if (argc<2)
      printf("Usage:\n"
             "To view the CSV output:\n"
             "multipart_pull [post_file]\n\n"
             "To import into a table:\n"
             "multipart_pull [post_file] [database] [quarantine table] [session_id]\n"
         );
   else
   {
      post_file_to_open = argv[1];
      if (argc>2)
      {
         database_to_use = argv[2];
         if (argc>3)
         {
            insert_table_to_use = argv[3];
            if (argc>4)
               session_id = atoi(argv[4]);
         }
      }
   }

   if (post_file_to_open)
   {
      FILE *f = fopen(post_file_to_open, "rb");
      if (f)
      {
         StrmStreamer str(f);
         Multipart_Pull mpp(str);
         demo_mp_pull(mpp, database_to_use, insert_table_to_use, session_id);
      
         fclose(f);
      }
      else
         printf("Unable to open %s.\n", post_file_to_open);

      return 0;
   }

   return 1;
}


#endif  // INCLUDE_MAIN
