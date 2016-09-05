// -*- compile-command: "g++ -std=c++11 -Wall -Werror -Weffc++ -pedantic -ggdb -DINCLUDE_QS_MAIN -o qstringer qstringer.cpp" -*-

/**
 * @file qstringer.cpp
 *
 * If compiled within EMACS, an executable file will be generated with 
 * main() and supporting functions and classes to demonstrate and test
 * how this module is to be used.
 */

#include <assert.h>
#include <ctype.h>   // for isdigit
#include "string.h"
#include "qstringer.hpp"

template <class Func>
void string_stepper(const char *str, char token, Func &cb)
{
   const char *start = str;
   const char *end = str;    // set value for initial loop entry

   auto f = [&start, &end, &cb](void)
   {
      if (end)
      {
         int len = end - start;
         char *buff = static_cast<char*>(alloca(1+len));
         memcpy(buff,start,len);
         buff[len] = '\0';
         cb(buff);
      }
      else
         cb(start);
   };

   while (end)
   {
      end = strchr(start,token);
      f();
      if (end)
         start = end+1;
   }
}

/**
 * @page BaseStringer_Usage BaseStringer Usage
 *
 * @todo Rewrite this page to use the Generic_User classes.
 *
 * BaseStringer provides random access to query string arguments and cookies.
 *
 * As a stack-resident object, a BaseStringer object is not directly constructed,
 * but rather built in a static function and handed off to an object implmenting
 * an IBaseStringerUser interface that is passed into the static build function.
 *
 * Here is an usage example:
 *
 * @code
 * class QUser : public IBaseStringerUser
 * {
 * public:
 *    virtual void use(const BaseStringer *qs)
 *    {
 *       // The use function always called, even if no query string is found.
 *       // An empty or missing query string will pass qs==nullptr.
 *       if (qs)
 *       {
 *          // These are arbitrary lengths for demo purposes.
 *          // The programmer must ensure the size of the buffers
 *          // is sufficient to hold the needed values.
 *          char buff_name[40];
 *          char buff_value[80];
 *
 *          // Ask for the number of arguments.
 *          int count = qs->count();
 *
 *          // The programmer must ensure that appropriate indexes are
 *          // used to access query string arguments.  In the interest
 *          // of performance, no bounds-checking is done on element
 *          // requests.
 *          if (count==0)
 *             return;
 *
 *          // Get a name by position:
 *          qs->get_name_at(0, buff_name, 40);
 *
 *          // Get a value by position:
 *          // First check if a value exists at the position.
 *          if (qs->has_val_at(0))
 *             qs->get_val_at(0,buff_value,80);
 *
 *          // Note that in the following example,  we don't retrieve
 *          // the name value: it is known because it was the search
 *          // value.
 
 *          // Attempt to find a name-value pair.
 *          int index = qs->get_index_of_name("fname");
 *          if (index>=0)
 *             qs->get_val_at(index, buff_value, 80);
 *       }
 *    }
 * };
 * @endcode
 *
 * The BaseStringer::build function scans the query string for arguments.  The
 * address of the beginning of each argument is saved in an array, and the
 * length of the argument is saved.  If an '=' character is found, the number
 * of characters that preceeds the '=' are saved as well.  The name length is
 * used to copy or constraint search comparsions.
 */

/**
 * @brief Convert a percent-prefixed URL hex code to char.
 *
 * Specific function to convert a three-character array prefixed with %
 * into a char (8-bit unsigned) value.
 *
 * The function assumes valid input.  Ranges inferred by comparing the
 * character against increasing maximum values.
 */

char BaseStringer::get_char_from_url(const char *c)
{
   assert(*c=='%');
   char rval = 0;
   for (int i=1; i<3; i++)
   {
      if (rval)
         rval *= 16;
      
      char x = c[i];
      assert((x>='0' && x<='9') || (x>='A' && x<='F') || (x>='a' && x<='f'));
      if (x<='9')
         rval += x-'0';
      else if (x<='F')
         rval += 10+x-'A';
      else if (x<='f')
         rval += 10+x-'a';
   }
   return rval;
}

