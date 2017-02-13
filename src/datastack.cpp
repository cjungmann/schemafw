// -*- compile-command: "g++ -std=c++11 -Wall -Werror -Weffc++ -pedantic -ggdb -DINCLUDE_MAIN -o datastack datastack.cpp" -*-

#include "datastack.hpp"


/**
 * @brief Calculate buffer size required to construct a line_handle.
 *
 * When this function was written, it was intended to coordinate with
 * init_line_handle() to build a line_handle linked list in a loop
 * rather than with recursion.  I expect it will see wider use by
 * eventually replacing some code with the Pool-based strategy.
 */
size_t base_handle::get_line_handle_size(const char *name, size_t size_object, size_t size_extra)
{
   // Start with 1, the \0 terminator of the identity string
   size_t rval = 1;
   if (name)
      rval += strlen(name);
   return rval + sizeof(DMAP) + size_object + size_extra;
}

/**
 * @brief Globally-available line_handle initializer for multiple uses.
 *
 * When I decided I needed a non-pool way to make a datastack, I needed
 * to make the line_handle initiation not dependant on the template class.
 *
 * This function makes it easier to make a linked-list of line_handles.
 *
 * The particular case that inspired this new protocol is when I want
 * to create a DataStack<BindC> from the results of a query.  There
 * is no practical way to make a StackPool from a query result without
 * having a lot of unnecessary stack allocations.
 *
 * /sa BindInfo_schema::use_result
 */
line_handle base_handle::init_line_handle(char *buffer, const char *name,
                                          size_t size_object, size_t size_extra,
                                          line_handle parent)
{
   DMAP *pMap =  reinterpret_cast<DMAP*>(buffer + size_object + size_extra);
   char* h = sizeof(DMAP) + reinterpret_cast<char*>(pMap);

   strcpy(h, name);
   
   pMap->offset = h - buffer;
   pMap->link = nullptr;

   if (parent)
      get_dmap_from_line_handle(parent)->link = h;

   return reinterpret_cast<line_handle>(h);
}

int base_handle::count(line_handle bh)
{
   int count=0;
   line_handle ptr = bh;
   while (ptr)
   {
      ++count;
      ptr = base_handle::get_link(ptr);
   }
   return count;
}

void base_handle::fill_array(line_handle *array, int count, line_handle top)
{
   line_handle cur = top;
   for (int i=0; i<count; ++i)
   {
      array[i] = cur;
      cur = base_handle::get_link(cur);
   }
}

int BaseStack::index_by_name(const char *name) const
{
   for (unsigned i=0; i<m_line_count; ++i)
      if (strcmp(m_line_array[i],name)==0)
         return i;
   
   return static_cast<unsigned>(-1);
}

line_handle BaseStack::line_by_name(const char *name) const
{
   unsigned index = index_by_name(name);
   if (index<m_line_count)
      return m_line_array[index];
   else
      return nullptr;
}

line_handle BaseStack::line_by_index(unsigned index) const
{
   assert(index<m_line_count);
   return m_line_array[index];
}

const void* BaseStack::_data_by_index(unsigned index) const
{
   assert(index<m_line_count);
   return base_handle::_cp_data(m_line_array[index]);
}

const void* BaseStack::_data_by_name(const char *name) const
{
   unsigned index = index_by_name(name);
   if (index<m_line_count)
      return base_handle::_cp_data(m_line_array[index]);
   else
      return nullptr;
}





#ifdef INCLUDE_MAIN

#include <stdio.h>

const char *namelist[4] = {"Zero", "One", "Two", "Three" };
const char *vallist[4] = {"Daddy", "Mommy", "Girl", "Boy" };
int element_count = sizeof(::namelist) / sizeof(char*);

//! [Snippet_Simple_String_Class]
struct strclass
{
   const char *str;
};
//! [Snippet_Simple_String_Class]


class MyTools : public IStackPool<strclass>
{
protected:
   int    m_index;

public:
   MyTools(void) : m_index(0)                { }
   
