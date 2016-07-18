// -*- compile-command: "g++ -DINCLUDE_PRANDSTR_MAIN -std=c++11 -Wall -Werror -Weffc++ -pedantic -ggdb -o prandstr prandstr.cpp"  -*-

#include <string.h>
#include <math.h>        // for pow() and log10()
#include <alloca.h>

#include <stdexcept>

#include <sys/stat.h>    // for mkdir()
#include <sys/types.h>   // for mkdir()

#ifdef INCLUDE_PRANDSTR_MAIN
#include "vclasses.hpp"
#endif

// For some reason, prandstr.hpp must be included last, or at least before vclasses.hpp.
// Because of this, I do a conditional include just above to ensure the order.  I can't
// wait until the main include because the prandstr.hpp would have already been
// included, causing a compile failure.
#include "prandstr.hpp"


const char SegmentStreamer::s_group_separator = '\035';
const char SegmentStreamer::s_record_separator = '\036';
const char SegmentStreamer::s_unit_separator = '\037';

const char *float_allowed = "eE.-0123456789";
const char *int_allowed = &float_allowed[3];
const char *uint_allowed = &float_allowed[4];


void print_char_as_xml(char c, FILE* out)
{
   switch(c)
   {
      case '\'':
         fputs("&apos;", out);
         break;
      case '"':
         fputs("&quot;", out);
         break;
      case '<':
         fputs("&lt;", out);
         break;
      case '>':
         fputs("&gt;", out);
         break;
      case '&':
         fputs("&amp;", out);
         break;
      default:
         fputc(c, out);
         break;
   }
}

void print_str_as_xml(const char *str, FILE *out)
{
   if (str)
   {
      while(*str)
      {
         print_char_as_xml(*str,out);
         ++str;
      }
   }
}

/**
 * @brief Reports if str is in the list.
 *
 * Reports if a string is represented in a list of strings.
 *
 * @param str The string for which to search.
 * @param list An array of const char* whose final element is null.
 *
 * An example of use:
 * @code
 const char **arr_list[] = {"table", "record", "form", nullptr};
 if (string_in_list("table", arr_list))  // returns true
    printf("\table\" found in list.\n");
 if (string_in_list("Table", arr_list))  // returns false
    printf("\"Table\" found in list.\n");
 * @endcode
 */
bool string_in_list(const char *str, const char * const* list)
{
   while (*list)
   {
      if (strcmp(str, *list)==0)
         return true;
      ++list;
   }
   return false;
}


void mydate::print(FILE* f, PRINT_TYPE pt)
{
   if (pt % 2)
   {
      print_uint(m_year,f);
      fputc('-',f);
      print_uint(m_month,2,'0',f);
      fputc('-',f);
      print_uint(m_day,2,'0',f);
   }
   if (pt==PT_DATETIME)
      fputc('T',f);
   if (pt / 2)
   {
      print_uint(m_hour,2,'0',f);
      fputc(':',f);
      print_uint(m_minute,2,'0',f);
      fputc(':',f);
      print_uint(m_second,2,'0',f);
   }
}


/**
 * @brief Get characters one-by-one, stopping at end-of-file (EOF) or
 * end-of-input (eoi).
 *
 * A '+' character is converted to a space value.  A true '+' value should
 * be sent as %2B.
 *
 * Special processing occurs if a '%' is encountered.  If the next character
 * is also a '%', a single '%' is returned.  Otherwise, the following two
 * characters are converted as a hex string to an integer value.
 *
 * It is assumed that the character string has come from an http form
 * submission, which would have valid hex values.  Thus, I'm not checking
 * for inappropriate characters like alpha values over 'F' or 'f' or other
 * non-numeric values.
 */
