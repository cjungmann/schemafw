// -*- compile-command: "g++ -std=c++11 -fno-inline -Wall -Werror -Weffc++ -pedantic -DINCLUDE_STOREDPROC_MAIN `mysql_config --cflags` -o storedproc storedproc.cpp `mysql_config --libs`" -*-

/** @file */

#include "storedproc.hpp"

/**
 * @brief Class to combine value access and null-detection into a single object.
 *
 * @tparam T An IClass-derived type to alias the data.
 * @tparam R The data type of this class. Must match the get_value() type of T.
 */
template <class T, typename R>
class BindEl
{
protected:
   BindC &m_bind;
   T     m_field;
public:
   BindEl(DataStack<BindC> &ds, int index)
      : m_bind(ds.object(index)), m_field()
   {
      // for debugging:
      // printf("%s\n", ds[index].str());
      
      m_bind.install_to_alias(m_field);
   }

   bool is_null(void) const          { return m_bind.is_null(); }
   operator R(void) const            { return static_cast<R>(m_field); }
   operator const BindC&(void) const { return m_bind; }
};

/** @brief Confirms the name, position, and type of a DataStack element. */
template <class T>
bool check_bind_element(DataStack<BindC> &ds, const char *name, int index)
{
   if (index>=ds.count())
      throw std::runtime_error("check_bind_element index out of range.");
   
   if (ds[index]==name)
   {
      T tester;
      BindC &bind = ds.object(index);
      if (bind.is_matched_type(tester))
         return true;
      else
      {
         class_types ttester = tester.vtype();
         class_types tbind = bind.vtype();
         char buff[200];
         snprintf(buff,200, "type mismatch this type is %d, the tester is %d",
                  static_cast<int>(tbind),
                  static_cast<int>(ttester));
         throw std::runtime_error(buff);
      }
   }
   else
   {
      char buff[200];
      snprintf(buff, 200,
               "name mismatch: seeking \"%s\", found \"%s\"",
               name,
               ds[index].str());
      throw std::runtime_error(buff);
   }
   return false;
}

int Result_User_Build_Schema::s_checked_result_1 = 0;
void Result_User_Build_Schema::check_result_1(DataStack<BindC> &ds)
{
   if (!s_checked_result_1 && !check_bind_element<ai_longlong>(ds,"parameter_count",0))
      s_checked_result_1 = -1;

   if (s_checked_result_1==-1)
      throw std::runtime_error("Result_User_Build_Schema mismatched fields in first result");
   else
      s_checked_result_1 = 1;
}

int Result_User_Build_Schema::s_checked_result_2 = 0;
void Result_User_Build_Schema::check_result_2(DataStack<BindC> &ds)
{
   int index = 1;
   if (!s_checked_result_2 && !check_bind_element<ai_text>(ds,"name",index++))
      s_checked_result_2 = -1;
   if (!s_checked_result_2 && !check_bind_element<ai_text>(ds,"dtype",index++))
      s_checked_result_2 = -1;
   if (!s_checked_result_2 && !check_bind_element<ai_long>(ds,"len",index++))
      s_checked_result_2 = -1;
   if (!s_checked_result_2 && !check_bind_element<ai_long>(ds,"num_prec",index++))
      s_checked_result_2 = -1;
   if (!s_checked_result_2 && !check_bind_element<ai_long>(ds,"num_scale",index++))
      s_checked_result_2 = -1;
   if (!s_checked_result_2 && !check_bind_element<ai_text>(ds,"dtdid",index++))
      s_checked_result_2 = -1;

   if (s_checked_result_2==-1)
      throw std::runtime_error("SchemaProc mismatched fields in second result");
   else
      s_checked_result_2 = 1;
}

/** @brief Process result_number==1 for parameter count. */
void Result_User_Build_Schema::get_param_count(DataStack<BindC> &result1, SimpleProcedure &proc)
{
   BindEl<ai_long, int32_t> count(result1,0);
   if (proc.fetch(result1))
      m_param_count = count;
}

/**
 * @brief Create the MYSQL_BIND array and DataStack<BindC> to hand off to next step.
 *
 * @todo <b>Priority</b><br />There could be an issue with dtdid being too long.
 * I'm not checking it right now, in the interest of finishing the project, but
 * this should be looked into.  It shouldn't be a problem checking for unsigned,
 * the first use of this field, but when we use this information for schemas, it
 * is important to realize that enum and set types do not have a length limit.
 */
