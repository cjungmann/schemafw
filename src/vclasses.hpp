// -*- compile-command: "g++ -DINCLUDE_VCLASSES_MAIN -std=c++11 -Wall -Werror -Weffc++ -pedantic -ggdb -o vclasses vclasses.cpp"  -*-

#ifndef VCLASSES_HPP_SOURCE
#define VCLASSES_HPP_SOURCE

#include <string.h>
#include <stdexcept>
#include "prandstr.hpp"

#include <endian.h>  // detect endian-ness for dash_val() function

/**
 * @brief Convert a char to a dashed-char uint16_t value for switch statements.
 */
inline constexpr uint16_t dash_val(char c)
{
#if __BYTE_ORDER==__LITTLE_ENDIAN
   return (static_cast<uint16_t>(c) << 8 ) + static_cast<uint16_t>('-');
#elif __BYTE_ORDER==__BIG_ENDIAN
   return (static_cast<uint16_t>('-') << 8 ) + static_cast<uint16_t>(c);
#endif
}

// -b converted to a uint16_t is 25133.  Use this constant
// value to confirm the return value of dash_val('b').  A
// failed assertion indicates a problem with endian-ness that
// must be corrected.
static_assert(dash_val('b') == 25133,
              "Check endian-ness situation for dash_val function." );

// effc++ delete assign and copy operators
#ifndef EFFC_2
#define EFFC_2(X) X(const X&)=delete; X& operator=(const X&)=delete;
#endif

// effc++ add virtual destructor and delete assign and copy operators
#ifndef EFFC_3
#define EFFC_3(X) X(const X&)=delete; X& operator=(const X&)=delete; virtual ~X() {}
#endif

class Pure_Virtual_Exception : public std::exception
{
public:
   Pure_Virtual_Exception(void)
      : std::exception() { }
};


size_t buffer_copy(void *destination, size_t len_d, const void *source, size_t len_s);

enum class_types
{
   CT_UNDEFINED     = 0,
   CT_NULL          = 1,

   CT_FLOAT_FLOAT   = (1<<3) + 1,
   CT_FLOAT_DOUBLE,
   
   CT_INT_TINY      = (1<<4)+1,
   CT_INT_SHORT,
   CT_INT_LONG,
   CT_INT_LONGLONG,
      
   CT_UINT_TINY     = (1<<5)+1,
   CT_UINT_SHORT,
   CT_UINT_LONG,
   CT_UINT_LONGLONG,

   CT_TIME_DATETIME = (1<<6)+1,
   CT_TIME_TIME,
   CT_TIME_DATE,

   CT_STRING_TEXT   = (1<<7)+1
};

/**
 * @name Class IClass_User and function pointer iclass_caster provide means to use MYSQL_BIND buffers.
 *
 * I moved this toward the top of vclasses.hpp to make it easier to find.
 * @{
 */
class IClass;   // forward declaration
/**
 * @brief Interface class that uses the stack-resident instance of an IClass object.
 */
class IClass_User
{
public:
   virtual void use(IClass &v) = 0;
};

/**
 * @brief Makes an IClass instance from an anonymous buffer.
 *
 * This function pointer is primarily used by BindC to cast the anonymous
 * BindC::m_data member (used as MYSQL_BIND::buffer) to a usable IClass instance.
 */
typedef void(*iclass_caster)(IClass_User&, void* data, size_t len_buffer, unsigned long len_data);
/**@}*/

class String_Base;

/**
 * @brief Common base class for all value types.
 */
class IClass
{
protected:
   /**
    * @brief Get address of data for variable-length types.
    *
    * This method is neither needed nor called by fixed-length
    * types based on class Number.  It is intended for
    * String_Base-derived classes for copying values.
    */
   virtual const void* get_vdata(void) const = 0;
   friend class String_Base;

   /**
    * @brief Flag for detecting if destructors have been called.
    *
    * This variable is meant to help detect and warn against instantiating
    * an Adhoc_Setter object using temporary IClass-derived arguments.
    */
   static bool s_destruct_flag;
   
public:
   virtual ~IClass() { s_destruct_flag=true; }
   
