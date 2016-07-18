// -*- compile-command: "g++ -DINCLUDE_PRANDSTR_MAIN -std=c++11 -Wall -Werror -Weffc++ -pedantic -ggdb -o prandstr prandstr.cpp"  -*-

/** @file */

#ifndef PRANDSTR_HPP_SOURCE
#define PRANDSTR_HPP_SOURCE

#include "istdio.hpp"

#include <stdint.h>
#include <math.h>          // for log10() in ltoccp
#include "genericuser.hpp" // used by ltoccp and _ltoccp for callback

/** @brief Class for printing and manipulating dates */
class mydate
{
public:
   unsigned m_year;
   unsigned m_month;
   unsigned m_day;
   unsigned m_hour;
   unsigned m_minute;
   unsigned m_second;

   enum PRINT_TYPE {
      PT_DATE = 1,
      PT_TIME = 2,
      PT_DATETIME = 3
   };
   
   mydate(unsigned year=0, unsigned month=0, unsigned day=0,
          unsigned hour=0, unsigned minute=0, unsigned second=0)
      : m_year(year), m_month(month), m_day(day),
        m_hour(hour), m_minute(minute), m_second(second)   { }

   void print(FILE* f, PRINT_TYPE pt);
};


/**
 * @name XML-printing functions for everyone to use.
 * @{
 */
void print_char_as_xml(char c, FILE* out);
void print_str_as_xml(const char *str, FILE *out);
/** @} */

bool string_in_list(const char *str, const char * const* list);

/**
 * @brief Class to print an integer value with optional base, padding, pad char, and output.
 *
 * Since the _print function is recursive, all of the working values are
 * data members so they don't have to go in an argument list.
 */
class uint_printer
{
protected:
   uint64_t  m_val;
   bool      m_negative;
   int       m_base;
   int       m_minlen;
   char      m_pad;
   FILE*     m_out;
   int       w_i;

   void _print(void);
   
public:
   uint_printer(uint64_t val, int base, int minlen, char pad, FILE *out)
      : m_val(val), m_negative(false),
        m_base(base), m_minlen(minlen),
        m_pad(pad), m_out(out), w_i(0)    { }
   virtual ~uint_printer()                { }


   void print(void) { _print(); }

   uint_printer(const uint_printer &)             = delete;
   uint_printer & operator=(const uint_printer &) = delete;
};

/** @brief Using uint_printer::print functions after adjusting if negative. */
class int_printer : public uint_printer
{
public:
   int_printer(int64_t val, int base, int minlen, char pad, FILE* out)
      : uint_printer(static_cast<uint64_t>(val<0?-val:val), base, minlen, pad, out)
   { m_negative = val<0; }
        
};

inline void print_int(int64_t val, FILE* f=stdout)                                           { int_printer p(val,10,0,' ',f); p.print(); }
inline void print_int(int64_t val, unsigned base, FILE *f=stdout)                            { int_printer p(val,base,0,' ',f); p.print(); }
inline void print_int(int64_t val, unsigned minlen, char pad, FILE *f=stdout)                { int_printer p(val,10,minlen,pad,f); p.print(); }
inline void print_int(int64_t val, unsigned base, unsigned minlen, char pad, FILE *f=stdout) { int_printer p(val,base,minlen,pad,f); p.print(); }

inline void print_uint(uint64_t val, FILE* f=stdout)                                           { uint_printer p(val,10,0,' ',f); p.print(); }
inline void print_uint(uint64_t val, unsigned base, FILE *f=stdout)                            { uint_printer p(val,base,0,' ',f); p.print(); }
inline void print_uint(uint64_t val, unsigned minlen, char pad, FILE *f=stdout)                { uint_printer p(val,10,minlen,pad,f); p.print(); }
inline void print_uint(uint64_t val, unsigned base, unsigned minlen, char pad, FILE *f=stdout) { uint_printer p(val,base,minlen,pad,f); p.print(); }

/** @name non-overloaded names for use with Number class in vclasses.hpp @{ */
inline void cprint_int(int64_t val, FILE* f)   { int_printer p(val,10,0,' ',f); p.print(); }
inline void cprint_uint(uint64_t val, FILE* f) { uint_printer p(val,10,0,' ',f); p.print(); }
inline void cprint_double(double val, FILE* f) { ifprintf(f, "%g", val); }
inline void cprint_mydate(mydate d, FILE *f)      { d.print(f, mydate::PT_DATE); }
inline void cprint_mydatetime(mydate d, FILE *f)  { d.print(f, mydate::PT_DATETIME); }
inline void cprint_mytime(mydate d, FILE *f)      { d.print(f, mydate::PT_TIME); }
inline void cprint_mytimestamp(mydate d, FILE *f) { d.print(f, mydate::PT_DATETIME); }
/** @} */



