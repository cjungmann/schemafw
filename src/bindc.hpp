// -*- compile-command: "g++ -std=c++11 -Wall -Werror -Weffc++ -pedantic -DINCLUDE_BC_MAIN `mysql_config --cflags` -U NDEBUG -o bindc bindc.cpp `mysql_config --libs`" -*-

#ifndef BINDC_HPP_SOURCE
#define BINDC_HPP_SOURCE

#include <mysql.h>
#include "datastack.hpp"
#include "vclasses.hpp"
#include "ctyper.hpp"

/**
 * @brief Structure to use with the DataStack.
 *
 * This structure is to be used in the data part of a line_handle.  It
 * must be used in parallel with an array of MYSQL_BIND, the elements of
 * which are referenced by the m_bind member of BindC.
 *
 * The BindC structure should probably be declared as a class due to the
 * many member functions that use the data contained by the structure.  For now,
 * it's a struct.
 *
 * The BindC structure includes members that the MYSQL_BIND element
 * wants to point at, specifically the m_length, m_is_null, and m_error
 * elements are referenced by the MYSQL_BIND::length, MYSQL_BIND::is_null,
 * and MYSQL_BIND::error elements.
 *
 * Of particular importance is the m_caster data member.  It is a pointer
 * to a function that will build the specific IClass-derived object from
 * the m_data member.  That provides the explicit cast needed to use
 * features of the data type without creating objects that need heap memory.
 *
 * @todo Add feature that uses mysql_stmt_send_long_data() to send multiple
 *       blocks of a large data item, like a photo.
 */
struct BindC
{
   MYSQL_BIND    *m_bind;
   void          *m_data;     /**< Pointer to allcated data.              */
   unsigned long m_length;    /**< For MYSQL_BIND::length pointer.        */
   my_bool       m_is_null;   /**< For MYSQL_BIND::is_null pointer.       */
   my_bool       m_error;     /**< For MYSQL_BIND::error pointer.         */
   unsigned long m_str_length; /**< Type-derived maximum length           */
   unsigned int  m_flags;     /**< Flags for MYSQL_FIELD-initiated BindCs */
   const _CType  *m_typeinfo; /**< Bundle of useful type info.            */
   const char    *m_dtdid;    /**< Null except for ENUM and SET types.
                               *   Points to a stack-allocaed buffer that
                               *   is NOT associated with the base_handle
                               *   memory block.  It will be allocated in
                               *   the same stack frame, so it will be
                               *   valid as long as the rest of the object
                               *   is valid.
                               */
   int           m_format;     /**< Place to save output formatting hints.
                                    In theory, Result_As_XXX::pre_fetch_use_result()
                                    sets the value, and Result_As_XXX::use_result_row()
                                    uses the value.  For example, Result_As_SchemaDoc
                                    leaves m_format==0 for attribute output, m_format==1
                                    for output as child element, and m_format==2 for
                                    output as text (the element value).
                               */
   //   iclass_caster m_caster;   /**< Create and use an IClass object.   */

   void initialize(MYSQL_BIND *bind,
                   void *extra,
                   size_t len_extra,
                   const _CType *ctype=nullptr);

   inline iclass_caster get_caster(void) const    { return m_typeinfo->m_caster; }
   inline void use_cast(IClass_User &user) const
   {
      // Assertion to avoid future DECIMAL-like blind-spots:
      assert(m_bind->buffer_length>0);
      (*get_caster())(user,m_data,m_bind->buffer_length,m_length);
   }
   
   void set_from(const IClass &rhs);
   void set_from(IStreamer &s);
   inline void set_from(const char *val)    { StringStreamer ss(val); set_from(ss); }
   inline void assign_to(IClass &lhs) const { _assign_to c(lhs); use_cast(c); }
   inline void print(FILE* f) const         { _print c(f); use_cast(c); }
   inline void print_xml_escaped(FILE *f) const { _print_xml_escaped c(f); use_cast(c); }
//   void print_xml(FILE *f) const;

   inline void set_values_from(const MYSQL_FIELD *f)
   {
      m_str_length = f->length;
      m_flags      = f->flags;
   }


   inline BindC &operator=(const IClass &rhs)     { set_from(rhs); return *this; }

   inline bool is_null(void) const                { return m_is_null; }
   inline void is_null(bool value)                { m_is_null = value ? true : false; }
   inline bool is_matched_type(const IClass &c)   { return c.vtype()==vtype(); }
   inline void install_to_alias(IClass &ac)       { ac.install(m_data); }
   inline class_types vtype(void) const           { return m_typeinfo->m_ctype; }

   /** @brief Set null flag, especially for BindStack::clear_for_fetch. */
   inline void set_is_null(bool value=true)       { m_is_null = value ? true : false; }