   virtual class_types vtype(void) const = 0;
   virtual class_types vclass(void) const = 0;
   
   virtual void set_from_eqtype(const IClass &rhs) = 0;
   virtual void set_from_eqclass(const IClass &rhs) = 0;
   virtual size_t set_from_streamer(IStreamer &str) = 0;

   virtual size_t data_length(void) const = 0;
   virtual size_t get_buffer_length(void) const { return data_length(); }
   virtual size_t get_string_length(void) const { return data_length(); }

   /** @brief install() is to attach others' memory to a virtual class.  Real classes should throw. */
   virtual void install(void *data) = 0;
   
   virtual void print(FILE *f) const = 0;
   virtual void print_xml_escaped(FILE *f) const = 0;
   
   void get(const IClass &rhs);
   void put(IClass &lhs) const;
   bool is_equal_to(const IClass &rhs) const;

   static void clear_destruct_flag(void)   { s_destruct_flag=false; }
   static bool read_destruct_flag(void)    { return s_destruct_flag; }
};

/** @brief Define class_types-based IClass pure virtual functions. */
template <class_types t>
class Class_typer : public virtual IClass
{
public:
   virtual class_types vtype(void) const { return t; }
   virtual class_types vclass(void) const { return static_cast<class_types>(t - (t % 8) ); }
};


class Null_Real : public virtual IClass,
                  public Class_typer<CT_NULL>
{
   virtual const void* get_vdata(void) const  { return nullptr; }
   void throw_unsetable(void)  { throw std::runtime_error("NULL object can't be set."); }
   
public:
   
   virtual void set_from_eqtype(const IClass &rhs)  { throw_unsetable(); }
   virtual void set_from_eqclass(const IClass &rhs) { throw_unsetable(); }
   virtual size_t set_from_streamer(IStreamer &str) { throw_unsetable(); return 0; }

   virtual size_t data_length(void) const           { return 0; }

   /** @brief install() is to attach others' memory to a virtual class.  Real classes should throw. */
   virtual void install(void *data)                 { }
   
   virtual void print(FILE *f) const                { throw std::runtime_error("NULL can't be printed"); }
   virtual void print_xml_escaped(FILE *f) const    { throw std::runtime_error("NULL can't be printed"); }

   static void cast_and_use(IClass_User &ic_u, void *data, size_t len_buffer, unsigned long len_data=0)
   {
      Null_Real nr;
      ic_u.use(nr);
   }
};

/** @brief Common superclass for Number to convert to common data type. */
template <class C>
class INumber_Common
{
public:
   virtual C get_common_value(void) const = 0;
};

/** @brief Template class to define the remainder of the IClass pure virtual functions. */
template <class T, class C, void(*P)(C,FILE*), C(*S)(IStreamer&),  class_types t>
class Number : public virtual IClass,
               public Class_typer<t>,
               public INumber_Common<C>
{
public:
   inline const T* get_value_ptr(void) const { return static_cast<const T*>(get_vdata()); }
   inline T get_value(void) const            { return *get_value_ptr(); }
   inline const T& get_value_ref(void) const { return *get_value_ptr(); }
   inline T& get_value_ref(void)             { return *const_cast<T*>(get_value_ptr()); }

   virtual C get_common_value(void) const    { return static_cast<C>(get_value()); }

   virtual void set_from_eqtype(const IClass &rhs)
   {
      get_value_ref() = (dynamic_cast<const Number<T,C,P,S,t>*>(&rhs))->get_value();
   }

   virtual void set_from_eqclass(const IClass &rhs)
   {
      C val = dynamic_cast<const INumber_Common<C>*>(&rhs)->get_common_value();
      get_value_ref() =  static_cast<T>(val); 
   }

   virtual size_t set_from_streamer(IStreamer &str)
   {
      get_value_ref() = static_cast<T>( (*S)(str) );
      return sizeof(T);
   }

   virtual size_t data_length(void) const { return sizeof(T); }
   
   virtual void print(FILE *f) const             { (*P)(get_common_value(), f); }
   // Numbers do not need to be xml-escaped
   virtual void print_xml_escaped(FILE *f) const { (*P)(get_common_value(), f); }

   // Non virtual casts operators to use after cast_and_use():
   operator const T&(void) const  { return get_value_ref(); }
   operator C(void)               { return static_cast<C>(get_value_ref()); }

   bool operator==(const T &v) const { return get_value_ref()==v; }
};



