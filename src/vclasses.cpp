// -*- compile-command: "g++ -DINCLUDE_VCLASSES_MAIN -std=c++11 -Wall -Werror -Weffc++ -pedantic -ggdb -o vclasses vclasses.cpp"  -*-

#include <math.h>  // for pow()
#include <stdexcept>
#include "vclasses.hpp"

extern "C" void __cxa_pure_virtual(void)
{
   throw Pure_Virtual_Exception(); 
}

void throw_unmatchable(const char *where, const IClass &lhs, const IClass &rhs)
{
   char buff[120];
   sprintf(buff,
           "IClass::%s unmatchable types.  lhs:t=%d,c=%d; rhs:t=%d,c=%d.\n",
           where,
           lhs.vtype(), lhs.vclass(),
           rhs.vtype(), rhs.vclass());

   throw std::runtime_error(buff);
}

bool IClass::s_destruct_flag = false;

void IClass::get(const IClass &rhs)
{
   if (rhs.vtype()==vtype())
      set_from_eqtype(rhs);
   else if (rhs.vclass()==vclass())
      set_from_eqclass(rhs);
   else
      throw_unmatchable("get", *this, rhs);
}

void IClass::put(IClass &lhs) const
{
   if (lhs.vtype()==vtype())
      lhs.set_from_eqtype(*this);
   else if (lhs.vclass()==vclass())
      lhs.set_from_eqclass(*this);
   else
      throw_unmatchable("put", lhs, *this);
}


size_t buffer_copy(void *destination, size_t len_d, const void *source, size_t len_s)
{
   char       *d = static_cast<char*>(destination);
   char       *end_d = d + len_d;
   const char *s = static_cast<const char*>(source);

   if (len_s==static_cast<size_t>(-1))
   {
      // string-copy to first '\0':
      while (d<end_d && *s)
         *d++ = *s++;
      // add terminating null if room
      if (d<end_d)
         *d = '\0';
   }
   else
   {
      // Copy specified characters, up to destination limit:
      const char *end_s = s + len_s;
      while (d<end_d && s<end_s)
         *d++= *s++;
   }

   return static_cast<size_t>(d - static_cast<char*>(destination));
}

/**
 * @brief Puts the data characters out to the file pointer.
 *
 * This function is a bit unusual in that it uses get_buffer_length()
 * as the string length.  This works, and should continue to work,
 * because when an instance of String_Base is constructed in a printing
 * situation, it is build using the string length as the buffer length,
 * even if the buffer length of its source is different. As a result,
 * the buffer_length is usable to prevent printing overruns.
 */
void String_Base::print(FILE *f) const
{
   size_t len = get_string_length();
   const char *data = static_cast<const char*>(get_vdata());

   // we don't need to print the terminating \0.
   for (size_t i=0; *data && i<len; ++i)
      fputc(*data++, f);
}

void String_Base::print_xml_escaped(FILE *f) const
{
   size_t len = get_string_length();
   const char *data = static_cast<const char*>(get_vdata());

   // we don't need to print the terminating \0.
   for (size_t i=0; *data && i<len; ++i, ++data)
   {
      switch(*data)
      {
         case '\'':
            fputs("&apos;", f);
            break;
         case '\"':
            fputs("&quot;", f);
            break;
         case '&':
            fputs("&amp;", f);
            break;
         case '<':
            fputs("&lt;", f);
            break;
         case '>':
            fputs("&gt;", f);
            break;
         default:
            fputc(*data, f);
            break;
      }
      
   }
   
}

void String_Base::set_from_eqtype(const IClass &rhs)
{
   const char *source = static_cast<const char*>(rhs.get_vdata());
   char *target = const_cast<char*>(static_cast<const char*>(get_vdata()));
   char *end = target + get_buffer_length();

   while (*source && target<end)
      *target++ = *source++ ;
   if (target<end)
      *target = '\0';
}

size_t String_Base::set_from_streamer(IStreamer &str)
{
   char *ptr = const_cast<char*>(static_cast<const char*>(get_vdata()));
   char *end = ptr + get_buffer_length();
   
   int temp;
   int eof = EOF;
   size_t count=0;
   while (ptr<end)
   {
      temp = str.getc();
      if (temp==eof)
         break;
      else
      {
         ++count;
         *ptr++ = temp;
      }
   }
   *ptr = '\0';

   return count;
}