   /**
    * @name Expose _CType elements:
    * @{
    */
   inline enum_field_types sqltype(void) const        { return m_typeinfo->m_sqltype; }
   inline bool             is_unsigned(void) const    { return m_typeinfo->m_is_unsigned; }
   inline class_types      classtype(void) const      { return m_typeinfo->m_ctype; }
   inline _CType::Types    ctype_type(void) const     { return m_typeinfo->m_stype; }
   inline size_t           size(void) const           { return m_typeinfo->m_size; }
   inline iclass_caster    caster(void) const         { return m_typeinfo->m_caster; }
   inline const char *     sqltype_name(void) const   { return m_typeinfo->m_sql_name; }
   inline const char *     paramtype_name(void) const { return m_typeinfo->m_param_data_type; }
   inline const char *     dtdid(void) const          { return m_dtdid; }
   inline void             format(int val)            { m_format=val; }
   inline int              format(void) const         { return m_format; }
   /**@}*/

   inline unsigned long    strlength(void) const      { return m_str_length; }

   /**
    * @name Expose MYSQL_FIELD flag settings
    *
    * Not all of these flags are currently used, and these flags do not
    * represent the fill list of flags (look at mysql_com.h for the complete
    * list).  However, most are useful for writing out the schema (not_null,
    * primary_key, autoincrement).  Others are useful, but ignored for now
    * (unique_key, no_default_value, on_update_now).  I have no intention of
o    * considering zerofill.
    *
    * 
    * @{
    */
   inline bool sqlflag_not_null(void) const         { return (m_flags & NOT_NULL_FLAG) !=0; }
   inline bool sqlflag_primary_key(void) const      { return (m_flags & PRI_KEY_FLAG) !=0; }
   inline bool sqlflag_unique_key(void) const       { return (m_flags & UNIQUE_KEY_FLAG) !=0; }
   inline bool sqlflag_multiple_key(void) const     { return (m_flags & MULTIPLE_KEY_FLAG) !=0; }
   inline bool sqlflag_blob(void) const             { return (m_flags & BLOB_FLAG) != 0; }
   inline bool sqlflag_zerofill(void) const         { return (m_flags & ZEROFILL_FLAG) != 0; }
   inline bool sqlflag_binary(void) const           { return (m_flags & BINARY_FLAG) != 0; }
   inline bool sqlflag_enum(void) const             { return (m_flags & ENUM_FLAG) != 0; }
   inline bool sqlflag_autoincrement(void) const    { return (m_flags & AUTO_INCREMENT_FLAG) != 0; }
   inline bool sqlflag_set(void) const              { return (m_flags & SET_FLAG) != 0; }
   inline bool sqlflag_no_default_value(void) const { return (m_flags & NO_DEFAULT_VALUE_FLAG) != 0; }
   inline bool sqlflag_on_update_now(void) const    { return (m_flags & ON_UPDATE_NOW_FLAG) != 0; }
   inline bool sqlflag_num(void) const              { return (m_flags & NUM_FLAG) != 0; }
   inline bool sqlflag_part_key(void) const         { return (m_flags & PART_KEY_FLAG) != 0; }
   inline bool sqlflag_group(void) const            { return (m_flags & GROUP_FLAG) != 0; }
   /**@}*/


private:
   /**
    * @brief Privatized this function to avoid confusion.
    *
    * _CType has an unsigned flag that is more reliable, so that is the
    * preferred method.  I've left this one in for now, it can be useful
    * for knowing when to set the _CType flag.
    *
    * @todo Confirm that unsigned is being set when writing out a table schema.
    */
   inline bool sqlflag_unsigned(void) const         { return (m_flags & UNSIGNED_FLAG) != 0; }
public:
   /**@}*/
   

   /** @name For handling truncated values: @{ */
   inline bool is_truncated(void) const           { return m_error!=0; }
   inline bool is_string_type(void) const
   {
      enum_field_types t = m_bind->buffer_type;
      return t==MYSQL_TYPE_DECIMAL
         || t==MYSQL_TYPE_VARCHAR
         || t >= MYSQL_TYPE_NEWDECIMAL;
      // return t==MYSQL_TYPE_DECIMAL
      //    || t==MYSQL_TYPE_VARCHAR
      //    || t==MYSQL_TYPE_NEWDECIMAL
      //    || t==MYSQL_TYPE_TINY_BLOB
      //    || t==MYSQL_TYPE_MEDIUM_BLOB
      //    || t==MYSQL_TYPE_LONG_BLOB
      //    || t==MYSQL_TYPE_BLOB
      //    || t==MYSQL_TYPE_VAR_STRING
      //    || t==MYSQL_TYPE_STRING;
 }
   inline unsigned long data_length(void) const   { return m_length; }
   inline void data_length(unsigned long newlen)  { m_length = newlen; }
   inline unsigned long buffer_length(void) const { return m_bind->buffer_length; }
   inline operator MYSQL_BIND *(void) const       { return m_bind; }
   /**@}*/

   /** @name Static conversion functions @{ */
   // static class_types get_ctype(const MYSQL_BIND *b);
   // static enum_field_types name_to_type(const char *name);
   // static size_t basic_type_length(enum_field_types type);
   // static iclass_caster get_caster(const MYSQL_BIND *bind);
   /**@}*/
   

   static inline size_t basic_type_length(const char *name) { return CTyper::get(name)->m_size; }