/**
 * @brief Abstract FILE* stream functions so we can fake one with a string.
 *
 * Implementations of this interface class will save the most recently read
 * character to enable checking how an item is terminated and to detect
 * escaped characters.
 */
class IStreamer
{
public:
   virtual int getc(void) = 0;
   virtual bool eof(void) const = 0;    /**< eof (end-of-file) standard C value. */
   virtual int recent(void) const = 0; /**< Forces derived class to track recent char. */
};

/**
 * @brief Wrap FILE* as an IStreamer.
 *
 * Use an open FILE* stream as the source of an IStreamer.
 *
 * NOTE: This class does not close the stream when finished, as appropriate
 * for stdin, but remember to close the stream if it was acquired through fopen().
 */
class StrmStreamer : public IStreamer
{
protected:
   FILE *m_file;
   int  m_recent;
public:
   StrmStreamer(FILE *f) : m_file(f), m_recent(0) { }
   
   virtual int getc(void)            { return (m_recent=ifgetc(m_file)); }
   virtual bool eof(void) const      { return ifeof(m_file); }
   virtual int recent(void) const    { return m_recent; }
};

/** @brief Fake FILE* stream using a const char* variable. */
class StringStreamer : public IStreamer
{
protected:
   const char *m_string;
   const char *m_ptr;
   int         m_recent;
public:
   StringStreamer(const char *str)
      : m_string(str), m_ptr(str), m_recent(-2)  { }

   /**
    * @brief Simulate fgetc(stdio) with a const char* string.
    *
    * EOF condition exists after retrieving the '\0' terminator,
    * so we start m_recent below 0 so the recent function returns
    * something usable if called before the first getc().
    */
   virtual int getc(void)
   {
      if (m_recent!=EOF)
      {
         m_recent = static_cast<int>(*m_ptr++);
         if (!m_recent)
            m_recent=EOF;
      }
      
      return m_recent;
   }
   virtual bool eof(void) const      { return m_recent==EOF; }
   virtual int recent(void) const    { return m_recent; }
};


/**
 * @brief Use a streamer to separate a stream into discrete segments
 * based on a terminator.
 *
 * @todo Allow the constructor to accept more than one eoi character.
 */
class SegmentStreamer : public IStreamer
{
protected:
   IStreamer &m_str;
   char      m_eoi;     /**< end-of-input for segment-terminating character. */
public:
   const static char s_group_separator;
   const static char s_record_separator;
   const static char s_unit_separator;

   SegmentStreamer(IStreamer &str, char eoi=s_unit_separator)
      : m_str(str), m_eoi(eoi)   { }

   virtual int getc(void);
   virtual bool eof(void) const;
   virtual int recent(void) const { return m_str.recent(); }

   void eat_value(void);
};


int64_t int_from_stream(IStreamer &s);
uint64_t uint_from_stream(IStreamer &s);
double double_from_stream(IStreamer &s);
mydate mydate_from_stream(IStreamer &s);

inline int64_t int_from_string(const char *str)
{
   StringStreamer s(str);
   return int_from_stream(s);
}

inline uint64_t uint_from_string(const char *str)
{
   StringStreamer s(str);
   return uint_from_stream(s);
}


void _ltoccp(long val, unsigned bufflen, const IGeneric_Callback_Const_Pointer<char> &cb);

/**
 * @brief Convenient function to convert a long value to a string.
 *
 * This function creates the callback object from Func&, which can be a lambda.
 * It is faster to declare the bufflen, but if you're not sure, a bufflen==0
 * will use log10() to calculate the necessary buffer size.
 *
 * This is a callback function that returns void, but calls f() with the
 * completed string as the means to return the value.
 */
template <typename Func>
void ltoccp(long val, const Func &f, unsigned bufflen=0)
{
   Generic_User_Const_Pointer<char,Func> user(f);
   if (bufflen==0)
   {
      // log10(1000) == 3, so add 1 for the '1' and 1 for the terminating '\0'.
      // If val is negative, add 1 more for the sign.
      if (val<0)
         bufflen = 3 + log10(-val);
      else
         bufflen = 2 + log10(val);
   }

   _ltoccp(val, bufflen, user);
}


#define MAX_TEMP_SCHEMA_PATH 50
const char *temp_schema_path(void);

char *make_random_string(char *buff, int len);


#endif