int SegmentStreamer::getc(void)
{
   int val = m_str.getc();
   if (val==(EOF) || val==m_eoi)
      return EOF;
   else if (val=='+')
      return ' ';
   else if (val=='%')
   {
      int val = m_str.getc();
      if (val=='%')
         return '%';
      else
      {
         int rval = 0;
         int arr[2] = { val, m_str.getc() };
         for (int i=0; i<2; ++i)
         {
            rval = rval * 16;
            int cval = arr[i];
            if (cval<='9')
               rval += cval - '0';
            else if (cval<='F')
               rval += 10 + cval - 'A';
            else if (cval<='f')
               rval += 10 + cval - 'a';
         }
         return rval;
      }
   }
   else
      return val;
}

/**
 * @brief Test if last read character marks the end of the current value.
 */
bool SegmentStreamer::eof(void) const
{
   int recent = m_str.recent();
   return recent==(EOF) || recent==m_eoi;
}

/**
 * @brief Retrieve and discard the remainer of the current value.
 *
 * This function is used to keep input stream in sync.  When it is
 * determined that a value is not needed or should not be used, calling
 * this member function will read characters up to the terminator,
 * leaving the stream pointer positioned at the beginning of the next
 * value.
 */
void SegmentStreamer::eat_value(void)
{
   while(!eof())
      getc();
}

/**
 * @brief Implementation of recursive print function.
 *
 * In order for int_printer::print() to work, the virtual print()
 * function cannot be recursive or the int_print version will keep
 * calling int_print::print() when the execution should have been
 * passed off to unit_print::print() after dealing with the sign.
 */
void uint_printer::_print(void)
{
   uint8_t pos = m_val % m_base;
   if (m_val == pos)     // last digit, print padding and sign
   {
      if (m_negative)
      {
         // count the sign if negative with padding
         --m_minlen;
         
         if (m_pad=='0')
            fputc('-',m_out);
      }
      
      for (w_i=1; w_i<m_minlen; ++w_i)
         fputc(m_pad,m_out);

      if (m_negative && m_pad!='0')
         fputc('-',m_out);
   }
   else
   {
      m_val /= m_base;
      --m_minlen;
      // recursively call:
      _print();
   }

   if (pos<10)
      pos += '0';
   else if (pos<36)
      pos += 'A'-10;
   else if (pos<62)
      pos += 'a'-36;

   fputc(pos,m_out);
}

/**
 * @brief Reads characters from the stream, converting to an integer value.
 *
 *
 * After skipping non-numeric characters, reads characters until first non-
 * numeric character OR until it reads a terminating character or it
 * reads EOF.
 *
 * This function is intended for use in cgi/fastcgi, so it considers '&'
 * to be a terminating character.  
 */
int64_t int_from_stream(IStreamer &s)
{
   int64_t v = 0;
   bool negative = false;
   int ch;
   const char* found_ch;
   while ((ch=s.getc())!=EOF)
   {
      found_ch = strchr(int_allowed, ch);
      if (found_ch)
      {
         if (*found_ch=='-')
         {
            // if found after number started, this terminates the number:
            if (v)
               break;
            // otherwise, toggle negative (handle double negative):
            else
               negative = ~negative;
         }
         else if (*found_ch)
            v = v*10 + *found_ch - '0';
      }
      else if (v)
         break;
      // if encountered '-' earlier, it wasn't a sign, so reset negativity.
      else if (negative)
         negative = false;
   }
   return negative ? -v : v;
}

uint64_t uint_from_stream(IStreamer &s)
{
   uint64_t v = 0;
   int ch;
   const char *found_ch;
   while ((ch=s.getc())!=EOF)
   {
      found_ch = strchr(uint_allowed, ch);
      if (found_ch)
         v = v*10 + *found_ch - '0';
      else if (v)
         break;
   }
   return v;
}