   /** efficient equality function for checking result values. */
   inline bool operator==(const char *str) const  { return strncmp(str, static_cast<const char*>(m_data), m_length)==0; }
   
   /** Checks for equivalent string values. Not safe, user responsible to ensure left- and right-side values are strings. */
   inline bool operator==(const BindC &b) const
   {
      if (!b.m_is_null && !m_is_null)
         if (b.vtype()==vtype())
            if (b.m_length == m_length)
               return 0==memcpy(b.m_data, m_data, m_length);

      return false;
   }

   
   class _print : public IClass_User
   {
      FILE* m_out;
   public:
      _print(FILE* out) : m_out(out)     { }
      EFFC_3(_print);
      virtual void use (IClass &v)       { v.print(m_out); }
   };

   class _print_xml_escaped : public IClass_User
   {
      FILE *m_out;
   public:
      _print_xml_escaped(FILE *out) : m_out(out) { }
      EFFC_3(_print_xml_escaped);
      virtual void use (IClass &v)               { v.print_xml_escaped(m_out); }
   };

   class _set_from : public IClass_User
   {
      const IClass &m_rhs;
   public:
      _set_from(const IClass &rhs) : m_rhs(rhs) { }
      virtual void use(IClass &lhs)             { m_rhs.put(lhs); }
   };

   class _set_from_streamer : public IClass_User
   {
      IStreamer &m_streamer;
      size_t    m_copy_length;
   public:
      _set_from_streamer(IStreamer &s)
         : m_streamer(s), m_copy_length(0) { }
      virtual void use(IClass &lhs)        { m_copy_length=lhs.set_from_streamer(m_streamer); }
      size_t copy_length(void) const       { return m_copy_length; }
   };

   class _assign_to : public IClass_User
   {
      IClass &m_lhs;
   public:
      _assign_to(IClass &lhs) : m_lhs(lhs) { }
      virtual void use(IClass &rhs)        { rhs.put(m_lhs); }
   };
};

/** ***************************************
 * @class IBindInfo
 * @brief Interface class that IStackPool-based BindCPool uses to create DataStack elements.
 */
class IBindInfo
{
public:
   virtual ~IBindInfo()   { }

   /** @brief Reports the total number of BINDs represented. */
   virtual unsigned count(void) const = 0;

   /** @name Functions to direct progress through defined elements. @{ */
   virtual void next(void) = 0;
   virtual bool end(void) const = 0;
   /**@}*/

   /** @name Functions that provide information about the current BIND. @{ */
   virtual const _CType* get_ctype(void) const = 0;
   virtual size_t buffer_size_required(void) const = 0;
   virtual size_t data_length(void) const = 0;
   virtual void set_buffer_type(MYSQL_BIND &bind) const = 0;
   virtual size_t cur_name_length(void) const = 0;
   virtual const char *cur_name(void) const = 0;
   /**@}*/
};

/** ***************************************
 * @class IBindUser
 * @brief Interface class for callback by BindCPool when the MYSQL_BIND array and datastack are available.
 */
class IBindUser
{
public:
   virtual ~IBindUser() { }
   virtual void use(DataStack<BindC> *ds, MYSQL_BIND *binds, unsigned count) = 0;
};

/** ***************************************
 * @class BindCPool
 * @brief IStackPool implementation for DataStack.
 */
class BindCPool : public IStackPool<BindC>
{
private:
   /** @name Prevent copy or assign operations on stack-based object. @{ */
   BindCPool(const BindCPool &)            = delete;
   BindCPool& operator=(const BindCPool &) = delete;
   /**@}*/

protected:
   IBindInfo   &m_bindinfo;
   IBindUser   &m_binduser;
   MYSQL_BIND  *m_binds;
   unsigned    m_count;
   unsigned    m_index;

public:
   BindCPool(IBindInfo &bi, IBindUser &bu, MYSQL_BIND *binds, unsigned count)
      : m_bindinfo(bi), m_binduser(bu), m_binds(binds),
        m_count(count), m_index(0)           { }

   virtual ~BindCPool()                      { }

   /** @name IStringSet methods @{ */
   virtual void next(void)                   { m_bindinfo.next(); ++m_index; }
   virtual bool end(void)                    { return m_bindinfo.end(); }
   virtual size_t string_length(void)        { return 1+m_bindinfo.cur_name_length(); }
   virtual void copy_string(char *buffer)    { strcpy(buffer, m_bindinfo.cur_name()); }
   /**@}*/

   /** @name IStackPool callback methods @{ */
   virtual size_t get_extra_size(void) const           { return m_bindinfo.buffer_size_required(); }
   virtual void set_data(BindC *v, void *extra, size_t len_extra);
   virtual void use_datastack(DataStack<BindC> &ds) { m_binduser.use(&ds, m_binds, m_count); }
   /**@}*/

   /** @brief Call this static function to build everything. */
   static void build(IBindInfo &bi, IBindUser &bu);
};

/** @brief Function to clear params before possible reuse. */
void clear_stack(DataStack<BindC> &ds);

#endif