void Result_User_Build_Schema::build_param_stack(DataStack<BindC> &result2, SimpleProcedure &proc)
{
   // Don't build if no parameters or, disaster, m_param_count<0, procedure not found:
   assert(m_param_count>0);
   
   // Make a handle for each field:
   int index=1;
   BindEl<ai_text, const char*> name(result2,index++);
   BindEl<ai_text, const char*> dtype(result2,index++);
   BindEl<ai_long, int32_t>     maxlen(result2,index++);
   BindEl<ai_long, int32_t>     num_prec(result2,index++);
   BindEl<ai_long, int32_t>     num_scale(result2,index++);
   BindEl<ai_text, const char*> dtdid(result2,index++);
//   BindEl<ai_longlong, int64_t>     is_unsigned(result2,index++);

   // Create and initialize the MYSQL_BIND array:
   size_t bindslen = sizeof(MYSQL_BIND) * m_param_count;
   MYSQL_BIND *binds = static_cast<MYSQL_BIND*>(alloca(bindslen));
   memset(binds, 0, bindslen);
   
   line_handle h_top = nullptr;
   line_handle h_bottom = nullptr;

   int i=0;
   while (proc.fetch(result2))
   {
      auto *ctype = CTyper::get(dtype, dtdid);

      size_t datalen;
      if (maxlen.is_null())
         datalen = ctype->get_size();
      else
         datalen = maxlen;

      if (!datalen)
      {
         switch (ctype->m_sqltype)
         {
            case MYSQL_TYPE_DECIMAL:
               // Add one for comma, one for (unused, I know) 0-terminator:
               datalen = get_decimal_length(dtdid) + 2;
               break;
            default:
               break;
         }
      }

      size_t linelen = base_handle::get_line_handle_size(name,
                                                         sizeof(BindC),
                                                         datalen);

      char *buffer = static_cast<char*>(alloca(linelen));
      memset(buffer,0,linelen);
      BindC &bindc = *reinterpret_cast<BindC*>(buffer);
      bindc.m_typeinfo = ctype;
      bindc.m_str_length = maxlen;

      h_bottom = base_handle::init_line_handle(buffer,
                                               name,
                                               sizeof(BindC),
                                               datalen,
                                               h_bottom);

      if (!h_top)
         h_top = h_bottom;

      MYSQL_BIND &bind = binds[i];

      bind.buffer_type = ctype->get_sqltype();
      bind.is_unsigned = ctype->is_unsigned();
      
      bindc.initialize(&bind,
                       static_cast<void*>(buffer+sizeof(BindC)),
                       datalen);

      switch(ctype->get_sqltype())
      {
         case MYSQL_TYPE_ENUM:
         case MYSQL_TYPE_SET:
         {
            // These types are invalid buffers, so change the
            // bind buffer type to MYSQL_TYPE_VARCHAR:
            bind.buffer_type = MYSQL_TYPE_VARCHAR;

            // Save the DTD_IDENTIFIER to later parse for options:
            size_t dtdlen = strlen(dtdid);
            char *buff = static_cast<char*>(alloca(dtdlen+1));
            memcpy(buff, dtdid, dtdlen+1);
            bindc.m_dtdid = buff;
            break;
         }
            
         default:
            break;
      }

      ++i;
   }

   // Clean-up and close statement to clear way for next one:
   proc.conclude();

   // Build stuff for use command:
   size_t array_byte_len = m_param_count * sizeof(line_handle);
   line_handle *array = static_cast<line_handle*>(alloca(array_byte_len));
   base_handle::fill_array(array, m_param_count, h_top);

   BindStack nds(array, m_param_count);
   m_cb(&nds);
}

/**
 * @brief Parse and return the first number of the MYSQL_TYPE_DECIMAL pair.
 */
size_t Result_User_Build_Schema::get_decimal_length(const char *dtdid)
{
   size_t rval = 0;
   if (strncmp(dtdid,"decimal",7)==0)
   {
      const char *num = &dtdid[8];
      const char *comma = strchr(num,',');
      if (!comma)
         comma = num+100;
      
      while (*num && num<comma)
      {
         rval += *num - '0';
         ++num;
      }
   }
   return rval;
}



/**
 * @brief Collects and saves name and data type information from stored procedure
 *        ssys_get_procedure_params.
 *
 * This function is use by StoredProc to collect parameter information.  This
 * function is notable also as an example of how to collect result information
 * for a stack-allocated BindStack.
 */
void Result_User_Build_Schema::pre_fetch_use_result(int result_number,
                                                    DataStack<BindC> &ds,
                                                    SimpleProcedure &proc)
{
   if (result_number==1)
   {
      if (!s_checked_result_1)
         check_result_1(ds);

      get_param_count(ds,proc);

      if (m_param_count<0)
      {
         const static char* pre = "Procedure '";
         const static char* post = "' not found.";
         const size_t len_pre = strlen(pre);
         const size_t len_post = strlen(post);
         const size_t len_extra = len_pre + len_post + 1;
         
         const DataStack<BindC> *bs = proc.get_bindstack();
         const BindC &bc = bs->start()->object();
         size_t len = bc.m_length;
         char *buff = static_cast<char*>(alloca(len+len_extra));
         char *p = buff;
         memcpy(p, pre, len_pre);
         p += len_pre;
         memcpy(p, bc.m_data, len);
         p += len;
         memcpy(p, post, len_post);
         p += len_post;
         *p = '\0';

         // flush procedure for next use:
         proc.conclude();
         throw std::runtime_error(buff);
      }
   }
   else if (result_number==2)
   {
      if (!s_checked_result_2)
         check_result_2(ds);
      
      if (m_param_count>0)
         build_param_stack(ds,proc);
      else
      {
         // flush procedure so we can run another query:
         proc.conclude();
         
         m_cb(nullptr);
      }
   }
}


