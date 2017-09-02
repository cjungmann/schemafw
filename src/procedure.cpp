// -*- compile-command: "g++ -std=c++11 -Wno-pmf-conversions -Wall -Werror -Weffc++ -pedantic -DINCLUDE_PROC_MAIN `mysql_config --cflags` -o procedure procedure.cpp `mysql_config --libs`" -*-

#include "procedure.hpp"
#include "bindstack.hpp"

// This seems like too common a name to not possibly conflict with another API:
#include <errmsg.h>

#include <unistd.h>
#include <termios.h>

#include <mysql/mysqld_error.h>

/** @brief Default implementation of message_reporter for SimpleProcedure. */
void default_procedure_message_reporter(const char *type,
                                        const char *msg,
                                        const char *where)
{
   ifputc('\n', stderr);
   ifputs(type, stderr);
   ifputs(" \"", stderr);
   ifputs(msg, stderr);
   if (where)
   {
      ifputs("\" at \"", stderr);
      ifputs(where, stderr);
   }
   ifputs("\"\n", stderr);
}

message_reporter SimpleProcedure::s_message_reporter = default_procedure_message_reporter;



/**
 * @brief Sets multiple values from a stream of name-value pairs.
 *
 * @todo Process http escape characters
 * @todo (Less important) Process paged parameters.
 *
 * The purpose of this function is to process http posted data from stdin.
 * It will need to convert +'s to spaces, %-prefixed values to characters,
 * and %% to a single %.
 */
bool Streamer_Setter::set_binds(DataStack<BindC> &ds) const
{
   SegmentStreamer s_name(m_str,'=');
   SegmentStreamer s_value(m_str,'&');

   char name[80];
   ai_text aname(name,80);

   while (!m_str.eof())
   {
      aname.set_from_streamer(s_name);

      // Check for and break on empty stream:
      if (aname.data_length()==0 && m_str.eof())
         break;
      
      // find index of name in ds
      int index = ds.index_by_name(name);

      if (index>=0)
      {
         t_handle<BindC> &handle = ds[index];
         handle.object().set_from(s_value);

         // Eat characters up to and including the field separator:
         if (m_str.recent()!='&')
            s_value.eat_value();
      }
      else if (strlen(name))
      {
         s_value.eat_value();
         fprintf(stderr, "Unable to find bound variable \"%s\"\n", aname.str());
      }
   }

   return true;
}

/**
 * @brief Simple function that returns 'true' for y or Y, false for n, N, or cancel.
 */
bool continue_yn(const char *prompt)
{
   struct termios oldt;
   struct termios newt;
   tcgetattr(STDIN_FILENO, &oldt);          /* save old settings */
   newt = oldt;                             /* copy old settings before modifying */
   newt.c_lflag &= ~(ICANON | ECHO);        /* make changes */
   tcsetattr(STDIN_FILENO, TCSANOW, &newt); /* apply changes */

   ifputs(prompt, stdout);
   ifputs(" [y/n]: ", stdout);

   int response;
   const char *allowed = "NYny";
   const char *chosen;
   while ((response=getc(stdin))!=EOF)
   {
      ifputs("\b \b", stdout);
      
      if ((chosen=strchr(allowed,response)))
         break;
   }
   
   tcsetattr(STDIN_FILENO, TCSANOW, &oldt); /*reapply the old settings */

   // Add newline to subsequent output is not on the same line:
   ifputc('\n', stdout);

   if (chosen)
      return 1 == (chosen - allowed) %2;
   else
      return false;
}

/**
 * @brief Presents prompts on the command line, waits for responses.
 *
 * @todo Redo to wait for several non-text characters that can more
 * forward, backward, or terminate early.
 *
 * This is pretty dumb right now, it simple presents each item
 * without the option to return to a previous item nor terminate
 * early.
 */
bool CommandLine_Setter::set_binds(DataStack<BindC> &ds) const
{
   StrmStreamer s(stdin);
   SegmentStreamer segs(s, '\n');
   
   while (getc(stdin)!='\n');
      
   for (unsigned i=0, stop=ds.count(); i<stop; ++i)
   {
      t_handle<BindC> &handle = ds[i];

      ifputs("Gimme the value of ", stdout);
      ifputs(handle, stdout);
      ifputs(": ", stdout);

      handle.object().set_from(segs);
   }

   return continue_yn("Save these values");
}