int BaseStringer::get_longest_name(void) const
{
   int max = 0;
   int *end = &m_len_names[m_count];
   for (int *p=m_len_names; p<end; ++p)
      if (*p > max)
         max = *p;

   return max;
}

int BaseStringer::get_longest_value(void) const
{
   int max = 0;
   int val;
   int *end = &m_len_names[m_count];
   
   int *pn = m_len_names;
   int *pa = m_len_args;
   for (; pn<end; ++pn, ++pa)
      if ((val=*pa-*pn) > max)
         max = val;
   
   return max;
}

long int BaseStringer::get_int_value_at(int index) const
{
   const char *ptr = get_raw_val_at(index);
   long int rval = 0;
   while (ptr && isdigit(*ptr))
   {
      rval *=10;
      rval += *ptr - '0';
      ++ptr;
   }
   return rval;
}

/** @brief bufflen limited, Get raw arg at index. Index not bounds checked. */
char *BaseStringer::get_arg_at(int index, char *buff, int bufflen) const
{
   const char *arg = m_qarray[index];
   int len = m_len_args[index];
   if (bufflen<=len)
      len = bufflen-1;

   memcpy(buff, arg, len);
   buff[len] = '\0';
   return buff;
}

/** @brief bufflen limited, Get raw name at index. Index not bounds checked. */
char *BaseStringer::get_name_at(int index, char *buff, int bufflen) const
{
   const char *arg = m_qarray[index];
   int len = m_len_names[index];
   if (bufflen<=len)
      len = bufflen-1;

   memcpy(buff, arg, len);
   buff[len] = '\0';
   return buff;
}

const char *BaseStringer::get_raw_val_at(int index) const
{
   int len_name = m_len_names[index];
   return m_qarray[index] + len_name + 1;
}

/** @brief bufflen limited, Get decoded value at index. Index not bounds checked. */
char *BaseStringer::get_val_at(int index, char *buff, int bufflen) const
{
   int len_name = m_len_names[index];
   int len_arg = m_len_args[index];
   
   if (len_arg > len_name)
   {
      const char *arg = m_qarray[index];
      const char *arg_end = arg + len_arg;  // identify end of value
      arg += len_name+1;                    // point to start of value

      // Calculate how many characters to copy, in case buff is too small:
      len_arg -= len_name+1;
      if (bufflen<=len_arg)
         len_arg = bufflen-1;

      char *out = buff;
      char *out_end = buff + len_arg;       // identify end of output

      // Copy strings, converting hex codes along the way:
      while (arg < arg_end)
      {
         if (out < out_end)
         {
            if (*arg=='+')
               *out = ' ';
            else if (*arg=='%')
            {
               *out = get_char_from_url(arg);
               // add only enough that per-loop increment moves appropriately:
               arg+=2;
            }
            else
               *out = *arg;
            
            ++arg;
            ++out;
         }
      }
      *out = '\0';
      
      return buff;
   }
   else
      return nullptr;
}

int BaseStringer::get_index_of_name(const char *str) const
{
   size_t len = strlen(str);
   for (int i=0; i<m_count; ++i)
      if (len==static_cast<size_t>(m_len_names[i]))
         if (0==memcmp(static_cast<const void*>(str),
                       static_cast<const void*>(m_qarray[i]),
                       len))
            return i;
   return -1;
}


/** @brief Scan string for ampersands to return number of values in query_string. */
int BaseStringer::count_args(const char *str, char separator)
{
   int count = 0;
   if (str && *str)
   {
      ++count;
      while (*str)
      {
         if (*str==separator)
            ++count;
         ++str;
      }
   }
   return count;
}

/**
 * @brief Uses callback to return the value.
 *
 * This function allocates stack memory into which the value is copied.
 * Then the buffer is returned to the calling function through the callback.
 *
 * This function does no error checking, so only use this function if you have
 * confirmed that an element exists at the index you use.
 *
 * As with other get_ functions, this only invokes callback if there is a value.
 */
