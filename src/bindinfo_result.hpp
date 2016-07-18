// -*- compile-command: "g++ -std=c++11 -fno-inline -Wall -Werror -Weffc++ -pedantic -DINCLUDE_SCHEMAPROC_MAIN `mysql_config --cflags` -o schemaproc schemaproc.cpp `mysql_config --libs`" -*-

// -*- compile-command: "g++ -std=c++11 -Wall -Werror -Weffc++ -pedantic -DINCLUDE_PROC_MAIN `mysql_config --cflags` -o procedure procedure.cpp `mysql_config --libs`" -*-

#ifndef BINDINFO_RESULT_HPP_SOURCE
#define BINDINFO_RESULT_HPP_SOURCE

#include "bindc.hpp"
#include <mysql.h>

#define MAX_FIELD_LENGTH 128

class Result_Stacker
{
protected:
   MYSQL_RES    *m_res;
   /**
    * @brief The fixed size portion of a line (BindC + the overlay).
    *
    * @todo This should be a const static variable once we move the code to a .cpp file
    */
   size_t m_extra_size;
   size_t m_fixed_size;
public:
   Result_Stacker(MYSQL_RES *res)
      : m_res(res),
        m_extra_size(0),
        m_fixed_size(t_handle<BindC>::fixed_size())  { }

   /**
    * @brief Return appropriate buffer length.
    *
    * For fixed-length types (POD or structs), _CType->get_size() returns
    * the appropriate size.  For variable-length types, CType->get_size()
    * returns 0, and we then get the length from the MYSQL_FIELD object.
    *
    * We are limiting the maximum length and are prepared to page the
    * data out if the field contents exceed the buffer size.
    *
    * @todo Reset MAX_FIELD_LENGTH to a larger number once we've
    * satisfied that paged access works where we're testing it.
    */
   static size_t get_buffer_length(const MYSQL_FIELD *fld, const _CType* ctype)
   {
      size_t len = ctype->get_size();
      if (len==0)
      {
         len = fld->length;
         if (len > MAX_FIELD_LENGTH)
            len = MAX_FIELD_LENGTH;
      }
      return len;
   }

   inline size_t get_alloc_len(const MYSQL_FIELD *fld)
   {
      return m_fixed_size + m_extra_size + fld->name_length + 1;
   }

   inline t_handle<BindC>* init_line_handle(void* buff,
                                            const MYSQL_FIELD *f,
                                            line_handle parent) const
   {
      line_handle h = base_handle::init_line_handle(static_cast<char*>(buff),
                                                    f->name,
                                                    sizeof(BindC),
                                                    m_extra_size,
                                                    parent);

      // Here's an idea: it easier to understand and easier to
      // const_cast line_handle than a template class:
      return reinterpret_cast<t_handle<BindC>*>(  const_cast<char*>(h)  );
   }

   void build(IBindUser &user)
   {
      unsigned count = m_res->field_count;
      MYSQL_BIND *binds = static_cast<MYSQL_BIND*>(alloca(count * sizeof(MYSQL_BIND)));
      memset(binds, 0, count*sizeof(MYSQL_BIND));

      line_handle *lines = static_cast<line_handle*>(alloca(count * sizeof(line_handle)));
      memset(lines, 0, count*sizeof(line_handle));

      // We only need one end pointer for both arrays:
      const MYSQL_BIND   *end = &binds[count];
      
      MYSQL_BIND         *bptr = binds;
      line_handle        *lptr = lines;
      const MYSQL_FIELD  *fptr = m_res->fields;
      const _CType       *ctype = nullptr;

      t_handle<BindC> *head = nullptr;
      t_handle<BindC> *tail = nullptr;

      while(bptr < end)
      {
         ctype = CTyper::get(fptr);
         assert(ctype);

         // Set this member variable as soon as we know it so
         // other member functions can use and reuse it:
         m_extra_size = get_buffer_length(fptr, ctype);

         size_t alloclen = get_alloc_len(fptr);
         void *linebuff = alloca(alloclen);

         tail = init_line_handle(linebuff, fptr, *tail);

         BindC &bindc = tail->object();
         bindc.initialize(bptr, tail->extra(), m_extra_size, ctype);
         bindc.set_values_from(fptr);

         // Perhaps redundant since we're saving each element in the array,
         // for completeness and in case this code is used as a reference:
         if (!head)
            head = tail;

         // Save the item to the line_handle array for DataStack constructor:
         *lptr = tail->lhandle();

         ++bptr;
         ++lptr;
         ++fptr;
      }


      // Send the information back:
      DataStack<BindC> ds(lines, count);
      user.use(&ds, binds, count);
   }
};


class BindInfo_result : public IBindInfo
{
protected:
   MYSQL_RES    *m_res;
   MYSQL_FIELD  *m_field;
   const _CType *m_ctype;
   unsigned      m_index;
public:
   BindInfo_result(MYSQL_RES *res)
      : m_res(res), m_field(nullptr), m_ctype(nullptr), m_index(0)  { next(); }


   virtual unsigned count(void) const { return m_res->field_count; }
   /** @brief No error-checking: depending on BindCPool to check count. */
   virtual void next(void)            { m_field = &m_res->fields[m_index++]; m_ctype=CTyper::get(m_field); }
   /** @brief We're checking after m_index has been incremented, judge "end" accordingly. */
   virtual bool end(void) const       { return m_index>count(); }

   /** @name Functions that provide information about the current BIND. @{ */

   /**
    * @brief Return the buffer size required for this MYSQL_BIND element.
    *
    * @todo Change the minimum size back up after testing for truncation is complete.
    *
    * This override of buffer_size_required gets the required size from
    * -# Inherent data-type size, or, if not fixed-size:
    * -# MYSQL_FIELD::length, or, if too big
    * -# The preset maximum size.
    *
    * While testing truncation, I set the max size allowed to be 128.  I think
    * 1024 or some multiple is more appropriate in production.
    */
   virtual size_t buffer_size_required(void) const
   {
      size_t rval = m_ctype->get_size();
      if (!rval)
      {
         rval = m_field->length;
         if (rval > 128)
            rval = 128;
      }
      return rval;
   }

   virtual const _CType* get_ctype(void) const          { return m_ctype; }
   virtual size_t data_length(void) const               { return m_field->length; }
   virtual void set_buffer_type(MYSQL_BIND &bind) const { bind.buffer_type = m_field->type; bind.is_unsigned=(m_field->flags & UNSIGNED_FLAG)?1:0; }
   virtual size_t cur_name_length(void) const           { return m_field->name_length; }
   virtual const char *cur_name(void) const             { return m_field->name; }
   /**@}*/

   /** @name Define functions to avoid effc++ warnings. @{ */
   virtual ~BindInfo_result()   { }
   BindInfo_result(const BindInfo_result&)            = delete;
   BindInfo_result& operator=(const BindInfo_result&) = delete;
   /**@}*/
};

#endif