void Result_User_Base::print_string(BindC &bc, unsigned column)
{
   auto chars_to_read = bc.data_length();
   auto buffer_len = bc.buffer_length();

   assert(buffer_len > 0);
   
   unsigned long position = 0;
   char *buff = static_cast<char*>(bc.m_data);
   char *ptr = buff;
   char *end = buff + buffer_len;

   // assert that there's room to copy data:
   assert(end > ptr);
         
   while (chars_to_read)
   {
      // Get more characters, we've exhausted current buffer:
      if (ptr>=end)
      {
         position += buffer_len;
         int result = mysql_stmt_fetch_column(m_stmt, bc, column, position);
         if (result==0)
            ptr = buff;
         else
         {
            if (result==CR_INVALID_PARAMETER_NO)
               throw std::runtime_error("Unrecoverable error: invalid column number.");
            // If MYSQL indicates there's no more data, don't argue, just leave:
            else if (result==CR_NO_DATA)
               break;
         }
      }

      if (ptr < end)
      {
         print_char_as_xml(*ptr, m_out);
         --chars_to_read;
         ++ptr;
      }
   }
}



void Result_As_XML::pre_fetch_use_result(int result_number,
                                         DataStack<BindC> &dsresult,
                                         SimpleProcedure &proc) 
{
   fprintf(m_out, "<result number=\"%d\">\n", result_number);
}

void Result_As_XML::use_result_row(int result_number, DataStack<BindC> &dsresult)
{
   ifputs("<item", m_out);
   for (unsigned i=0, stop=dsresult.count(); i<stop; ++i)
   {
      auto &line = dsresult[i];
      BindC &obj = line.object();
      
      ifputc(' ', m_out);
      ifputs(line, m_out);
      ifputs("=\"", m_out);

      /**
       * @todo I think string types should be printed in Result_User_Base rather
       * that in a IClass.  IClass shouldn't have to know about handling a
       * truncation.  And IClass probably shouldn't have to know about printing
       * XML, either.
       */

      if (obj.is_string_type())
         print_string(obj, i);
      else
         obj.print(m_out);

      ifputc('\"', m_out);
   }

   ifputs(" />\n", m_out);
}
void Result_As_XML::result_complete(int result_number, DataStack<BindC> &ds)
{
   ifputs("</result>\n", m_out);
}

void Result_As_XML::report_error(const char *str)
{
   ifputs("<message type=\"error\">", m_out);
   ifputs(str, m_out);
   ifputs("</message>\n", m_out);
}


void SimpleProcedure::bind_result_and_run(BindStack &bs)
{
   mysql_stmt_bind_result(m_stmt, bs.binds());
   
   if (m_user)
   {
      m_user->pre_fetch_use_result(m_result_number, bs, *this);
      if (running())
      {
         while (fetch(bs))
            m_user->use_result_row(m_result_number, bs);

         m_user->result_complete(m_result_number, bs);
      }
   }
   // If no user assigned, just bleed-out the results:
   else if (running())
      while (fetch(bs))
         ;
}

/**
 * @brief Call this method to finish with and close the statement.
 *
 * This method is needed by SchemaProc to avoid OUT_OF_SYNC errors.
 * SchemaProc runs a SimpleProcedure to get the parameters of a
 * stored procedure and, because of how the memory is allocated,
 * SchemaProc can't wait for SimpleProcedure to go out of scope to
 * close the MYSQL_STMT.  This method expedites that closure.
 */
void SimpleProcedure::conclude(void)
{
   if (m_stmt)
   {
      // Finish current result:
      while (mysql_stmt_fetch(m_stmt)==0)
         ;
      // Skip subsequent results:
      while (mysql_stmt_next_result(m_stmt)==0)
         ;

      // close and clear m_stmt so it can be known to be closed.
      mysql_stmt_close(m_stmt);
      m_stmt = nullptr;
   }
}

/**
 * @brief Perform the fetch().  Prepare DataStack if a truncated row detected.
 */
bool SimpleProcedure::fetch(DataStack<BindC> &ds)
{
   m_truncated_row = false;
   
   int result = mysql_stmt_fetch(m_stmt);
   switch(result)
   {
      case MYSQL_DATA_TRUNCATED:
         m_truncated_row = true;
      case 0:
         return true;

      default:
         report_error(result, "stmt_fetch");
      case MYSQL_NO_DATA:   // have previously fetched the final row:
         return false;
   }
}