void BaseStringer::t_get_value(int index, 
                            const IGeneric_Callback_Const_Pointer<char> &callback) const
{
   int len = val_len_at(index);
   if (len>0)
   {
      char *buff = static_cast<char*>(alloca(len+1));
      get_val_at(index, buff, len+1);
      callback(buff);
   }
}

/**
 * @brief Uses callback to return the value if found.
 *
 * This function searches for name, and if found and the value has a positive length,
 * allocates some stack memory into which the value is copied.  Then the buffer is
 * returned to the calling function through the callback.
 *
 * As with other get_ functions, this only invokes callback if there is a value.
 */
void BaseStringer::t_get_value(const char* name,
                            const IGeneric_Callback_Const_Pointer<char> &callback) const
{
   int index = get_index_of_name(name);
   if (index>=0)
      t_get_value(index,callback);
}

/**
 * @brief Shortcut/direct access to an integer value.
 *
 * @param name tag name for which to search.
 * @param value Reference to variable in which to return the value
 * @return true if founct, false if not.
 *
 * A convenience method to avoid having to allocate a buffer to get the
 * value of a known integer type
 *
 * It seems easier to write code that checks for a bool return value
 * than compare an interger return value against an arbitrary _NOT FOUND_
 * value.
 */ 
bool BaseStringer::get_int_value(const char *name, long int &value) const
{
   int index = get_index_of_name(name);
   if (index>=0)
   {
      value = get_int_value_at(index);
      return true;
   }
   else
      return false;
}

/**
 * @brief Assemble data to construct a BaseStringer object.
 *
 * Stack-allocate and populate 3 arrays for BaseStringer object.
 * When the object is ready, call the IBaseStringerUser method
 * to make available the query string values.
 *
 * The function works with a char string instead of the
 * QUERY_STRING environment variable to enable testing the
 * conversions outside of a CGI environment.
 *
 * NOTE: The @p user callback function is ALWAYS called, either
 * with an object or nullptr if @p str is null or empty;
 */
void BaseStringer::t_build(const char *str,
                        IGeneric_Callback_Const_Pointer<BaseStringer> &user,
                        char separator)
{
   int count = count_args(str,separator);
   if (count)
   {
      const char **qarray = static_cast<const char**>(alloca(count*sizeof(const char*)));
      int *len_names = static_cast<int *>(alloca(count*sizeof(int)));
      int *len_args = static_cast<int *>(alloca(count*sizeof(int)));
      int index = 0;
      int len_name = 0;
      int len_arg = 0;
      bool getting_name = true;

      qarray[index] = str;
      while (*str)
      {
         if (*str==separator)
         {
            // Finish with current arg:
            len_names[index] = len_name;
            len_args[index ] = len_arg;

            // Start next arg:
            len_name = len_arg = 0;
            getting_name = true;
            
            // skip the separator:
            ++str;
            // skip initial whitespace:
            while (*str && isspace(*str))
               ++str;
            // save as the beginning of the next item:
            qarray[++index] = str;
            // skip the str incrementer at the bottom of this while loop:
            continue;
         }
         else if (*str=='=')
         {            
            getting_name = false;
            ++len_arg;
         }
         else
         {
            ++len_arg;
            if (getting_name)
               ++len_name;
         }

         ++str;
      }

      len_names[index] = len_name;
      len_args[index ] = len_arg;
      
      BaseStringer qs(qarray, len_names, len_args, count);
      user(&qs);
   }
   else
      user(nullptr);
}

/**
 * @brief Checks the query string to see if the first value is anonymous.
 *
 * The algorithm I'm using is to scan the string to the first ampersand,
 * equals sign, or the end of the string.
 *
 * The first value is anonymous if the first of the three is an equals sign.
 * If we find an ampersand, the first value is named, if we find the end of
 * the string, there are no values.
 */
