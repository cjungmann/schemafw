// -*- compile-command: "g++ -std=c++11 -Wall -Werror -Weffc++ -pedantic -DINCLUDE_PROC_MAIN `mysql_config --cflags` -o procedure procedure.cpp `mysql_config --libs`" -*-

#ifndef BINDINFO_STR_HPP_SOURCE
#define BINDINFO_STR_HPP_SOURCE

#include "bindc.hpp"

class BindInfo_str : public IBindInfo
{
public:
   static const char *s_allowed_types;
   static const size_t s_default_str_len;
   static const size_t s_max_str_len;
   
protected:
   const char      *m_typestr; /**< Saved const char* to constructor const char* parameter */
   const char      *m_typeptr; /**< Pointer to current position in m_typestr. */
   unsigned         m_count;   /**< Number of elements in string.  Scanned in constructor. */
   
   _CType::Types    m_curtype; /**< Saved type from last call to next(). */
   unsigned         m_curlen;  /**< Current element calculated length. */
   const _CType    *m_ctype;   /**< Current element _CType typeinfo struct pointer. */
public:
   BindInfo_str(const char *str);

   virtual unsigned count(void) const   { return m_count; }

   /** @name Functions to direct progress through defined elements. @{ */
   virtual void next(void);
//   virtual bool end(void) const         { return *m_typeptr==0; }
   // I'm not sure why I used this dodgy code, so I'm saving it in case
   // a future problem reveals this design idea:
   // virtual bool end(void) const         { return m_curtype==0; }
   virtual bool end(void) const            { return m_ctype==nullptr; }
   /**@}*/

   /** @name Functions that provide information about the current BIND. @{ */
   virtual const _CType* get_ctype(void) const        { return m_ctype; }
   virtual size_t buffer_size_required(void) const;
   virtual size_t data_length(void) const             { return m_curlen; }
   virtual void set_buffer_type(MYSQL_BIND &bind) const;
   virtual size_t cur_name_length(void) const         { return 0; }
   virtual const char *cur_name(void) const           { return ""; }
   /**@}*/

   static unsigned count_types(const char *str);


public:
   /** @name Delete functions to avoid effc++ warnings @{ */
   virtual ~BindInfo_str()               { }
   BindInfo_str(const BindInfo_str&)            = delete;
   BindInfo_str& operator=(const BindInfo_str&) = delete;
   /**@}*/


};


#endif // BINDINFO_STR_HPP_SOURCE