void SimpleProcedure::process_current_result(void)
{
   if (mysql_stmt_field_count(m_stmt)>0)
   {
      MYSQL_RES *mresult = mysql_stmt_result_metadata(m_stmt);
      if (!mresult)
         report_error(-1, "result_metadata");
      else
      {
         // Ensure mresult is freed with try-catch:
         try
         {
            auto f = [this](BindStack &bs)
            {
               bind_result_and_run(bs);
            };
            BindStack::build(mresult, f);
            
            // Result_Stacker stacker(mresult);
            // stacker.build(*this);
            
            // BindInfo_result bi_r(mresult);
            // BindCPool::build(bi_r, *this);
         }
         catch(const std::runtime_error &e)  // more e.what() info for runtime_error
         {
            mysql_free_result(mresult);
            throw e;
         }
         catch(const std::exception &e)
         {
            mysql_free_result(mresult);
            throw e;
         }
         
         mysql_free_result(mresult);
      }
   }
}


void SimpleProcedure::loop_through_results(void)
{
   if (m_user)
      m_user->register_stmt(m_stmt);

   auto err_rpt = [this](const char *msg)
   {
      // Save error to log:
      ifputs(msg, stderr);
      ifputc('\n', stderr);
         
      // report error to client:
      if (m_user)
         m_user->report_error(msg);
   };

   int result = 0;
   while (!result)
   {
      ++m_result_number;
      
      // Ensure mysql_stmt_next_result() loop runs to completion
      // if there are exceptions thrown from fetch().
      try
      {
         process_current_result();
      }
      catch(std::runtime_error &e)
      {
         err_rpt(e.what());
      }
      catch(const std::exception &e)
      {
         err_rpt(e.what());
      }

      // If conclude() has been called, m_stmt will be NULL;
      // break out of the loop.
      if (m_stmt)
         result = mysql_stmt_next_result(m_stmt);
      else
         break;
   }

   if (m_user)
      m_user->register_stmt(nullptr);


   // -1 means no more results, 0 continues loop, so anything else is an error:
   if (result>0)
      report_error(result, "next_result");
}

/**
 * @brief Run the prepared procedure.
 *
 * Either or both of the last two parameters (_ps_ and _ru_) can be NULL.
 *
 * Taking the last first, if _ru_ is null, the results will be discarded.
 *
 * Parameter _ps_ can be NULL for procedure that have no parameters, or
 * an error will occur.  If _ps_ does have a value, there must be an
 * array of MYSQL_BIND (SimpleProcedure::m_binds), and a DataStack<BindC>
 * with which the elements of the MYSQL_BIND array will be set.
 */
void SimpleProcedure::run(MYSQL *conn, const IParam_Setter *ps, IResult_User *ru)
{
   bool ok_to_execute = true;
   if ((m_stmt=mysql_stmt_init(conn)))
   {
      m_conn = conn;
      int result = mysql_stmt_prepare(m_stmt, m_query, strlen(m_query));
      if (result)
         report_error(result, "prepare");
      else
      {
         if (ps && m_binds && m_dstack)
         {
            ok_to_execute = false;
            
            result = mysql_stmt_bind_param(m_stmt, m_binds);
            if (result)
               report_error(result, "bind_param");
            else if (ps)
            {
               ok_to_execute = ps->set_binds(*m_dstack);
            }
         }

         if (ok_to_execute)
         {
            result = mysql_stmt_execute(m_stmt);
            if (result)
               report_error(result, "execute");
            else
            {
               try
               {
                  m_user = ru;
                  loop_through_results();
               }
               catch(const std::runtime_error &e)
               {
                  bleed_procedure();
                  throw e;
               }
            }
         }
      }

      if (m_stmt)
      {
         mysql_stmt_close(m_stmt);
         m_stmt = nullptr;
      }
   }
   else
      ifputs("Failed to initialize the statement.\n", stderr);
}

void SimpleProcedure::report_error(const char *message, const char *where) const
{
   (*s_message_reporter)("error", message, m_query);
}

void SimpleProcedure::report_error(int result, const char *where) const
{
   // This should only be called if result!=0;
   assert(result);

   size_t len_query = strlen(m_query);
   size_t len_where = where ? strlen(where) : 0;
   size_t len_detail = len_query + (len_where ? len_where+4 : 0);

   // Make buffer for concatenated string:
   char *detail = static_cast<char*>(alloca(len_detail+1));

   // Build string with moving pointer into the buffer:
   char *ptr = detail;
   if (len_where)
      *ptr++ = '(';
   memcpy(ptr, m_query, len_query);
   ptr += len_query;
   if (len_where)
   {
      *ptr++ = ')';
      *ptr++ = ' ';
      memcpy(ptr, where, len_where);
      ptr += len_where;
   }
   *ptr = '\0';

   // Virtually all errors should be the result of calling mysql_stmt_execute,
   // so check it first for the error string.  Failing there, check for a
   // connection error
   const char *str = mysql_stmt_error(m_stmt);
   if (*str==0)
      str = mysql_error(m_conn);
   
   // Always get the stmt error:
   int stmterr = mysql_stmt_errno(m_stmt);
   if (stmterr==ER_SIGNAL_EXCEPTION)
      (*s_message_reporter)("signal", str, detail);
   else
      (*s_message_reporter)("error", str, detail);
}