   /** @name IStringSet methods @{ */
   virtual void next(void)                   { ++m_index; }
   virtual bool end(void)                    { return m_index>=element_count; }
   virtual size_t string_length(void)        { return 1+strlen(namelist[m_index]); }
   virtual void copy_string(char* buffer)    { strcpy(buffer,namelist[m_index]); }
   /**@}*/

   /** @name IStackPool methods @{ */
   virtual size_t get_extra_size(void) const { return 1+sizeof(vallist[m_index]); }
   virtual void set_data(strclass *v, void* extra, size_t len_extra)
   {
      strcpy(static_cast<char*>(extra), vallist[m_index]);
      v->str = static_cast<char*>(extra);
   }
   virtual void use_datastack(DataStack<strclass> &ds)
   {
      const char* narray[] = {"Zero","One","Two"};

      for (int i=0; i<3; ++i)
      {
         const char *name = narray[i];
         int ndx = ds.index_by_name(name);
         if (ndx>=0)
         {
            auto &cur = ds[ndx];
            line_handle lh = ds.line_by_index(ndx);
            assert(static_cast<const void*>(&cur) == static_cast<const void*>(lh));

            printf("I found \"%s\" with \"%s\" as the value.\n",
//                   name, ds.data(ndx)->str);
                   name, cur.object().str);
         }
         else
            printf("I didn't find \"%s\".\n", name);
      }
   }
};


void test_stackbuilder(void)
{
// Make sure both arrays have the same number of elements:
   assert(sizeof(::namelist)==sizeof(::vallist));

   MyTools mt;
   StackBuilder<strclass> sb(mt);
   sb.build();
}



const char *spairs[][2] = {
   { "one", "first" },
   { "two", "second" },
   { "three", "third" },
   { "four", "fourth" },
   { "five", "fifth" },
   { "six", "sixth" },
   { "seven", "seventh" },
   { "eight", "eighth" },
   { "nine", "ninth" },
   { "ten", "tenth" }
};

typedef const char*  cptype;

/**
 * @brief Build a DataStack that maps one string to another.
 */
//! [DataStack_Building_Snippet_Simple_String_Map]
void new_datastack_builder(IDataStack_User<strclass> &user)
{
   int elcount = sizeof(spairs) / sizeof(char*[2]);

   line_handle *array = static_cast<line_handle*>(alloca(elcount*sizeof(char*[2])));
   
   t_handle<strclass> *head = nullptr;
   t_handle<strclass> *tail = nullptr;
   
   for (int i=0; i<elcount; ++i)
   {
      // Get info about the data:
      const char *name = spairs[i][0];
      const char *value = spairs[i][1];
      size_t len_extra = strlen(value) + 1;

      // Calculate the memory size needed to represent the data:
      size_t len_line = t_handle<strclass>::line_size(name, len_extra);
      // Allocate and initialize the name and DMAP
      tail = t_handle<strclass>::init_handle(alloca(len_line), name, len_extra, tail);
      array[i] = tail->lhandle();

      if (!head)
         head = tail;

      // Initialize the data:
      strclass &tdata = tail->object();
      char *extra = static_cast<char*>(tail->extra());
      tdata.str = extra;
      strcpy(extra, value);

      // point the T data to the extra data:
   }

   DataStack<strclass> ds(array, elcount);
   user.use(ds);
}
//! [DataStack_Building_Snippet_Simple_String_Map]

void test_new_datastack_user(void)
{
   const char *captured = "CAPTURED";
   auto f = [&captured](DataStack<strclass> &ds) ->void
   {
      for (auto *ptr = ds.start(); ptr; ptr=ptr->next())
         printf("%s = %s (%s)\n", ptr->str(), ptr->object().str, captured);
   };

   TDataStack_User<strclass, decltype(f)> tds_u(f);

   new_datastack_builder(tds_u);
}

int main(int argc, char **argv)
{
//   test_stackbuilder();
   test_new_datastack_user();
   return 0;
}


#endif // INCLUDE_MAIN
