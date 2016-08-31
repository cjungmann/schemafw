/**
 * @file
 *
 * This file provides "portable" access to a subset of the stdio.h functions.  By
 * "portable", I mean that using the functions defined here will decrease the
 * pain associated with using these functions in FASTCGI and non-FASTCGI mode.
 *
 * Functions that start with _i_ will use the FCGI_FILE and FCGI_f... functions
 * or FILE and f... functions in FASTCGI mode or non-FASTCGI mode respectively.
 * Use, for example, ifputs() where you would otherwise use fputs(), and it will
 * work in both FASTCGI and non-FASTCGI mode.
 *
 * There is also a set of functions that begin with _g_ that will always use
 * FILE* and standard f... functions from stdio.h.  These are mainly used in
 * response mode type=save-post to write the POST data to a disk file.
 *
 * See @ref ISTDIO_Examples for usage examples.
 */

/**
 * @page ISTDIO_Examples istdio.hpp Examples
 *
 * Look at the following snippet for examples of using _i_ prefixed functions
 * to read from stdin, and _g_ prefixed to open a FILE instead of a FCGI_FILE,
 * and to write using stdin.h functions.
 * @snippet schema.cpp Save_Stdin
 *
 * The following snippet uses _g_ prefixed function stdio.h functions to work with
 * a FILE*, even if the code is compiled in FASTCGI mode.
 * @snippet schema.cpp Write_Multipart_Preamble
 *
 */

#ifndef ISTDIO_HPP_SOURCE
#define ISTDIO_HPP_SOURCE

#include <assert.h>
#include <stdio.h>

#ifdef FASTCGI
#define GFILE FILE

#include <fcgi_stdio.h>
inline int  ifgetc(FCGI_FILE* f)                { return FCGI_fgetc(f); }
inline bool ifeof(FCGI_FILE* f)                 { return FCGI_feof(f); }
inline int  iferror(FCGI_FILE* f)               { return FCGI_ferror(f); }
inline int  ifputs(const char* s, FCGI_FILE* f) { return FCGI_fputs(s,f); }
inline int  ifputc(char c, FCGI_FILE *f)        { return FCGI_fputc(c,f); }
inline size_t ifread(void* bf, size_t s, size_t n, FCGI_FILE *fp)
                                                { return FCGI_fread(bf,s,n,fp); }

void reset_fcgi_streams(void);

// Functions to access global FILE functions after FASTCGI defines
#pragma push_macro("FILE")
#undef FILE

#pragma push_macro("fopen")
#undef fopen
inline FILE* gfopen(const char* path, const char* mode) { return fopen(path,mode); }
#pragma pop_macro("fopen")

#pragma push_macro("fclose")
#undef fclose
inline int gfclose(FILE* f) { return fclose(f); }
#pragma pop_macro("fclose")

#pragma push_macro("feof")
#undef feof
inline int gfeof(FILE* f) { return feof(f); }
#pragma pop_macro("feof")

#pragma push_macro("ferror")
#undef ferror
inline int gferror(FILE* f)  { return ferror(f); }
#pragma pop_macro("ferror")

#pragma push_macro("fputs")
#undef fputs
inline int gfputs(const char *s, FILE* f) { return fputs(s,f); }
#pragma pop_macro("fputs")

#pragma push_macro("fwrite")
#undef fwrite
inline size_t gfwrite(void* ptr, size_t size, size_t nmemb, FILE *fp)
{
   return fwrite(ptr,size,nmemb,fp);
}
#pragma pop_macro("fwrite")

#pragma pop_macro("FILE")


// Implemented in istdio.cpp to avoid macro renaming
// FCGI_FILE *ifopen(const char *path, const char *mode, FCGI_FILE &f);
// int ifclose(FCGI_FILE &f);
int ifprintf(FCGI_FILE* f, const char *format, ...);

#else   //  ifndef FASTCGI
inline int  ifgetc(FILE* f)                { return fgetc(f); }
inline bool ifeof(FILE* f)                 { return feof(f); }
inline int  iferror(FILE* f)               { return ferror(f); }
inline int  ifputs(const char* s, FILE* f) { return fputs(s,f); }
inline int  ifputc(char c, FILE *f)        { return fputc(c,f); }
inline size_t ifread(void* bf, size_t s, size_t n, FILE *fp)
                                           { return fread(bf,s,n,fp); }

void reset_fcgi_streams(void)              { ; }

inline FILE *gfopen(const char* path, const char* mode) { return fopen(path,mode); }
inline int gfclose(FILE* f) { return fclose(f); }
inline int gfeof(FILE* f)   { return feof(f); }
inline int gferror(FILE* f)   { return ferror(f); }
inline int gfputs(const char *s, FILE* f) { return fputs(s,f); }
inline size_t gfwrite(void* ptr, size_t size, size_t nmemb, FILE *fp)
{
   return fwrite(ptr,size,nmemb,fp);
}

// Implementions of same named function in ifdef FASTCGI section above:
int  ifprintf(FILE* f, const char *format, ...);
#endif   // ifdef FASTCGI


#endif
