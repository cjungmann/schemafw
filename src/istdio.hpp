/** @file */

#ifndef ISTDIO_HPP_SOURCE
#define ISTDIO_HPP_SOURCE

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

#ifdef FASTCGI
#include <fcgi_stdio.h>
inline int  ifgetc(FCGI_FILE* f)                { return FCGI_fgetc(f); }
inline bool ifeof(FCGI_FILE* f)                 { return FCGI_feof(f); }
inline int  ifputs(const char* s, FCGI_FILE* f) { return FCGI_fputs(s,f); }
inline int  ifputc(char c, FCGI_FILE *f)        { return FCGI_fputc(c,f); }

void reset_fcgi_streams(void);


// Implemented in istdio.cpp to avoid macro renaming
// FCGI_FILE *ifopen(const char *path, const char *mode, FCGI_FILE &f);
// int ifclose(FCGI_FILE &f);
int ifprintf(FCGI_FILE* f, const char *format, ...);

#else   //  ifndef FASTCGI
inline int  ifgetc(FILE* f)                { return fgetc(f); }
inline bool ifeof(FILE* f)                 { return feof(f); }
inline int  ifputs(const char* s, FILE* f) { return fputs(s,f); }
inline int  ifputc(char c, FILE *f)        { return fputc(c,f); }

void reset_fcgi_streams(void)              { ; }

// Implementions of same named function in ifdef FASTCGI section above:
int  ifprintf(FILE* f, const char *format, ...);
#endif   // ifdef FASTCGI


#endif
