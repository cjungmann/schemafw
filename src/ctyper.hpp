// -*- compile-command: "g++ -std=c++11 -Wall -Werror -Weffc++ -pedantic -DINCLUDE_CTYPER_MAIN `mysql_config --cflags` -o ctyper ctyper.cpp `mysql_config --libs`" -*-

#ifndef CTYPER_HPP_SOURCE
#define CTYPER_HPP_SOURCE

#include <mysql.h>       // for enum_field_types
#include "vclasses.hpp"

/**
 * @brief Structure to hold type information for building BindStacks.
 *
 * This class also includes the _Types_ enum that contains the characters
 * used to build a _typestr_.
 *
 * @section TypestrDefinitions Typestr Definitions
 *
 * ### Signed Integer Types
 * The default integer type is 32-bit signed, so _i_ for integer
 * refers to that type.  Here are the integer types:
 * - __i__ 32-bit signed integer
 * - __j__ 16-bit signed integer
 * - __k__ 8-bit signed integer
 * - __l__ (lower-case L) 64-bit signed integer
 *
 * ### Signed Integer Types
 * Make upper-case the integer chars for unsigned versions of
 * the same size.
 * - __I__ (upper-case i) 32-bit signed integer
 * - __J__ 16-bit signed integer
 * - __K__ 8-bit signed integer
 * - __L__  64-bit signed integer
 *
 * ### Float Types
 * - __f__ (float) 32-bit floating point
 * - __d__ (double) 64-bit floating point
 *
 * ### String Type
 * The string type starts with a lower-case __s__, and is optionally followed by
 * numerals to indicate the length.  Without a length, it will be made 1000 bytes
 * long for MYSQL_TYPE_BLOB.
 *
 * Build a _typestr_ by stringing several type values together:
 * _Is32_ declares a BindStack with an unsigned 32-bit integer, followed by a
 * 32-character string.  An unsigned int is a common datatype for table keys,
 * so typestr will often begin with _I_.
 *
 */
struct _CType
{
   enum Types : char
   {
      TYPE_UNDEFINED = '\0',
      TYPE_NULL      = 'n',
      TYPE_DOUBLE    = 'd',
      TYPE_FLOAT     = 'f',

      TYPE_LONG      = 'i',
      TYPE_SHORT     = 'j',
      TYPE_TINY      = 'k',
      TYPE_LONGLONG  = 'l',

      TYPE_ULONG     = 'I',
      TYPE_USHORT    = 'J',
      TYPE_UTINY     = 'K',
      TYPE_ULONGLONG = 'L',

      TYPE_TIME      = 't',

      TYPE_STRING    = 's',
      TYPE_BLOB      = 'b'
   };

   static const char* s_allowed_types;

   enum_field_types m_sqltype;
   bool             m_is_unsigned;
   class_types      m_ctype;
   Types            m_stype;
   size_t           m_size;
   iclass_caster    m_caster;
   const char       *m_sql_name;
   const char       *m_param_data_type;

   inline enum_field_types get_sqltype(void) const { return m_sqltype; }
   inline bool is_unsigned(void) const          { return m_is_unsigned; }
   inline class_types get_ctype(void) const     { return m_ctype; }
   inline char get_stype(void) const            { return static_cast<char>(m_stype); }
   inline size_t get_size(void) const           { return m_size; }
   inline iclass_caster get_caster(void) const  { return m_caster; }
   inline const char *get_type_name(void) const { return m_sql_name; }

   inline bool operator==(const char *c) const  { return 0==strcmp(c, m_param_data_type); }
};

class CTyper
{
private:
   static const _CType s_types[];
   static const _CType *s_end;   /**< Address past last element for iterating elements. */
   static const int s_count;     /**< Number of elements in s_types array. */

   static CTyper s_instance;

   /** @brief Enforce singleton-ness. */
   CTyper() { }

public:
   static const CTyper& instance(void)    { return s_instance; }

   static const _CType* get(_CType::Types t);
   static const _CType* get(class_types t);
   static const _CType* get(enum_field_types t, bool is_unsigned);
   static const _CType* get(const char *data_type, const char *dtd_identifier=nullptr);
   inline static const _CType *get(const IClass *c) { return get(c->vtype()); }

   inline static const _CType* get(char c)               { return get(static_cast<_CType::Types>(c)); }
   inline static const _CType* get(const MYSQL_FIELD &f) { return get(f.type, (UNSIGNED_FLAG & f.flags)!=0); }
   inline static const _CType* get(const MYSQL_FIELD *f) { return get(f->type, (UNSIGNED_FLAG & f->flags)!=0); }
   inline static const _CType* get(const MYSQL_BIND &b)  { return get(b.buffer_type, b.is_unsigned!=0); }
   inline static const _CType* get(const MYSQL_BIND *b)  { return get(b->buffer_type, b->is_unsigned!=0); }

   inline static size_t buffer_size_required(MYSQL_FIELD &f) { return get(f)->get_size(); }
   inline static size_t buffer_size_required(MYSQL_FIELD *f) { return get(*f)->get_size(); }

   /**
    * @name Somewhat convenient subscript operator.
    *
    * These would be more convenient we could use the subscript operator
    * as a static function.   Not allowed, so use these like this:
    *
    * @code
    * _CType &ct = CTyper::instance()[myfield];
    * @endcode
    *
    * @{
    */
   inline const _CType &operator[](const MYSQL_FIELD &f) { return *get(f); }
   inline const _CType &operator[](const MYSQL_BIND &b)  { return *get(b); }
   inline const _CType &operator[](class_types t)        { return *get(t); }
   /**@}*/
};

void t_make_typestr(const IClass **array,
                    int len_array,
                    const IGeneric_String_Callback &gsc);

template <class F>
void make_typestr(const IClass **array,
                  int len_array,
                  const F &f)
{
   t_make_typestr(array, len_array, Generic_String_User<F>(f));
}


#endif
