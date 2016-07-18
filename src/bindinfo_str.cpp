// -*- compile-command: "g++ -std=c++11 -Wall -Werror -Weffc++ -pedantic -DINCLUDE_PROC_MAIN `mysql_config --cflags` -o procedure procedure.cpp `mysql_config --libs`" -*-

#include "bindinfo_str.hpp"

const char *BindInfo_str::s_allowed_types = "ijklIJKLsntdfb";
const size_t BindInfo_str::s_default_str_len = 32;
const size_t BindInfo_str::s_max_str_len = 128;

BindInfo_str::BindInfo_str(const char *str)
   : m_typestr(str), m_typeptr(str), m_count(0),
     m_curtype(static_cast<_CType::Types>(-1)),
     m_curlen(0), m_ctype(nullptr)
{
   m_count = count_types(str);
   next();
}


/**
 * @brief Unintuitive implementation of IBindInfo::next.
 *
 * Other implementations of IBindInfo are ready to access the
 * initial item immediately after the constructor is called, so
 * this implementation calls next() in the constructor to be
 * consistent.
 *
 * However, that means that this next() function eventually
 * creates an invalid state, which is detected by end().
 *
 * Since CTyper::get() throws an error for an unrecognized type,
 * setting m_ctype must check m_curtype before attempting to use
 * CTyper::get().
 */
void BindInfo_str::next(void)
{
   m_curtype = static_cast<_CType::Types>(*m_typeptr);
   m_typeptr++;

   m_curlen = 0;
   while (isdigit(*m_typeptr))
   {
      m_curlen = (m_curlen*10) + *m_typeptr-'0';
      ++m_typeptr;
   }

   // Don't allow CTyper::get() to throw 
   m_ctype = m_curtype ? CTyper::get(m_curtype) : nullptr;
}

/**
 * @brief Use m_msqltype to retrieve the corresponding _CType to use it m_size value.
 */
size_t BindInfo_str::buffer_size_required(void) const
{
   if (m_curtype==_CType::TYPE_STRING || m_curtype==_CType::TYPE_BLOB)
   {
      if (m_curlen==0)
         return s_default_str_len;
      else if (m_curlen > s_max_str_len)
         return s_max_str_len;
      else
         return m_curlen;
   }
   else
      return m_ctype->get_size();
}


void BindInfo_str::set_buffer_type(MYSQL_BIND &bind) const
{
   bind.buffer_type = m_ctype->get_sqltype();
   bind.is_unsigned = m_ctype->is_unsigned();
}

/**
 * @brief Scans str to determine number of types defined.
 *
 * This function is mainly used in the constructor to allow this
 * class to inform the builder of the array size required to contain
 * the elements in the type string.
 */
unsigned BindInfo_str::count_types(const char *str)
{
   const char *p = str;
   unsigned count = 0;
   while (*p)
   {
      assert(strchr(s_allowed_types, *p));
      count++;
      while (isdigit(*++p))
         ;
   }
   return count;
}