template <class T>
class Real_Store : public virtual IClass
{
protected:
   T m_data;
   virtual const void* get_vdata(void) const { return static_cast<const void*>(&m_data); }
   
public:
   Real_Store(void)  : m_data()        { }
   Real_Store(T val) : m_data(val)     { }
   
   virtual void install(void *data)    { throw std::runtime_error("Invalid attempt to Real_Store::install"); }
};


template <class T, class C, void(*P)(C,FILE*), C(*S)(IStreamer&), class_types t>
class Number_Real : public Number<T,C,P,S,t>, public Real_Store<T>
{
public:
   Number_Real(void)
      : Real_Store<T>()    { }
   Number_Real(T val)
      : Real_Store<T>(val) { }

};

typedef Number_Real<uint8_t,  uint64_t, cprint_uint, uint_from_stream, CT_UINT_TINY>     ri_utiny;
typedef Number_Real<uint16_t, uint64_t, cprint_uint, uint_from_stream, CT_UINT_SHORT>    ri_ushort;
typedef Number_Real<uint32_t, uint64_t, cprint_uint, uint_from_stream, CT_UINT_LONG>     ri_ulong;
typedef Number_Real<uint64_t, uint64_t, cprint_uint, uint_from_stream, CT_UINT_LONGLONG> ri_ulonglong;

typedef Number_Real<int8_t,  int64_t, cprint_int, int_from_stream, CT_INT_TINY>          ri_tiny;
typedef Number_Real<int16_t, int64_t, cprint_int, int_from_stream, CT_INT_SHORT>         ri_short;
typedef Number_Real<int32_t, int64_t, cprint_int, int_from_stream, CT_INT_LONG>          ri_long;
typedef Number_Real<int64_t, int64_t, cprint_int, int_from_stream, CT_INT_LONGLONG>      ri_longlong;

typedef Number_Real<float, double, cprint_double, double_from_stream, CT_FLOAT_FLOAT>    ri_float;
typedef Number_Real<double, double, cprint_double, double_from_stream, CT_FLOAT_DOUBLE>  ri_double;

typedef Number_Real<mydate, mydate, cprint_mydate, mydate_from_stream, CT_TIME_DATETIME>   ri_date;


template <class T>
class Virtual_Store : public virtual IClass
{
protected:
   T* m_pdata;
   virtual const void* get_vdata(void) const { return static_cast<const void*>(m_pdata); }
   
public:
   Virtual_Store(void)      : m_pdata(nullptr)              { }
   Virtual_Store(T* val)    : m_pdata(val)                  { }
   Virtual_Store(void* val) : m_pdata(static_cast<T*>(val)) { }

   virtual void install(void *data)    { m_pdata = static_cast<T*>(data); }

   EFFC_3(Virtual_Store)
};


template <class T, class C, void(*P)(C,FILE*), C(*S)(IStreamer&), class_types t>
class Number_Virtual : public Number<T,C,P,S,t>, public Virtual_Store<T>
{
public:
   Number_Virtual(void)
      : Virtual_Store<T>()    { }
   Number_Virtual(T* val)
      : Virtual_Store<T>(val) { }
   Number_Virtual(void* val)
      : Virtual_Store<T>(val) { }

   static void cast_and_use(IClass_User &ic_u, void *data, size_t len_buffer, unsigned long len_data)
   {
      Number_Virtual<T,C,P,S,t> o(data);
      ic_u.use(o);
   }
};

typedef Number_Virtual<uint8_t,  uint64_t, cprint_uint, uint_from_stream, CT_UINT_TINY>     ai_utiny;
typedef Number_Virtual<uint16_t, uint64_t, cprint_uint, uint_from_stream, CT_UINT_SHORT>    ai_ushort;
typedef Number_Virtual<uint32_t, uint64_t, cprint_uint, uint_from_stream, CT_UINT_LONG>     ai_ulong;
typedef Number_Virtual<uint64_t, uint64_t, cprint_uint, uint_from_stream, CT_UINT_LONGLONG> ai_ulonglong;