/***************************************************/
/* Beginning of EMACS compile-include code.        */
/* This code will only be included when compiling  */
/* from within EMACS.  Regular project builds will */
/* not include the following code.                 */
/***************************************************/
#ifdef INCLUDE_VCLASSES_MAIN

#include <limits>  // std::numeric_limits
#include <sys/resource.h>  // for rlimit stuff
#include <stdio.h>

#include "prandstr.cpp"

void print_five(const IClass &r1, const IClass &r2, const IClass &r3, const IClass &r4, const IClass &r5)
{
   fputs("print_five:\n" ,stdout);
   fputs("r1 = ", stdout);
   r1.print(stdout);
   fputs("\nr2 = ", stdout);
   r2.print(stdout);
   fputs("\nr3 = ", stdout);
   r3.print(stdout);
   fputs("\nr4 = ", stdout);
   r4.print(stdout);
   fputs("\nr5 = ", stdout);
   r5.print(stdout);
   fputc('\n', stdout);
}

void test_print_five(void)
{
   ri_utiny    rutiny(8);
   ri_ushort   rushort(16);
   ri_long     rlong(32);
   ri_longlong rlonglong(64);
   ri_double   rpi(3.1415926535);

   print_five(rutiny, rushort, rlong, rlonglong, rpi);
}

// class cuser : public IClassUser
// {
// protected:
//    iclass_caster f_caster;
//    void          *m_data;
//    size_t        m_len;
//    FILE          *m_print_stream;

// public:
//    cuser(iclass_caster f, void* d, size_t len)
//       : f_caster(f), m_data(d), m_len(len),
//         m_print_stream(nullptr) { }

//    /** @brief Implement IClassUser::use() */
//    virtual void use(IClass &c) { c.raw_print(m_print_stream); }

//    void raw_print(FILE *f)
//    {
//       m_print_stream = f;
//       (*f_caster)(m_data, m_len, *this);
//       m_print_stream = nullptr;
//    }

//    cuser &operator=(const IClass &rhs)
//    {
//       IClassGetter icg(f_caster);
//       icg.get(m_data, m_len, rhs);
//       return *this;
//    }

//    void put(IClass &target) const
//    {
//       IClassPutter icp(f_caster);
//       icp.put(m_data, m_len, target);
//    }

//    void print(FILE *out) const
//    {
//       IClassPrinter_raw icp(f_caster);
//       icp.print(m_data, m_len, out);
//    }
   
// };

struct Test
{
   int8_t   m_tiny;
   int16_t  m_short;
   int32_t  m_long;
   uint32_t m_ulong;
   double   m_double;
   mydate   m_date;
};

struct cuser
{
   iclass_caster m_caster;
   void          *m_data;
   size_t        m_length;

   // Internal printing IClass_User class
   class cprint : public IClass_User
   {
      FILE *m_out;
   public:
      cprint(FILE *out) : m_out(out)    { }
      virtual void use(IClass &v)       { v.print(m_out); }
      
      EFFC_3(cprint)
   };

   // Internal setting IClass_User class
   class cset : public IClass_User
   {
      const IClass &m_rhs;
   public:
      cset(const IClass &rhs) : m_rhs(rhs) { }
      virtual void use(IClass &lhs)        { m_rhs.put(lhs); }

      EFFC_3(cset)
   };
   
   void print(FILE* f)         { cprint c(f); (*m_caster)(c,m_data,m_length,m_length); }

   void put(const IClass &rhs) { cset c(rhs); (*m_caster)(c,m_data,m_length,m_length); }
};

void test_set_cast_and_print(void)
{
   Test test = { 8, 16, -32, 32, 3.14159, {2015,5,7} };

   fputs("Prepare an array of cuser elements and show "
         "the original settings of the test struct.\n", stdout);
   
   cuser icast[6] = {
      { &ai_tiny::cast_and_use, &test.m_tiny, 0 },
      { &ai_short::cast_and_use, &test.m_short, 0 },
      { &ai_long::cast_and_use, &test.m_long, 0 },
      { &ai_ulong::cast_and_use, &test.m_ulong, 0 },
      { &ai_double::cast_and_use, &test.m_double, 0 },
      { &ai_date::cast_and_use, &test.m_date, 0 }
   };

   int stop = static_cast<int>(sizeof(icast)/sizeof(cuser));
   for (int i=0; i<stop; i++)
   {
      printf("Value of element %d is \"", i);
      icast[i].print(stdout);
      fputs("\"\n", stdout);
   }

   // fputs("\nNow test the IClass 'get', 'put', and 'print_raw' functions.\n", stdout);
   
   // ri_long rlong;
   // icast[2].put(rlong);

   // int32_t vlong = rlong.get_value();
   // printf("Made a ri_long and used icast[2].put() to set its value.\n"
   //        "Then I used T Class_Number<T...>::get_value() to get "
   //        "the value to a int32_t variable.  This is necessary in order\n"
   //        "to access the results of a MySQL query.\n"
   //        "The value of this int32_t variable is %d\n", vlong);

   // icast[1] = rlong;
   
   // fputs("\nNow testing the '=' operator, setting icast[1] from the "
   //       "ri_long variable, then using cuser::print to output:\nicast[1] = \"", stdout);
   // icast[1].print(stdout);
   // fputs("\"\n", stdout);
   
}