/**
 * @brief Build the CALL procedure string.
 *
 * Assuming an adequate buffer length as a result of a previous call
 * to get_query_length, this function builds the CALL Procedure string
 * with the appropriate number of question marks in the parameter list.
 *
 * The string size calculation is simplified according to the following:
 *   CALL ProcName(?,?,?)
 *   5                "CALL "
 *   + strlen(ProcName)
 *   + param_count     question marks
 *   + param_count-1   commas
 *   + 2               parentheses
 *   + 1               '\0'
 *   reduce: 5 + strlen() + param_count + param_count - 1 + 2 + 1
 *   rearrange: strlen() + param_count + param_count -1 + 2 + 1 + 5
 *   reduce: strlen() + (param_count * 2) - 1 + 8
 *   reduce: strlen() + (param_count*2) + 7
 */
void StoredProc::set_query_string(char *buff,
                                  const char* name,
                                  unsigned param_count)
{
   int len = strlen(name);
   char *p = buff;
   memcpy(p,"CALL ",5);
   p += 5;
   memcpy(p,name,len);
   p += len;
   *p++ = '(';
   for (unsigned i=0; i<param_count; ++i)
   {
      if (i)
         *p++ = ',';
      *p++ = '?';
   }
   *p++ = ')';
   *p = '\0';
}
   

/**
 * @brief Static function that stack-allocates StoredProc member variables ahead of instantiation.
 *
 * @param mysql    An open MYSQL connection.  The connection must be ready to use
 *                 to avoid out-of-sync errors.  The build function will return the
 *                 connection to the ready state before invoking the callback.
 * @param procname The name of the procedure for which the BindStack is prepared.
 * @param cb       A callback class whose function data member will be called when
 *                 the new StoredProc object is complete and the MYSQL connection has
 *                 has been returned to the ready state.
 *
 * This function is the only way to create a StoredProc object because the data members
 * of the class are to be allocated from the stack rather than the heap, so invoking
 * the callback ensures that the data members are in scope when callback function runs.
 */
void StoredProc::build(MYSQL *mysql,
                       const char *procname,
                       IGeneric_Callback<StoredProc> &cb)
{
   // Create the IResult_User that calls cb with the StoredProc object:
   auto fBindStack = [&mysql, &procname, &cb](BindStack *bs)
   {
      unsigned pcount = bs ? bs->count() : 0;
      size_t plen = get_query_length(procname, pcount);
      char *querystr = static_cast<char*>(alloca(plen));
      set_query_string(querystr, procname, pcount);
      
      StoredProc sp(mysql, procname, querystr, bs);
      cb(sp);
   };
   Generic_User_Pointer<BindStack, decltype(fBindStack)> gu(fBindStack);
   Result_User_Build_Schema rubs(gu);

   // Callback that creates the DataStack<BindC> for ssys_get_procedure_params.
   // Note that this function calls the fBindStack lambda function.
   auto fParamStack = [&mysql, &procname, &rubs](BindStack &bs)
   {
      SimpleProcedure sp("CALL ssys_get_procedure_params(?)", &bs);
      sp.run(mysql,
             Adhoc_Setter<1>(si_text(procname)),
             &rubs);
   };
   // Build the DataStack<BindC> for the procedure name,
   BindStack::build("s64", fParamStack);
}

#ifdef INCLUDE_STOREDPROC_MAIN

#include "prandstr.cpp"
#include "vclasses.cpp"
#include "ctyper.cpp"
#include "bindc.cpp"
#include "datastack.cpp"
#include "bindstack.cpp"
#include "procedure.cpp"

#include <stdio.h>


void test_storedproc(MYSQL *mysql, const char *procname)
{
   auto f = [&mysql](StoredProc &sp)
   {
      printf("%s\n", sp.querystr());
      
      BindStack *bs = sp.bindstack();
                            
      if (bs)
      {
         CommandLine_Setter cl_s;
         Result_As_XML user(stdout);

         SimpleProcedure simpp(sp.querystr(), bs);
         simpp.run(mysql, &cl_s, &user);
         
         // for (auto *p=bs->start(); p; p=p->next())
         // {
         //    printf("arg %s\n", p->str());
         // }
      }
   };
   Generic_User<StoredProc, decltype(f)> spu(f);

   StoredProc::build(mysql, procname, spu);
}


int main(int argc, char **argv)
{
   MYSQL mysql;
   // Read credentials from the ~/.my.cnf 
   mysql_init(&mysql);
   mysql_options(&mysql,MYSQL_READ_DEFAULT_GROUP,"");

   const char *host = nullptr;
   const char *user = nullptr;
   const char *password = nullptr;
//   const char *database = "SchemaDemo";
   const char *database = "AllowanceDemo";

   if (mysql_real_connect(&mysql, host, user, password, database, 0, nullptr, 0))
   {
      test_storedproc(&mysql, "App_Person_Submit");

      mysql_close(&mysql);
   }

   mysql_library_end();

   
   return 0;
}






#endif // INCLUDE_STOREDPROC_MAIN