typedef Number_Virtual<int8_t,  int64_t, cprint_int, int_from_stream, CT_INT_TINY>         ai_tiny;
typedef Number_Virtual<int16_t, int64_t, cprint_int, int_from_stream, CT_INT_SHORT>        ai_short;
typedef Number_Virtual<int32_t, int64_t, cprint_int, int_from_stream, CT_INT_LONG>         ai_long;
typedef Number_Virtual<int64_t, int64_t, cprint_int, int_from_stream, CT_INT_LONGLONG>     ai_longlong;

typedef Number_Virtual<float, double, cprint_double, double_from_stream, CT_FLOAT_FLOAT>      ai_float;
typedef Number_Virtual<double, double, cprint_double, double_from_stream, CT_FLOAT_DOUBLE>    ai_double;

typedef Number_Virtual<mydate, mydate, cprint_mydate, mydate_from_stream, CT_TIME_DATETIME>   ai_date;





class String_Base : public virtual IClass, public Class_typer<CT_STRING_TEXT>
{
protected:
   size_t        m_len_buffer;
   unsigned long m_len_string;
protected:
   virtual const void* get_vdata(void) const = 0;
   // Add pure virtual function needed for this kind of data:
   void set_buffer_length(size_t len)     { m_len_buffer = len; }
   
public:
   String_Base(size_t len_buffer, unsigned long len_string)
      : m_len_buffer(len_buffer),
        m_len_string(len_string)                      { }

   virtual ~String_Base()                             { }
   virtual void set_from_eqclass(const IClass &rhs)   { }
   virtual void set_from_eqtype(const IClass &rhs);
   virtual size_t set_from_streamer(IStreamer &str);
   virtual size_t data_length(void) const       { return get_string_length(); }

   virtual size_t get_buffer_length(void) const { return m_len_buffer; }
   virtual size_t get_string_length(void) const { return m_len_string; }

   virtual void print(FILE *f) const;
   virtual void print_xml_escaped(FILE *f) const;

   operator const char*(void) const                 { return static_cast<const char*>(get_vdata()); }
   const char *str(void) const                      { return static_cast<const char*>(get_vdata()); }
   bool operator==(const char* v) const             { return 0==strcmp(v,static_cast<const char*>(get_vdata())); }
};

class String_Virtual : public String_Base
{
protected:
   char*         m_pdata;

   virtual const void* get_vdata(void) const        { return static_cast<const void*>(m_pdata); }

public:
   String_Virtual(char *buffer, size_t len_buffer, unsigned long len_string=0)
      : String_Base(len_buffer, len_string),
        m_pdata(buffer)                         { if (!len_string) m_len_string=strlen(buffer); }
   String_Virtual(void)
      : String_Base(0,0), m_pdata(nullptr)      { }

   virtual void install(void *data)             { m_pdata = static_cast<char*>(data); }

   static void cast_and_use(IClass_User &ic_u, void *data, size_t len_buffer, unsigned long len_data)
   {
      String_Virtual o(static_cast<char*>(data), len_buffer, len_data);
      ic_u.use(o);
   }

   EFFC_3(String_Virtual)
};

class String_Const : public String_Base
{
protected:
   const char*   m_pdata;

   virtual const void* get_vdata(void) const        { return static_cast<const void*>(m_pdata); }

public:
   String_Const(const char *str)
      : String_Base(0,strlen(str)),
        m_pdata(str)                   { m_len_buffer=m_len_string+1; }

   virtual void set_from_eqtype(const IClass &rhs)  { throw std::runtime_error("The compiler should have warned you not to set this."); }
   virtual size_t set_from_streamer(IStreamer &str) { throw std::runtime_error("Illegal attempt to set a const string."); }

   virtual void install(void *data)                 { m_pdata = static_cast<const char*>(data); }

   EFFC_3(String_Const)
};

typedef String_Virtual ai_text;
typedef String_Const   si_text;  // for "static" text, string literals.


#endif
