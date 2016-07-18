// -*- compile-command: "g++ -std=c++11 -Wall -Werror -Weffc++ -pedantic -ggdb -DINCLUDE_QS_MAIN -o qstringer qstringer.cpp" -*-

#ifndef QSTRINGER_HPP
#define QSTRINGER_HPP

#include <stdlib.h>
#include <unistd.h>       // for write() in print_name_at and print_value_at
#include "genericuser.hpp"
#include "vclasses.hpp"   // for EFFC_3 and EFFC_2


template <class Func>
void string_stepper(const char *str, char token, Func &cb);

class BaseStringer;

template <class Func> using BaseStringer_User = Generic_User_Const_Pointer<BaseStringer,Func>;

/**
 * @brief Stack-resident QString parser
 *
 * Processing QUERY_STRING must be done for each request in a long-running
 * application, so it is being implemented as a stack object that calls
 * IBaseStringerUser::use() when prepared.  It is designed so that it can't
 * be directly constructed or returned or moved.
 *
 * This class makes an array of pointers to the beginning of each query string
 * argument, but does not allocate memory to make copies of them.  As a result,
 * if you get a pointer to an argument, it will not be zero-terminiated because
 * it will be a pointer to a substring within a longer string.  If you need to
 * retrieve a value, use a combination of name_val_at and get_val_at
 *
 * A code example can be found @ref BaseStringer_Usage.
 */
class BaseStringer
{
private:
   const char **m_qarray;
   int        *m_len_names;
   int        *m_len_args;
   int        m_count;

protected:
   BaseStringer(const char **qarray, int *len_names, int *len_args, int count)
      : m_qarray(qarray), m_len_names(len_names),
        m_len_args(len_args), m_count(count)               {}

public:
   EFFC_3(BaseStringer)

   int count(void) const        { return m_count; }
   const char *raw(void) const  { return m_qarray[0]; }

   static char get_char_from_url(const char *c);

   /**
    * @name Functions for getting names and values.
    *
    * NOTE: These do not check array boundaries.  The result of an attempt
    * to access an out-of-range element is undefined.
    *
    * The process is to ask for the length of the item, allocate a buffer
    * for the item, then ask to fill the buffer.
    *
    * The get_xxx_at functions copy a string with a terminating zero. If
    * the buffer isn't large enough, characters will be omitted to
    * make room for the final NULL character.
    * @{
    */
   int get_longest_name(void) const;
   int get_longest_value(void) const;
   
   inline int arg_len_at(int index) const        { return m_len_args[index]; }
   inline int name_len_at(int index) const       { return m_len_names[index]; }
   inline bool has_val_at(int index) const       { return m_len_args[index] > m_len_names[index]; }
   inline bool has_value_at(int index) const     { return m_len_args[index] > m_len_names[index]; }
   inline int val_len_at(int index) const        { return m_len_args[index] - m_len_names[index]; }
   inline int value_len_at(int index) const      { return m_len_args[index] - m_len_names[index]; }


   long int get_int_value_at(int index) const;

   char *get_arg_at(int index, char *buff, int bufflen) const;
   char *get_name_at(int index, char *buff, int bufflen) const;
   char *get_val_at(int index, char *buff, int bufflen) const;
   inline char *get_value_at(int index, char *buff, int bufflen) const { return get_val_at(index,buff,bufflen); }
   inline const char *get_raw_name_at(int index) const                 { return m_qarray[index]; }
   /** @brief For BaseStringer_Setter to create a stream from this value. */
   const char *get_raw_val_at(int index) const;
   int get_index_of_name(const char *name) const;
   /**@}*/

   /** @brief Returns true if the name at position index is str.  No bounds checking. */
   inline bool is_name_at(int index, const char *str) const
   {
      return 0==strncmp(m_qarray[index], str, m_len_names[index]);
   }

   /** @brief Returns true if the value at position index is str.  No has_val_at or bounds checking. */
   inline bool is_value_at(int index, const char *str) const
   {
      return 0==strncmp(get_raw_val_at(index), str, val_len_at(index));
   }

   /** @brief Send name to file descriptor for index.  No bounds checking. */
   inline void print_name_at(int index, int fh) const   { write(fh, get_raw_name_at(index), name_len_at(index)); }
   /** @brief Send value to file descriptor for index.  No bounds checking or value confirmation. */
   inline void print_val_at(int index, int fh) const    { write(fh, get_raw_val_at(index), val_len_at(index)); }

   template <class F>
   void get_name_at(int index, F &lambda) const
   {
      int len = 1+name_len_at(index);
      char *buff = static_cast<char*>(alloca(len));
      get_name_at(index,buff,len);
      lambda(buff);
   }

   void t_get_value(int index, const IGeneric_Callback_Const_Pointer<char> &callback) const;
   void t_get_value(const char *name, const IGeneric_Callback_Const_Pointer<char> &callback) const;
   template <class Func>
   void get_value(const char *name, const Func &f)
   {
      t_get_value(name, Generic_User_Const_Pointer<char,Func>(f));
   }

   bool get_int_value(const char *name, long int &value) const;

   /** @name Static functions that build a BaseStringer object. @{ */
   static int count_args(const char *str, char separator='&');
   static void t_build(const char *str,
                       IGeneric_Callback_Const_Pointer<BaseStringer> &qsu,
                       char separator='&');

   /**@}*/
};

class QStringer : public BaseStringer
{
public:
   QStringer(const char **qarray, int *len_names, int *len_args, int count)
      : BaseStringer(qarray, len_names, len_args, count)      {}
   EFFC_3(QStringer)

   template <class Func>
   static void build(const char *str, const Func &f)
   {
      Generic_User_Const_Pointer<BaseStringer, Func> user(f);
      BaseStringer::t_build(str, user);
   }

   template <class Func>
   static void build(const Func &f)
   {
      Generic_User_Const_Pointer<BaseStringer, Func> user(f);
      BaseStringer::t_build(getenv("QUERY_STRING"), user);
   }

   bool anonymous_first_value(void) const;
};
class CStringer : public BaseStringer
{
public:
   CStringer(const char **qarray, int *len_names, int *len_args, int count)
      : BaseStringer(qarray, len_names, len_args, count)      {}
   EFFC_3(CStringer)

   template <class Func>
   static void build(const char *str, const Func &f)
   {
      Generic_User_Const_Pointer<BaseStringer, Func> user(f);
      BaseStringer::t_build(str, user, ';');
   }

   template <class Func>
   static void build(const Func &f)
   {
      Generic_User_Const_Pointer<BaseStringer, Func> user(f);
      BaseStringer::t_build(getenv("HTTP_COOKIE"), user, ';');
   }
};


#endif