/** @brief Simple, private helper class for SimpleProcedure::build. */
class SP_Builder : public IBindUser
{
   MYSQL               *m_conn;
   const char          *m_query;
   const IParam_Setter *m_setter;
   IResult_User        *m_user;
public:
   SP_Builder(MYSQL *conn,
              const char *query,
              const IParam_Setter *ps,
              IResult_User *ru)
      : m_conn(conn), m_query(query), m_setter(ps), m_user(ru) { }
   EFFC_3(SP_Builder);
   
   virtual void use(DataStack<BindC> *ds, MYSQL_BIND *binds, unsigned count)
   {
      SimpleProcedure su(m_query, ds, binds);
      su.run(m_conn, m_setter, m_user);
   }
};

/**
 * @brief Simple function to ease the use a SimpleProcedure.
 *
 * This static function creates and uses the necessary callback
 * classes so one can use a SimpleProcedure without having to
 * define all those things.
 *
 * @param conn    An open MySQL connection
 * @param query   A query string like `CALL App_My_Proc(?)`
 * @param typestr A string from which to build an input BindStack
 *                (@ref TypestrDefinitions)
 * @param ps      An IParam_Setter derived object for setting the input parameters
 *                BindStack
 * @param ru      An IResult_User derived object for consuming the DataStack<BindC>
 *                objects generated by SimpleProcedure.
 */
// void SimpleProcedure::build(MYSQL *conn, const char *query, IBindInfo *bi,
//                             const IParam_Setter *ps, IResult_User *ru)
// {
//    if (bi)
//    {
//       SP_Builder sp_b(conn, query, ps, ru);
//       BindCPool::build(*bi, sp_b);
//    }
//    else
//    {
//       SimpleProcedure su(query);
//       su.run(conn, ps, ru);
//    }
// }

void SimpleProcedure::build(MYSQL *conn, const char *query, const char *typestr,
                     const IParam_Setter *ps, IResult_User *ru)
{
   auto f = [&](BindStack &bs)
   {
      SimpleProcedure sp(query, &bs, bs.binds());
      sp.run(conn, ps, ru);
   };
   
   BindStack::build(typestr, f);
}

void SimpleProcedure::build(MYSQL *conn, const char *query,
                            MYSQL_RES *result,
                            const IParam_Setter *ps, IResult_User *ru)
{
   auto f = [&](BindStack &bs)
   {
      SimpleProcedure sp(query, &bs, bs.binds());
      sp.run(conn, ps, ru);
   };
   
   BindStack::build(result, f);
}


/**
 * @brief Utility function to build a query string and return it to the caller
 *        using a callback function.
 *
 * 
 */
void SimpleProcedure::t_build_query_string(const char *procname,
                                           int count_params,
                                           const IGeneric_String_Callback &gsb)
{
   // Create the query string:
   int len_name = strlen(procname);
   int len_qs = count_params ? 1+(2*count_params) : 2;
   int len_query = len_name + len_qs + 6; // 6 = "CALL_" + '\0'
   char *query = static_cast<char*>(alloca(len_query));

   // ptr points to next unused position:
   char *ptr = query;
   memcpy(ptr, "CALL ", 5);
   ptr += 5;
   memcpy(ptr, procname, len_name);
   ptr += len_name;

   // Now ptr will point to the last copied character:
   *ptr = '(';
   for (int i=0; i<count_params; ++i)
   {
      *++ptr = '?';
      *++ptr = ',';
   }

   // If the last character is a comma, there were at least one
   // parameter, and the comma should be changed to a close-paren.
   if (*ptr==',')
      *ptr = ')';
   // If the last character is not a comma, there were no
   // parameters.  We need to advance past the open-paren
   // and follow it with a close-paren:
   else
      *++ptr = ')';

   // advance past the close-paren:
   ++ptr;

   // Assert that we haven't overrun the buffer:
   assert(ptr <= query + len_query);

   // Terminate the string:
   *ptr = '\0';

   gsb(query);
}