// void test_set_and_print_iclass(void)
// {
//    Test test = { 8, 16, -32, 32, 3.14159, {2015,5,7} };

//    ai_tiny rftiny(test.m_tiny);
//    ai_short rfshort(test.m_short);
//    ai_long rflong(test.m_long);
//    ai_ulong rfulong(test.m_ulong);
//    ai_double rfdouble(test.m_double);
//    print_five(rftiny, rfshort, rflong, rfulong, rfdouble);

//    ai_date  dfdate(test.m_date);

//    fputs("Testing date print: ", stdout);
//    dfdate.raw_print(stdout);
//    fputs("\n\n", stdout);
   

//    ri_tiny rtiny(8);
//    ri_tiny rtiny2(12);
//    ri_short rshort(16);
//    ri_ulong rulong(32);

//    print_five(rtiny, rtiny2, rshort, rulong, rfdouble);
   

//    rtiny.set(ri_tiny(16));
//    rtiny2.set(ri_tiny(24));
//    rshort.set(ri_short(32));
//    rulong.set(ri_ulong(64));

//    print_five(rtiny, rtiny2, rshort, rulong, rfdouble);
   
// }

// char *top_o_stack = nullptr;
// char *save_a_stack = nullptr;
// char *now_a_stack = nullptr;

// void test_address_stuff(void)
// {
//    int tostack;

//    printf("Let's see some addresses:\n"
//           "global top_o_stack: %p\n"
//           "main tostack      : %p\n",
//           static_cast<void*>(&top_o_stack),
//           static_cast<void*>(&tostack));
// }



// void test_text_class(void)
// {
//    long lval = 100;
//    ai_long ail(&lval);
//    printf("Got long value %d\n", ail.get_value());
   
//    char *buff = static_cast<char*>(alloca(100));
//    ai_text ct(buff, 100);
//    ct.set_value("This is my text!");

   

//    fputs("Test, for Alias_Text, for assignment and printing:\n\"", stdout);
//    ct.raw_print(stdout);
//    fputs("\"\n", stdout);

// }

void test_string_classes(void)
{
   char buff[100];
   String_Virtual sv(buff,100,0);
   String_Const sc("Bogus, man!");

   sv.get(sc);

   fputs("After assignment of String_Const \"", stdout);
   sc.print(stdout);
   fputs(",\"\nassigned in turn to String_Virtual \"", stdout);
   sv.print(stdout);
   fputs(".\"\nHow did it work?\n", stdout);
}

void print_class_match(const char *type, const IClass &c)
{
   printf("%24s: %d (%d)\n", type, c.vtype(), c.vclass());
}

void test_class_matches(void)
{
   ri_tiny     rit(5);
   ri_short    ris(10);
   ri_long     ril(20);
   ri_longlong rill(40);

   print_class_match("tiny", rit);
   print_class_match("short", ris);
   print_class_match("long", ril);
   print_class_match("longlong", rill);

   ri_ushort   ruis(10);
   ri_ulong    ruls(20);

   print_class_match("unsigned short", ruis);
   print_class_match("unsigned long", ruls);

   char buff[20];
   strcpy(buff, "Hi dad");
   ai_text     atext(buff,20,strlen(buff));
   si_text     stext("Hi mom");

   print_class_match("ai_text", atext);
   print_class_match("si_text", stext);
}


int main(int argc, char **argv)
{
   test_print_five();
   test_set_cast_and_print();

   test_string_classes();
   test_class_matches();
   
   
   // test_text_class();
   
   // test_set_and_print_iclass();
   // test_address_stuff();
}


#endif // INCLUDE_VCLASSES_MAIN

