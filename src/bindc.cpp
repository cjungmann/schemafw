// -*- compile-command: "g++ -std=c++11 -Wall -Werror -Weffc++ -pedantic -DINCLUDE_BC_MAIN `mysql_config --cflags` -U NDEBUG -o bindc bindc.cpp `mysql_config --libs`" -*-

#include "bindc.hpp"

void BindC::initialize(MYSQL_BIND *bind,
                       void *extra,
                       size_t len_extra,
                       const _CType *ctype)
{
   // ensure we don't initialize with a zero-initialized bind*
   
   m_bind = bind;
   m_data = extra;

   // If ctype supplied, use it to initialize bind:
   if (ctype)
   {
      bind->buffer_type = ctype->get_sqltype();
      m_typeinfo = ctype;
   }
   else
   {
      // If no ctype, *bind better be initialized
      assert(bind->buffer_type!=MYSQL_TYPE_DECIMAL);  // MYSQL_TYPE_DECIMAL==0
      m_typeinfo = CTyper::get(bind);
   }
      
   bind->buffer = extra;
   bind->buffer_length = len_extra;
   bind->length = &m_length;
   bind->is_null = &m_is_null;
   bind->error = &m_error;

   // The bind should be NULL before anything is added:
   m_is_null = true;
   
//   m_caster = get_caster(bind);
}

void BindC::set_from(const IClass &rhs)
{
   _set_from c(rhs);
   use_cast(c);
   m_length = rhs.data_length();
   m_is_null = false;
}

void BindC::set_from(IStreamer &s)
{
   _set_from_streamer c(s);
   use_cast(c);
   m_length = c.copy_length();
   m_is_null = false;
}

void BindCPool::set_data(BindC *obj, void *extra, size_t len_extra)
{
   MYSQL_BIND &bind = m_binds[m_index];
   
   m_bindinfo.set_buffer_type(bind);
   
   obj->m_bind = &bind;
   bind.buffer = obj->m_data = extra;
   bind.buffer_length = len_extra;
   
   bind.length = &obj->m_length;
   bind.is_null = &obj->m_is_null;
   bind.error = &obj->m_error;

   obj->m_typeinfo = m_bindinfo.get_ctype();
   obj->m_format = 0;
}

void BindCPool::build(IBindInfo &bi, IBindUser &bu)
{
   unsigned count = bi.count();
   MYSQL_BIND *binds = static_cast<MYSQL_BIND*>(alloca(count*sizeof(MYSQL_BIND)));
   memset(binds, 0, count*sizeof(MYSQL_BIND));

   BindCPool bpool(bi, bu, binds, count);
   StackBuilder<BindC> sb(bpool);
   sb.build();
}

void clear_stack(DataStack<BindC> &ds)
{
   unsigned stop = ds.count();
   for (unsigned i=0; i<stop; ++i)
   {
      BindC &bind = ds.object(i);
      bind.m_is_null = false;
   }
}



#ifdef INCLUDE_BC_MAIN

#include "istdio.cpp"
#include "prandstr.cpp"
#include "vclasses.cpp"
#include "ctyper.cpp"
#include "bindinfo_str.hpp"

#include "bindinfo_str.cpp"
#include "datastack.cpp"



class DemoBindUser : public IBindUser
{
public:
   virtual ~DemoBindUser() { }

   virtual void use(DataStack<BindC> *ds, MYSQL_BIND *binds, unsigned count)
   {
      printf("We got here!\n");
      
      BindC &int1 = ds->object(0);
      BindC &int2 = ds->object(1);
      BindC &date1 = ds->object(2);
      BindC &str1 = ds->object(3);

      si_text stext("This is the str1.");

      int1 = ri_long(12);
      int2 = ri_long(42);
      date1 = ri_date(mydate(1997,6,6));
      str1 = si_text("This is the str1.");

      for (unsigned i=0; i<count; i++)
      {
         BindC &b = ds->object(i);
         printf("row %d: \"", i);
         b.print(stdout);
         fputs("\"\n", stdout);
      }
   }
};

void test_make_bind_str(void)
{
   fputs("About to run test_make_bind_str().\n",stdout);
   const char *bstr = "iits64";
   BindInfo_str bi_s(bstr);

   DemoBindUser dbuser;
   BindCPool::build(bi_s, dbuser);
}


int main(int argc, char** argv)
{
   test_make_bind_str();
   return 0;
}


#endif // INCLUDE_BC_MAIN