#ifdef INCLUDE_PROC_MAIN

#include "istdio.cpp"
#include "prandstr.cpp"
#include "vclasses.cpp"
#include "ctyper.cpp"
#include "bindc.cpp"
#include "datastack.cpp"
#include "bindstack.cpp"


/**
 * @brief Sample using Adhoc_Setter to run a procedure.
 *
 * I had trouble with this test because I forgot an important
 * consideration of Adhoc_Setter: its constructor parameters can
 * be temporary arguments if and only if the Adhoc_Setter object
 * is itself a temporary argument.
 *
 * Thus, in this example, you see param_catalog instantiated
 * individually and included in the argument list of Adhoc_Setter.
 * This is to handle scope issues with temporary variables.  The
 * Adhoc_Setter documentation attempts to explain this issue.
 */
void test_simple_procedure_with_bindstack(MYSQL &mysql)
{
  const char *query = "SELECT * FROM SCHEMATA WHERE CATALOG_NAME=?";
   
   Result_As_XML rax(stdout);

   // This one works:
   SimpleProcedure::build(&mysql, query, "s64",
                          Adhoc_Setter<1>(si_text("def")),
                          rax);

   // This one works because param_catalog is instantiated separately:
   // si_text param_catalog("def");
   // Adhoc_Setter<1> setter(param_catalog);
   // SimpleProcedure::build(&mysql, query, "s64",
   //                        &setter,
   //                        &rax);

   // The previous call would have failed if Adhoc_Setter was make like this:
   // Adhoc_Setter<1> setter(si_text("def"));
}

void test_procedure_with_date(MYSQL &mysql)
{
   CommandLine_Setter cl_s;
   Result_As_XML rax(stdout);
   
   const char *query = "CALL App_Person_Submit(?,?,?)";
   SimpleProcedure::build(&mysql, query, "is30t",
                           &cl_s,
                           &rax);
}

/**
 * @brief Test and demonstrate how to call the ssys_get_procedure_params.
 *
 * Using lambda function and Adhoc_Setter for 
 */
void test_parameter_getter(MYSQL &mysql, const char *procname)
{
   auto fBS = [&mysql, &procname](BindStack &bs)
   {
      Result_As_XML user(stdout);
      SimpleProcedure sp("CALL ssys_get_procedure_params(?)", &bs);
      sp.run(&mysql,
             Adhoc_Setter<1>(si_text(procname)),
             &user);
   };
   BindStack::build("s64", fBS);
}

void test_paged_field(MYSQL &mysql)
{
   const char *query = "SELECT COLUMN_NAME,"
                       "       CHAR_LENGTH(COLUMN_TYPE),"
                       "       COLUMN_TYPE"
                       "  FROM information_schema.COLUMNS"
                       " WHERE CHAR_LENGTH(COLUMN_TYPE) > 100";

   Result_As_XML rax(stdout);
   SimpleProcedure::build(&mysql, query,rax);
}

void test_build_query_string(void)
{
   auto f = [](const char* query)
   {
      if (query)
         ifputs(query, stdout);
      else
         ifputs("Query not returned.", stdout);

      ifputc('\n', stdout);
   };
   SimpleProcedure::build_query_string("App_Bogus_Proc", 5, f);
}

void test_make_typestr(void)
{
   auto f = [](const char *typestr)
   {
      printf("Returned with typestr=\"%s\".\n", typestr);
   };
   
   const char *tname = "Session_Import_People";
   int limit_rows = 0;
   Adhoc_Setter<2>(si_text(tname), ri_long(limit_rows)).make_typestr(f);
}

int main(int argc, char** argv)
{
   test_make_typestr();
   test_build_query_string();
   return 0;
   
   MYSQL mysql;
   // Read credentials from the ~/.my.cnf 
   mysql_init(&mysql);
   mysql_options(&mysql,MYSQL_READ_DEFAULT_GROUP,"");

   const char *host = nullptr;
   const char *user = nullptr;
   const char *password = nullptr;
//   const char *database = "information_schema";
   const char *database = "AllowanceDemo";

   if (mysql_real_connect(&mysql, host, user, password, database, 0, nullptr, 0))
   {
//      test_simple_procedure_with_bindstack(mysql);
//      test_paged_field(mysql);

//      mysql_select_db(&mysql, "Allowances");
      test_procedure_with_date(mysql);
//      test_parameter_getter(mysql, "App_Person_Submit");
      mysql_close(&mysql);
   }

   mysql_library_end();
   
   return 0;
}

#endif
