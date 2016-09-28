// -*- compile-command: "g++ -std=c++11 -Wno-pmf-conversions -Wall -Werror -Weffc++ -pedantic -ggdb -DINCLUDE_MAIN -o linebuffer linebuffer.cpp" -*-

#include <string.h>  // for memmove(), strerror()
#include <unistd.h>  // for pipe()
#include <stdlib.h>  // for exit()
#include <stdio.h>   // for fprintf()
#include <errno.h>   // for errno
#include <stdexcept> // for std::runtime_error

#include <sys/types.h>  // for waitpid()
#include <sys/wait.h>   // for waitpid()

#include "linebuffer.hpp"
namespace linebuffer
{

const static char format_line_too_long[] =
   "LineBuffer failed at line %d: buffer size of %d insufficient.";

// Leave room for expanded numbers
const static int buffsize_line_too_long =
   strlen(format_line_too_long) + 16;


void throw_line_too_long(int line_number, int buffer_length)
{
   char* buffer =
      static_cast<char*>(alloca(buffsize_line_too_long));

   snprintf(buffer,
            buffsize_line_too_long,
            format_line_too_long,
            line_number,
            buffer_length);
         
   throw std::runtime_error(const_cast<const char*>(buffer));
}


/**
 * @brief Detects if a character is a newline
 *
 * This is a very lazy function that considers to be a newline
 * character any control character whose value is below 0x20 (space).
 *
 * See the end of linebuffer::t_BufferFile() for an explanation.
 */
inline bool is_newline(const char *p) { return *p < 32; }
   
/**
 * @brief Detects if a newline character has not yet been processed.
 *
 * See the end of linebuffer::t_BufferFile() for an explanation.
 */
inline bool is_unsent(const char *p)  { return *p < 20; }

/**
 * @brief Read from @p filehandle, reporting encountered lines with @p line_callback.
 *
 * This feature has been refactored as a function from a simple class.  There
 * was no reason to have a class when all the variables are local to the function.
 *
 * The function attempts to be very efficient within the constraints of being
 * read only and non-random access.  It fills the buffer to its maximum capacity,
 * then scans characters to find lines of text.  A line is detected with the
 * discovery of a newline, which results in the following steps:
 *
 * - Replace the newline with a '\0' to terminate the string,
 * - Uses the callback functor to return a pointer to the beginning of the line, now
 *   '\0' terminated, then
 * - Restores the '\0' with a character whose value is 10 greater than the
 *   original newline character.
 *
 * If the function scans to the end of the buffer without finding a newline to
 * terminate the current line, the characters from the start of the line to
 * the end of the buffer are copied to the front of the buffer, and then the
 * remainder of buffer is filled from the open file handle, and the search for
 * a newline continues.
 *
 * The changed newline character serves to preserve the original newline value to
 * distinguish between '\r', '\n', "\r\n", "\r\r", or "\n\n".  The first three
 * indicate a single newline, the final two indicate two newlines (thus an empty
 * line).  The main problem this solves is when a single newline character
 * terminates the buffer: the next character may also be a newline, and its
 * interpretation is dependant on the initial newline that would be unavailable
 * if it wasn't copied to the head of the buffer.
 *
 * The line associated with the initial newline is sent as soon as the newline
 * is detected.  The problem is that if a single buffer-terminating newline is
 * copied to the beginning of the buffer, it should not trigger a second
 * invocation of line_callback function.  It must also be possible to compare
 * two consecutive newlines characters.  For these reasons, the newline character
 * is changed to a value 10 higher than its original value, signalling both that
 * the newline has already been processed, while maintaining the ability to
 * determine if the two consecutive newline characters are the same or are
 * different.
 */
void t_BufferFile(I_LBCallback &line_callback, /**< Callback functor that will be
                                                *   called with each line encountered
                                                *   when scanning @p filehandle.
                                                */
                     int filehandle,           /**< File handle to the read-end of an
                                                *   open pipe.
                                                */
                     int buffsize)             /**< User-defined buffer length, with
                                                *   a default size of 1024, but can be
                                                *   easily shrunk for debugging or
                                                *   increased if 1024 is not enough.
                                                */
{
   char* buffer = static_cast<char*>(alloca(buffsize));
   char* end_buffer = buffer;
   char* start_line = buffer;
   char* ptr = buffer;

   int len_incomplete_line = 0;
   int lines_printed = 0;

   /***********************************************************************/
   auto sendline = [&line_callback, &start_line, &ptr, &lines_printed](void)
   {
      char c = *ptr;
      *ptr = '\0';
      line_callback(start_line);
      *ptr = c + 10;
      start_line = nullptr;

      ++lines_printed;
   };

   /***********************************************************************/
   auto readbuff = [&filehandle,
                    &buffsize,
                    &buffer,
                    &end_buffer,
                    &ptr,
                    &len_incomplete_line](void) -> bool
   {
      ssize_t cr = read(filehandle, ptr, buffsize-len_incomplete_line);
      if (cr>0)
      {
         end_buffer = ptr + static_cast<size_t>(cr);
         return true;
      }
      return false;
   };


   while (readbuff())
   {
      while (ptr < end_buffer)
      {
         if (is_newline(ptr) && is_unsent(ptr))
         {
            // if newline the first character of buffer
            if (ptr==buffer)
               sendline();
            // if previous character is not newline,
            else if (!is_newline(ptr-1))
               sendline();
            // if two identical newline characters (compare against unsent value)
            else if (*(ptr-1)-10==*ptr)
               sendline();
               
            start_line = ptr+1;
         }
         ++ptr;
      }

      // If the line concludes with a single newline character,
      // adjust start_line so the newline is copied to the start
      // of buffer to be considered along with the next character
      // read from the file.
      if (is_newline(ptr-1) && !is_newline(ptr-2))
         --start_line;

      // Copy unterminated string to beginning of the
      // buffer and set the pointers accordingly:
      len_incomplete_line = ptr - start_line;

      if (len_incomplete_line == buffsize)
         throw_line_too_long(lines_printed+1, buffsize);
      else
      {
         memmove(buffer, start_line, len_incomplete_line);
         ptr = buffer + len_incomplete_line;
         start_line = buffer;
      }
   }

   if (ptr > buffer)
      sendline();
}

} // namespace linebuffer

#ifdef INCLUDE_MAIN
#undef INCLUDE_MAIN

const char *testbuff =
   "This is the first line.\r\n"
   "This is the second line.\r\n"
   "This is the third line.\r\n"
   "This is the fourth line.\r\n"
   "This is the fifth line.";

void test_file(void)
{
   int fpipe[2];
   if (pipe(fpipe))
      fprintf(stderr, "Error opening a pipe: %d (%s)\n", errno, strerror(errno));
   else
   {
      pid_t pid = fork();
      if (pid==0) // child process
      {
         close(fpipe[0]);
         write(fpipe[1], testbuff, strlen(testbuff));
         close(fpipe[1]);

         exit(0);
      }
      else
      {
         close(fpipe[1]);
         int counter = 0;

         auto f = [&counter](const char *line)
         {
            printf("%02d: %s\n", ++counter, line);
         };

         linebuffer::BufferFile(f, fpipe[0], 25);
         
         close(fpipe[0]);

         int status;
         waitpid(-1, &status, 0);
      }
   }
}



int main(int argc, char** argv)
{
   test_file();
   
   return 0;
}


#endif
