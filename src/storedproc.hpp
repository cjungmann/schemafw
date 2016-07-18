// -*- compile-command: "g++ -std=c++11 -fno-inline -Wall -Werror -Weffc++ -pedantic -DINCLUDE_STOREDPROC_MAIN `mysql_config --cflags` -o storedproc storedproc.cpp `mysql_config --libs`" -*-

/** @file */

#ifndef STOREDPROC_HPP_SOURCE
#define STOREDPROC_HPP_SOURCE

#include "procedure.hpp"

/**
 * @brief An IResult_User that builds a schema BindStack
 *
 * Derived from Result_User_Base to implement register_stmt() pure virtual function.
 */
//class Result_User_Build_Schema : public Result_User_Base
class Result_User_Build_Schema : public IResult_User
{
protected:
   IGeneric_Callback_Pointer<BindStack> &m_cb;
   int m_param_count;
   
public:
   Result_User_Build_Schema(IGeneric_Callback_Pointer<BindStack> &cb)
      : m_cb(cb), m_param_count(0)                                       { }

   /** @brief Empty function because it's not necessary for this class. */
   virtual void register_stmt(MYSQL_STMT *stmt)                          { }
   virtual void pre_fetch_use_result(int result_number,
                                     DataStack<BindC> &ds,
                                     SimpleProcedure &proc);
   
   virtual void  use_result_row(int result_number, DataStack<BindC> &ds) { }
   virtual void result_complete(int result_number, DataStack<BindC> &ds) { }
   virtual void    report_error(const char *str)                         { }


protected:
   void get_param_count(DataStack<BindC> &result1, SimpleProcedure &proc);
   void build_param_stack(DataStack<BindC> &result2, SimpleProcedure &proc);

   /** @name Confirm DataStack elements' name, type, and position @{ */
   static int s_checked_result_1;
   static int s_checked_result_2;
   static void check_result_1(DataStack<BindC> &ds);
   static void check_result_2(DataStack<BindC> &ds);
   /**@}*/

};

/**
 * @brief Prepares a BindStack and query string from the parameters
 *        of a stored procedure to simplify the use of stored procedures.
 *
 * This class cannot be directly created, but rather is accessed through a static
 * function (StoredProc::build()) that queries the information_schema.PARAMETERS
 * table to build a BindStack with the appropriate names and data types.  Along
 * with the BindStack, a query string with the appropriate number of <i>?</i>s is also
 * generated to be used when running the procedure.
 *
 * The reason it cannot be directly created is because it will have allocated stack
 * memory for the query string and the BindStack (MYSQL_BIND array and associated
 * memory buffers).  Using a callback ensures that the member values are still
 * in scope when the callback is run.
 *
 ~~~sql
 // "Magically" acquire the required IParam_Setter and IResult_User objects.
 // Normally, they would be created in the same function as StoreProc::build is called.
 void demo_StoredProc(MYSQL *mysql,
                      const char *procname,
                      IParam_Setter *setter,
                      IResult_User *user)
 {
    auto f = [&mysql, &setter, &user](StoredProc &sproc)
    {
       SimpleProcedure::build(mysql,
                              sproc.querystr(),
                              sproc.bindstack(),
                              setter,
                              user);
    };
    Generic_User<StoredProc, decltype(f)> sp_user(f);

    StoredProc::build(mysql, procname, sp_user);
 }
 ~~~
 * @sa TheSimpleProcedure
 * @sa SimpleProcedure
 */
class StoredProc
{
protected:
   MYSQL      *m_mysql;
   const char *m_procname;
   const char *m_querystr;
   BindStack  *m_bindstack;

   StoredProc(MYSQL *mysql, const char *procname, const char *querystr, BindStack *bs)
      : m_mysql(mysql), m_procname(procname),
        m_querystr(querystr), m_bindstack(bs) { }

public:
   static void build(MYSQL *mysql,
                     const char *procname,
                     IGeneric_Callback<StoredProc> &sp);

   const char *procname(void) const       { return m_procname; }
   const char *querystr(void) const       { return m_querystr; }
   const BindStack* bindstack(void) const { return m_bindstack; }
   BindStack* bindstack(void)             { return m_bindstack; }

protected:
   /**
    * @brief Get buffer length required to hold the procedure call string.
    *
    * See set_query_string for an explanation of how the length is calculated.
    */
   static size_t get_query_length(const char *name, unsigned param_count)
   {
      return strlen(name) + param_count*2 + 7;
   }
   static void set_query_string(char *buff, const char *name, unsigned param_count);
   
};


#endif