bool QStringer::anonymous_first_value(void) const
{
   const char *str = get_raw_name_at(0);
   if (str && *str)
   {
      const char *ptr = str;
      while(*ptr)
      {
         if (*ptr=='=')
            return true;
         else if (*ptr=='&')
            break;

         ++ptr;
      }
   }

   return false;
}


#ifdef INCLUDE_QS_MAIN

#include <stdio.h>

void use_qstringer(const BaseStringer *qs)
{
   char buff_name[40];
   char buff_value[80];
   int count = qs->count();
   if (count>0)
   {
      const char *searchname = "address";
      printf("Searching for \"%s\":\n", searchname);
      int index = qs->get_index_of_name(searchname);
      if (index<0)
         printf("Did not find %s.\n", searchname);
      else if (qs->has_val_at(index))
      {
         qs->get_val_at(index,buff_value,80);
         printf("Found \"%s\" having a value of \"%s\".\n",
                searchname,
                buff_value);
      }
      else
         printf("Found \"%s\" without a value.\n", searchname);
            
      for (int i=0; i<count; ++i)
      {
         qs->get_name_at(i,buff_name,40);
         if (qs->has_val_at(i))
         {
            printf("Raw value at %d is \"%s\"\n", i, qs->get_raw_val_at(i));
                  
            qs->get_val_at(i,buff_value,80);
            printf("For %d, \"%s\" = \"%s\"\n", i, buff_name, buff_value);
         }
         else
            printf("for %d, \"%s\"\n", i, buff_name);
      }
   }
}

/** @brief Simple sample implementation of IBaseStringerUser. */
class MyUser : public IGeneric_Callback_Const_Pointer<BaseStringer>
{
public:
   MyUser(void) { }
   virtual void operator()(const BaseStringer *qs) const
   {
      if (qs)
      {
         use_qstringer(qs);
      }
   }
};


const char *qsample = "SpecsName&fname=Bozo&lname=LePitre&address=123+Silly+Road,%20Somewhere%20AK%2099900";

const char *csample = "SessionID=17; SessionHash=94ZFdPf5AVwXIs8I8YWYiRTgpigjEl72";


/**
 * @brief Old pattern to get a BaseStringer.
 *
 * This pattern depends instantiating a class that has implemented the
 * virtual use() function for a callback.  This pattern is still value,
 * but the new method, get_qstringer_lambda is an improvement, in my
 * opinion, because the code that uses the BaseStringer is in the same
 * function that requests the BaseStringer, making it easier to understand
 * and debug.
 */
void get_qstringer_old(void)
{
   MyUser mu;
   QStringer::t_build(qsample, mu, '&');
}

/**
* @brief New pattern to get a BaseStringer with a lambda function.
*
* This function improves on the old pattern for two reasons
* -# The code that uses the BaseStringer is in the same function that requested it.
* -# The code is more terse because we're using a template class to make a
*    IBaseStringerUser implementation.
*
* @sa get_qstringer_old
*/
void get_qstringer_lambda(void)
{
   auto fLoc = [](const BaseStringer *qs)
   {
      if (qs)
         use_qstringer(qs);
   };
   QStringer::build(qsample, fLoc);
}

void get_cstringer_lambda(void)
{
   auto fLoc = [](const BaseStringer *qs)
   {
      if (qs)
         use_qstringer(qs);
   };
   CStringer::build(csample, fLoc);
}

void test_string_stepper(const char *str, char token)
{
   int count = 0;
   auto f = [&count](const char *s)
   {
      ++count;
      printf("%2d: \"%s\"\n", count, s);
   };

   printf("Running string_stepper test with \"%s\"\n", str);
   string_stepper(str,token,f);
}



int main(int argc, char **argv)
{
//   get_qstringer_old();
   get_qstringer_lambda();
   get_cstringer_lambda();

   // test_string_stepper("default.spec:main_display",':');
   // test_string_stepper("p_lname=Charles&p_lname=Jungmann&p_address=620+Lanewood+Lane+Plymouth+MN+55447", '&');
   // test_string_stepper(qsample, '&');

   
   
   return 0;
}



#endif