double double_from_stream(IStreamer &s)
{
   enum STATE
   {
      NOT_FOUND=1,    /**< In pre-number...skipping non-number characters */
      START_WHOLE,    /**< About to start whole mode, but nothing gotten yet. */
      IN_WHOLE,       /**< After sign (minus), reading numerals. */
      IN_FRACTION,    /**< After '.' if any */
      START_EXPONENT, /**< After 'E' or 'e', but before numerals or sign. */
      IN_EXPONENT,    /**< Reading numerals of the exponent. */
      DONE            /**< Finished, found end-of-stream or non-allowed character. */
   };
   
   STATE state = NOT_FOUND;
   bool negative = false;
   bool neg_exponent = false;
   uint64_t whole = 0;
   uint64_t fraction = 0;
   int32_t exponent = 0;  /**< signed to allow changing sign if negative */
   int fraction_digits = 0;
   
   double v = 0.0;
   int ch;
   const char *found_ch;

   // ensure this consumes the entire item so streamer points at next item:
   while ((ch=s.getc())!=EOF)
   {
      if (state==DONE)
         continue;
      
      found_ch = strchr(float_allowed,ch);

      if (!found_ch)
      {
         if (state>START_WHOLE)
         {
            state = DONE;
            break;      // skip the rest of the loop
         }
         else
         {
            state = NOT_FOUND;
            continue;   // skip the rest of the loop
         }
      }
      
      if (*found_ch=='-')
      {
         if (state<IN_WHOLE)
            negative = !negative;
         else if (state==START_EXPONENT)
            neg_exponent = !neg_exponent;
         else
            state = DONE;
      }
      else if (*found_ch=='.')
      {
         if (state<=IN_WHOLE)
            state = IN_FRACTION;
         else
            state = DONE;
      }
      else if (*found_ch=='e' || *found_ch=='E')
         state = START_EXPONENT;
      else
      {
         switch(state)
         {
            case NOT_FOUND:
            case START_WHOLE:
               state = IN_WHOLE;
            case IN_WHOLE:
               whole = (whole*10) + *found_ch - '0';
               break;
            case IN_FRACTION:
               ++fraction_digits;
               fraction = (fraction*10) + *found_ch - '0';
               break;
            case START_EXPONENT:
               state = IN_EXPONENT;
            case IN_EXPONENT:
               exponent = (exponent*10) + *found_ch - '0';
               break;
            default:
               break;
         }
      }
   }

   // printf("\n%c%lu.%lue%d\n",
   //        (negative?'-':'+'),
   //        whole, fraction,exponent);

   // I should use static_cast, but that makes the line is too long.
   // I'm using old style casts for readability.
   v = (double)whole +
      ((double)fraction / pow(10.0,(double)fraction_digits));
   if (exponent)
   {
      if (neg_exponent)
         exponent = -exponent;
      v *= pow(10.0, (double)exponent);
   }

   return negative?-v:v;
}

mydate mydate_from_stream(IStreamer &s)
{
   mydate     rval;
   unsigned   *arr[6] = { &rval.m_year, &rval.m_month, &rval.m_day,
                        &rval.m_hour, &rval.m_minute, &rval.m_second };
   char       terminators[6] = {'-','-','T',':',':',' '};
   char       ch;
   const char *digit;

   int index = -1;

   while ((ch=s.getc())!=EOF)
   {
      if (index>5)
         continue;
      
      digit = strchr(uint_allowed, ch);

      if (digit)
      {
         if (index<0)
            ++index;
         
         *arr[index] *= 10;
         *arr[index] += *digit - '0';
      }
      else
      {
         if (index<0)
            continue;
         else if (ch==terminators[index])
            ++index;
         else
            index = 6;
      }
   }

   return rval;
}


/**
 * @brief Obscured function for converting Long-TO-Const-Char-Pointer ltoccp.
 *
 * This function does not check len for suitable size, and the result
 * is undefined if the length is not sufficient.
 *
 * For convenience, code should call ltoccp, which takes a lambda instead of
 * the somewhat non-intuitive IGeneric_Callback_Const_Pointer<char>.
 */
