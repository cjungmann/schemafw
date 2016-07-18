#include <stdio.h>

#ifdef FASTCGI

// Prevent FASTCGI override of fopen and fclose
#define NO_FCGI_DEFINES

#include "istdio.hpp"


/**
 * @brief Change streams to default console streams.
 *
 * When running schema.fcgi in batch mode for debugging, we want the input and
 * output to be with the console, not the FCGI replacements.  This function
 * modifies FCGI_stdin, FCGI_stdout, and FCGI_stderr to use stdin, stdout, and
 * stderr, respectively.
 */
void reset_fcgi_streams(void)
{
   _fcgi_sF[0].stdio_stream = stdin;
   _fcgi_sF[1].stdio_stream = stdout;
   _fcgi_sF[2].stdio_stream = stderr;
}



int ifprintf(FCGI_FILE* f, const char *format, ...)
{
   va_list args;
   va_start(args,format);
   int rval = FCGI_vfprintf(f,format,args);
   va_end(args);
   return rval;
}

#else   // ifndef FASTCGI
inline int  ifprintf(FILE* f, const char *format, ...)
{
   valist args;
   va_start(args,format);
   int rval = vfprintf(f,format,args);
   va_end(args);
   return rval;
}

#endif // ifdef FASTCGI
