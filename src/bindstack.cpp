// -*- compile-command: "g++ -std=c++11 -Wall -Werror -Weffc++ -pedantic -DINCLUDE_BINDSTACK_MAIN `mysql_config --cflags` -o bindstack bindstack.cpp `mysql_config --libs`" -*-

#include "bindstack.hpp"

void BindStack::set_null_all_binds(void)
{
   for (auto *p = start(); p; p=p->next())
      p->object().set_is_null();
}

size_t BindStack::calc_buffer_size(const _CType *ctype, size_t requested)
{
   size_t bsize = ctype->m_size;
   if (bsize==0)
   {
      if (requested > s_max_buffer_length)
         bsize = s_max_buffer_length;
      else
         bsize = requested;
   }
   return bsize;
}

void BindStack::t_build(MYSQL_RES *res, IGeneric_Callback<BindStack> &user)
{
   int count = res->field_count;
   line_handle *lines = static_cast<line_handle*>(alloca(count*sizeof(line_handle)));
   memset(lines, 0, count*sizeof(line_handle*));
   
   MYSQL_BIND *binds = static_cast<MYSQL_BIND*>(alloca(count * sizeof(MYSQL_BIND)));
   memset(binds, 0, count*sizeof(MYSQL_BIND));
      
   // We only need one end pointer for both arrays:
   const MYSQL_BIND   *end = &binds[count];
      
   MYSQL_BIND         *bptr = binds;
   line_handle        *lptr = lines;
   const MYSQL_FIELD  *fptr = res->fields;
   const _CType       *ctype = nullptr;

   t_handle<BindC> *head = nullptr;
   t_handle<BindC> *tail = nullptr;

   while(bptr < end)
   {
      ctype = CTyper::get(fptr);
      assert(ctype);

      // Set this member variable as soon as we know it so
      // other member functions can use and reuse it:
      size_t extra_size = calc_buffer_size(ctype, fptr->length);

      size_t alloclen = t_handle<BindC>::line_size(fptr->name_length, extra_size);
      void *linebuff = alloca(alloclen);

      tail = t_handle<BindC>::init_handle(linebuff, fptr->name, extra_size, tail);

      BindC &bindc = tail->object();
      memset(&bindc, 0, sizeof(BindC));
      bindc.initialize(bptr, tail->extra(), extra_size, ctype);
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
   BindStack ds(lines, count);
   user(ds);
}


unsigned BindStack::count_types(const char *typestr)
{
   unsigned count = 0;
   const char *ptr = typestr;
   while (*ptr)
   {
      ++count;
      while((*++ptr) && isdigit(*ptr))
         ;
   }
   return count;
}

/**
 * @brief Build a BindStack from a string.
 *
 * @note If a buffer size is not indicated in the string, and if the type
 * doesn't have a natural size, the default size for a string or blob will be 64.
 */
void BindStack::t_build(const char *typestr, IGeneric_Callback<BindStack> &user)
{
   unsigned     count = count_types(typestr);
   const char   *ptr = typestr;
   const _CType *ctype = nullptr;

   MYSQL_BIND *binds = static_cast<MYSQL_BIND*>(alloca(sizeof(MYSQL_BIND)*count));
   memset(binds, 0, sizeof(MYSQL_BIND)*count);
   MYSQL_BIND *bptr = binds;

   line_handle *lines = static_cast<line_handle*>(alloca(sizeof(line_handle)*count));
   memset(lines,0,sizeof(line_handle)*count);
   line_handle *lptr = lines;

   t_handle<BindC> *head = nullptr;
   t_handle<BindC> *tail = nullptr;

   while(*ptr)
   {
      ctype = CTyper::get(*ptr);
      assert(ctype);
      
      size_t extra_size = 0;
      while (*++ptr && isdigit(*ptr))
         extra_size = extra_size*10 + '0'-*ptr;

      // Use a minimum size for string-types that don't request space,
      // i.e  "iis" rather than "iis32"
      if (extra_size==0)
         extra_size=64;

      extra_size = calc_buffer_size(ctype, extra_size);
      
      size_t alloclen = t_handle<BindC>::line_size("", extra_size);
      void *linebuff = alloca(alloclen);

      tail = t_handle<BindC>::init_handle(linebuff, "", extra_size, tail);

      // Attach the MYSQL_BIND and set its fields:
      BindC &bindc = tail->object();
      memset(&bindc, 0, sizeof(BindC));
      bindc.initialize(bptr, tail->extra(), extra_size, ctype);

      // Perhaps redundant since we're saving each element in the array,
      // for completeness and in case this code is used as a reference:
      if (!head)
         head = tail;

      // Save the item to the line_handle array for DataStack constructor:
      *lptr = tail->lhandle();

      ++bptr;
      ++lptr;
   }

   BindStack bs(lines, count);
   user(bs);
}


#ifdef INCLUDE_BINDSTACK_MAIN

#include <stdio.h>

#include "prandstr.cpp"
#include "vclasses.cpp"
#include "ctyper.cpp"
#include "bindc.cpp"
#include "datastack.cpp"


void show_bindstack(const BindStack &bs)
{
   int count=0;
   for (auto *ptr = bs.start(); ptr; ptr=ptr->next())
   {
      const _CType *ctype =  ptr->object().m_typeinfo;
      printf("%d: %s of type %s (%s)\n",
             ++count,
             ptr->str(),
             ctype->m_sql_name,
             ctype->m_param_data_type);
   }
}

void test_string_bind(const char *str)
{
   auto f = [](BindStack &bs)
   {
      printf("I got the bindstack!\n");
      show_bindstack(bs);
   };

   BindStack::build(str, f);
}

int main(int argc, char **argv)
{

   test_string_bind("iiis20ft");
   
   return 0;
}

#endif