void _ltoccp(long val, unsigned len, const IGeneric_Callback_Const_Pointer<char> &cb)
{
   char *buff = static_cast<char*>(alloca(len));
   char *ptr = &buff[len-1];
   *ptr = '\0';
   if (val==0)
      *--ptr = '0';
   else
   {
      bool negative = false;
      char *end = buff;
      if (val<0)
      {
         // set flag, reverse sign, make room for '-':
         negative = true;
         val = 0 - val;
         ++end;
      }

      while (val>0 && --ptr>=end)
      {
         *ptr = '0' + val%10;
         val /= 10;
      }

      if (negative)
         *--ptr = '-';
   }
   cb(ptr);
}

const char *s_schema_dirname = "/schemafw";

// Leave enough for s_schema_dirname + '\0'  after
// up to 40 character of temp path name:
char g_schema_path[MAX_TEMP_SCHEMA_PATH] = {0};

/**
 * @brief Gets the pathname to a schemafw directory under
 * the system temporary directory, creating it if necessary.
 */
const char *temp_schema_path(void)
{
   if (*g_schema_path=='\0')
   {
      const char *tdir = nullptr;
      const char *envarr[] = { "TMPDIR","TMP","TEMP","TEMPDIR","USERPROFILE",nullptr };
      const char **loc = envarr;
      while (*loc && !(tdir=getenv(*loc)))
         ++loc;

      if (!tdir)
         tdir = "/tmp";

      int len = strlen(tdir);
      if (len + strlen(s_schema_dirname) + 1 >= sizeof(g_schema_path))
         throw std::runtime_error("temp_schema_path: path name too long.");

      memcpy(g_schema_path, tdir, len);
      strcpy(g_schema_path+len, s_schema_dirname);
      
      int result = mkdir(g_schema_path, 0777);
      if (!(result==0 || errno==EEXIST))
      {
         fprintf(stderr, "temp_schema_path mkdir failed: \"%s\"\n", strerror(errno));
         throw std::runtime_error("temp_schema_path: mkdir failed");
      }
   }

   return g_schema_path;
}

/**
 * @brief will make a random string from the system entropy file /dev/urandom.
 *
 * The string length will be one less than the len argument, to leave
 * room for the terminating \0.
 *
 * The random characters will be shifted to upper- and lower-case alpha-numeric.
 */
char *make_random_string(char *buff, int len)
{
   FILE *f = fopen("/dev/urandom", "r");

   if (f)
   {
      char *ptr = buff;
      char *end = buff + len - 1;

      while (ptr < end)
      {
         int val = fgetc(f) % 62;
         if (val < 10)
            *ptr = '0' + val;
         else if (val < 36)
            *ptr = 'A' + val-10;
         else
            *ptr = 'a' + val-36;

         ++ptr;
      }

      *ptr = '\0';
      
      fclose(f);
   }

   return buff;
}



#ifdef INCLUDE_PRANDSTR_MAIN

#include <stdio.h>
#include "vclasses.cpp"

void test_int_printing(void)
{
   fputs("About to test uint_printer.\n", stdout);
   print_int(12345);
   fputc('\n',stdout);

   int64_t tval = -12345;

   fputs("\nAbout to test int_printer:   '", stdout);
   print_int(tval,8,'0');
   fputs("\nTesting with space padding:  '", stdout);
   print_int(tval,8,' ');
   printf("\nSame number, printf(%%8ld)    '%8ld", tval);
   printf("\nSame number, printf(%%08ld)   '%08ld", tval);
   fputc('\n',stdout);

   fputs("\nTest printing a 0 with 02 padding\n", stdout);
   print_int(0,2,'0');

   fputc('\n',stdout);
}


/**
 * @brief Test reading ints from stream.
 *
 * It's not so easy to test reading from stdin, so I'm just
 * passing this source file as a test to avoid having to
 * ensure the name and existence of a separate test file.
 */
void test_stream_to_int(void)
{
   FILE *f = fopen("vclasses.cpp", "r");
   if (f)
   {
      StrmStreamer s(f);
      while (!s.eof())
      {
         int64_t rval = int_from_stream(s);
         printf("We got a %ld from the stream.\n",  rval);
      }
      fclose(f);
   }
}

void test_stream_to_uint(void)
{
   FILE *f = fopen("vclasses.cpp", "r");
   if (f)
   {
      StrmStreamer s(f);
      while (!s.eof())
      {
         uint64_t rval = uint_from_stream(s);
         printf("We got a %lu from the stream.\n",  rval);
      }
      fclose(f);
   }
}


void test_string_to_int(void)
{
   const char *val = "12345-55454 -454";
   StringStreamer s(val);
   while (!s.eof())
   {
      int64_t rval = int_from_stream(s);
      printf("We got a %ld from \"%s\"\n", rval, val);
   }
}

void test_string_to_double(void)
{
//   const char *val = "5.12e-3 3.14159265358979 12345.99  5.12e5";
   const char *val = "2.0e0 3e02 4.321768e3 -5.3e4 2e-1 7.51e-9 0.00008";
   StringStreamer s(val);
   fputc('\n', stdout);
   while (!s.eof())
   {
      double rval = double_from_stream(s);
      printf("%f from \"%s\"\n", rval, val);
   }
}

void test_string_to_date(void)
{
   const char *test = "2015-06-01T04:11:00";
   StringStreamer s(test);
   mydate md = mydate_from_stream(s);

   printf("md fields:\n"
          "year:   %4u\n"
          "month:  %02u\n"
          "day:    %02u\n"
          "hour:   %02u\n"
          "minute: %02u\n"
          "second: %02u\n",
          md.m_year, md.m_month, md.m_day, md.m_hour, md.m_minute, md.m_second);
   
}


struct REC
{
   unsigned m_id;
   char     m_family[80];
   mydate   m_bday;
   REC(void) : m_id(0), m_family(), m_bday() { }
};

void test_parse_cgi_data(void)
{
   const char *qstring = "id=12&family=jungmann&bday=1960-04-21";
   StringStreamer s(qstring);
   SegmentStreamer s_name(s,'=');
   SegmentStreamer s_value(s,'&');

   REC rec;

   char name[80];
   ai_text aname(name,80);

   ai_ulong ai_id(&rec.m_id);
   ai_text  ai_family(&rec.m_family[0], 80);
   ai_date  ai_bday(&rec.m_bday);

   while (!s_name.eof())
   {
      aname.set_from_streamer(s_name);

      if (aname=="id")
         ai_id.set_from_streamer(s_value);
      else if (aname=="family")
         ai_family.set_from_streamer(s_value);
      else if (aname=="bday")
         ai_bday.set_from_streamer(s_value);
   }


   printf("id:     %d\n"
          "family: %s\n"
          "bday:   ",
          ai_id.get_value(),
          ai_family.str());

   ai_bday.print(stdout);
   fputc('\n',stdout);
}

void test_ltoccp(long val)
{
   auto f = [&val](const char *str)
   {
      printf("%ld was converted to \"%s\"\n", val, str);
   };

   // We don't know the size of val, so allow the function to
   // calculate the buffer length (leave last parameter blank).
   ltoccp(val, f);
}

void test_string_in_list(void)
{
   const char *list[] = { "table", "record", "form", nullptr };
   
   if (string_in_list("table", list))
      printf("Found \"table\" in list.\n");
   else
      printf("Didn't find \"table\" in list.\n");

   if (string_in_list("Table", list))
      printf("Found \"Table\" in list.\n");
   else
      printf("Didn't find \"Table\" in list.\n");
}

void test_random_string(void)
{
   char buff[11];
   printf("The 10-character, 11-byte buffer has become \"%s\"\n",
          make_random_string(buff, 11));
}



int main(int argc, char **argv)
{
//test_int_printing();

// test_stream_to_int();
// test_stream_to_uint();

// test_string_to_int();
// test_string_to_double();


// test_string_to_date();
   // test_parse_cgi_data();

   // test_ltoccp(9876);

   // test_string_in_list();

   // printf("Testing get_schema_path, which returns \"%s\"\n", temp_schema_path());

   test_random_string();
   
   return 0;
}


#endif
