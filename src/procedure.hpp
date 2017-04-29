// -*- compile-command: "g++ -std=c++11 -Wno-pmf-conversions -Wall -Werror -Weffc++ -pedantic -DINCLUDE_PROC_MAIN `mysql_config --cflags` -o procedure procedure.cpp `mysql_config --libs`" -*-

/** @file */

#ifndef PROCEDURE_HPP_SOURCE
#define PROCEDURE_HPP_SOURCE

#include <mysql.h>
#include "vclasses.hpp"
#include "bindc.hpp"
#include "bindstack.hpp"

// Forward declaration for 
class SimpleProcedure;
class Procedure;
class IResult_User;

/**
 * @brief Function pointer typedef for SimpleProcedure to report progress and errors.
 */
typedef void(*message_reporter)(const char *type, const char *msg, const char *where);

void default_procedure_message_reporter(const char *type, const char *msg, const char *where);



/**
 * @brief Interface class for setting MYSQL_BIND values for input parameters.
 *
 * This interface class provides SimpleProcedure the means to set the input parameter
 * values after they have been bound to the MYSQL_STMT.  This allows the stream-based
 * SimpleProcedure interaction to work efficiently.
 */
class IParam_Setter
{
public:
   /**
    * @brief Callback class for setting parameter values.
    *
    * @param ds DataStack object through which the values are to be set.
    * @return True to continue to run procedure, False to abort.
    */
   virtual bool set_binds(DataStack<BindC> &ds) const = 0;
};

/**
 * @brief IParam_Setter derived class that provides parameter values
 * without callbacks.
 *
 * @tparam array_size This number should match the type and number of MYSQL_BIND
 * elements in the BindStack as well as the arguments in the Adhoc_Setter
 * constructor.
 *
 * This class is designed so it is possible, though not required, to be
 * constructed as a temporary variable.  That is, directly in the parameter
 * list of a function.
 *
 * @code
 Adhoc_Setter<3> setter(ai_long(1960), si_text("Chuck"), si_text("Jungmann"));
 * @endcode
 *
 * The example above will make a BindParam for a procedure that takes an integer
 * parameter followed by two strings.  This setter will be used to fill the
 * BindStack just before the procedure is executed.  This form must be used
 * as a temporary argument to SimpleProcedure.
 *
 * The following is another example, showing how Adhoc_Setter is used to run
 * a procedure.
 *
 * @snippet schema.cpp SimpleProcedure_Adhoc_Params
 *
 * @section Adhoc_Setter_Named_vs_Temporary Named vs Temporary Variables
 *
 * Temporary variables are only guaranteed to exist during the expression
 * in which they are declared.  Named object created using temporary objects
 * will have undefined results.  In particular (and as it pertains to our
 * discussion), yours truly has frequently made this mistake using Adhoc_Setter.
 *
 * Consider this expression, attempting to create an Adhoc_Setter to be use
 * in a SimpleProcedure:
 *
 * @code
 * // Please don't do this, it will cryptically fail:
 * auto &ahs = Adhoc_Setter(ai_ulong(&id), si_text(hash));
 *
 * SimpleProcedure::build(&mysql,
 *                        "CALL ssys_session_abandon(?,?)",
 *                        "Is32",
 *                        ahc,
 *                        user);
 * @endcode
 *
 * In this code, the `ai_ulong(&id)` and `si_text(hash)` will go out of
 * scope when the Adhoc_Setter constructor returns.  Consequently, when
 * SimpleProcedure::build() tries to use the setter object, two (or more)
 * problems may occur:
 *
 * 1. The memory that held the temporary objects may have been overwritten.
 * 1. The vtable of function pointers will be invalid.
 * 1. Other invalid object errors that I haven't yet experienced may occur.
 *
 * The problem with these errors is that generally fail catastrophically,
 * and the failure may occur far from the cause of the failure.
 *
 * Anticipating this error, I added a static flag and two static functions
 * referencing it to the IClass, which is the base class of the parameters
 * of the Adhoc_Setter.  Look at Adhoc_Setter::set_bind(), IClass::s_destruct_flag,
 * ~IClass(), IClass::clear_destruct_flag(), and IClass::read_destruct_flag() to
 * see how I anticipate the problem and provide a useful error message.
 */
template <int array_size>
class Adhoc_Setter : public IParam_Setter
{
protected:
   const IClass *m_array[array_size];
public:
   template<class... T>
   Adhoc_Setter(const T&... args)
      : m_array { &args... }
   {
      IClass::clear_destruct_flag();
   }

   int count(void) const { return array_size; }

   /** @brief Implement IParam_Setter::set_binds. */
   virtual bool set_binds(DataStack<BindC> &ds) const
   {
      if (ds.count()!=array_size)
         throw std::runtime_error("Adhoc_Setter size must match DataStack size.");

      if (IClass::read_destruct_flag())
      {
         throw std::runtime_error("Named Adhoc_Setter variable constructed"
                                  " with temporary constructor values. See"
                                  " Adhoc_Setter documentation.");
      }

      try
      {
         for (int i=0; i<array_size; ++i)
            ds.object(i).set_from(*m_array[i]);
      }
      catch(const Pure_Virtual_Exception &e)
      {
         ifputs("Missing pure virtual function: see the note "
               "in the Adhoc_Setter documentation.\n",
               stderr);
      }

      return true;
   }

   /** @brief Call t_make_typestr from ctyper.hpp,.cpp to make a typestring. */
   template <class F>
   void make_typestr(const F &f)
   {
      t_make_typestr(m_array, array_size, Generic_String_User<F>(f));
   }

   // Function defined below, after IResult_User is defined.
   void build(MYSQL *conn, const char *procname, IResult_User &ru) const;
};

/**
 * @brief Another IParam_Setter class for locally executed setter function.
 *
 * This class helps create an IResult_Setter object with access to closure variables.
 * The primary benefit of this class is to allow more control than with Adhoc_Setter.
 *
 * I added this template class for a situation where a string parameter is made by
 * joining several strings (@ref Schema::resolve_enum_set_references).  It makes more
 * sense to allocate to MYSQL_BIND buffer and write the several strings directly to
 * the buffer.  This class allows that to happen.
 */
template <class Func>
class TParam_Setter : public IParam_Setter
{
protected:
   const Func &m_func;
public:
   TParam_Setter(const Func &f) : m_func(f)           { }
   virtual bool set_binds(DataStack<BindC> &ds) const { return m_func(ds); }
};

/**
 * @brief For use with CGI/FASTCGI posted values.
 *
 * Parses name-value pairs, pairs separated by '&' and the name
 * is separated from the value by '='.
 *
 * To allow testing, this class takes a prepared IStreamer
 * object as its initializer.  The IStreamer object can be
 * constructed with stdin using StrmStreamer or, for testing,
 * with a string using StringStreamer.
 */
class Streamer_Setter : public IParam_Setter
{
protected:
   IStreamer &m_str;
public:
   Streamer_Setter(IStreamer &s) : m_str(s) { }
   virtual bool set_binds(DataStack<BindC> &ds) const;
};

/** @brief Uses stdin to get a value for each parameter. */
class CommandLine_Setter : public IParam_Setter
{
public:
   virtual bool set_binds(DataStack<BindC> &ds) const;
};



/**
 * @brief Interface class to be called by SimpleProcedure to process query results.
 *
 * Implementations of this class can be notified of progress processing a prepared
 * statement.
 *
 * One example, Result_As_XML, encloses a result's row elements with
 * pre_fetch_use_result() to write the open tag, and result_complete() to write
 * the close tag, and use_result_row() writes each row.  In this case, no data is
 * retained, but is written immediately as encountered.
 *
 * Template class Result_User_Row is useful when each row is consulted and discarded.
 * The Schema class uses Result_User_Row for executing many SchemaFW system stored
 * procedures.  See the source code for Schema::create_session_records(),
 * Schema::is_session_authorized(), and Schema::print_root_attributes_from_procedure()
 * (among several others) for other examples using Result_User_Row.
 
 *
 * Otherwise, for queries whose results must be saved, using pre_fetch_use_result()
 * to collect all the rows provides a single stack frame in which to allocate
 * many stack-resident items using alloca, then making those items available through
 * a callback interface. Template class Result_User_Pre_Fetch has a simple usage
 * example, and the source code of extensively-used
 * Result_User_Build_Schema::pre_fetch_use_result() is another pre-fetch-only usage.
 *
 * 
 */
class IResult_User
{
public:
   /**
    * @brief Means by which the related MYSQL_STMT can be accessed.
    *
    * The MYSQL_STMT object is needed to page out truncated results.
    * This function saves the MYSQL_STMT to the object so it is more
    * easily available to all stages of processing the result.
    */
   virtual void register_stmt(MYSQL_STMT *stmt) = 0;
   
   /**
    * @brief Procedure will call this function before fetching records.
    *
    * This function is called to provide the opportunity to use information
    * about the resultset before any records are retrieved.  It is the time
    * to print a schema, if desired, and to prepare the per-field
    * customizations desired for this result.
    *
    * The result_number is included in case of multiple resultsets in a
    * procedure.  It allows the programmer to correlate the current resultset
    * with another source for customization.
    */
   virtual void pre_fetch_use_result(int result_number, DataStack<BindC> &ds, SimpleProcedure &proc) = 0;

   /** @brief Procedure will repeatedly call this function until the rows are complete. */
   virtual void use_result_row(int result_number, DataStack<BindC> &ds) = 0;

   /** @brief Procedure will call this function after the final result record has been processed. */
   virtual void result_complete(int result_number, DataStack<BindC> &ds) = 0;

   /** @brief Provides means of putting an error message in the output. */
   virtual void report_error(const char *str) = 0;
};

/**
 * @brief Lambda-based class that implements IResult_User::use_result_row().
 *
 * Create a lambda function whose signature matches
 * IResult_User::use_result_row(int,DataStack<BindC>&) that will be called
 * for each row of each result.
 *
 * Use this class when it is not necessary to save the results for later use
 * (assuming that the user is maintaining the discipline of stack-only values).
 * This Result_User class works best for printing out results, after which the
 * values can be forgotten.
 *
 * For situations where the values must be saved, and an example of how that can
 * be accomplished, see Result_User_Pre_Fetch.
 *
 ~~~c++
 void demo_Result_User_Row(MYSQL *mysql, unsigned int id_person)
 {
    int count = 0;

    // Lambda function to implement virtual function IResult_User::use_result_row()
    auto f = [&count](int result_number, DataStack<BindC> &ds)
    {
       if (count==0)
          ifputs("Friend list: ", stdout);
       else
          ifputs(", ", stdout);

       BindC &bcFName = ds[0].object();
       BindC &bcLName = ds[1].object();

       if (!bcFName.is_null() && !bcLName.is_null())
       {
          bcFName.print(stdout);
          ifputc(' ', stdout);
          bcLName.print(stdout);
       }
       else if (!bcFName.is_null())
          bcFName.print(stdout);
       else if (!bcLName.is_null())
          bcLName.print(stdout);
       else
          ifputs("anonymous", stdout);
    };
    
    // Package the lambda function in a class to pass to SimpleProcedure::build:
    Result_User_Row<decltype(f)> user(f);

    SimpleProcedure::build(mysql,
                           "CALL App_Person_Friend_List(?)",
                           "I",
                           Adhoc_Setter<1>(au_ulong(&id_person)),
                           user);
 }
 ~~~
 */
template <class Func>
class Result_User_Row : public IResult_User
{
protected:
   const Func &m_func;
   MYSQL_STMT *m_stmt;
public:
   Result_User_Row(const Func &f)
      : m_func(f), m_stmt(nullptr)  { }
   virtual ~Result_User_Row()      { assert(m_stmt==nullptr); }

   virtual void register_stmt(MYSQL_STMT *stmt) { m_stmt = stmt; }
   EFFC_2(Result_User_Row<Func>)


   /** @brief No-op pure virtual function. */
   virtual void pre_fetch_use_result(int result_number,
                                     DataStack<BindC> &ds,
                                     SimpleProcedure &proc) { }
   /** @brief No-op pure virtual function. */
   virtual void result_complete(int result_number,
                                DataStack<BindC> &ds) { }
   
   virtual void use_result_row(int result_number,
                               DataStack<BindC> &ds)  { m_func(result_number,ds); }

   virtual void report_error(const char *str)         { ifputs(str, stderr); }
};

/**
 * @brief Lambda-based class that implements IResult_User::pre_fetch_use_result().
 *
 * Use this function when it is necessary to save values on the stack for a
 * function that it calls.
 *
 * For situations where the values need not be saved, and an example of how that is
 * done, see Result_User_Row.
 *
 * The following example fills an array with up to _limit_-1 stack-allocated strings.
 * The last element will be NULL to mark that last element without having to provide
 * a length parameter.
 *
 ~~~c++
 function demo(MYSQL *mysql, const char *seed)
 {
    int limit = 11;  // up to 10 elements, null-terminated array.
    size_t memlen = limit * sizeof(char*);
    const char **wordlist = static_cast<const char**>(alloca(memlen));
    memcpy(wordlist,0,memlen);

    // Lambda function to implement virtual function IResult_User::pre_fetch_use_result():
    auto cb_ruser = [this, &limit, &wordlist](int result_number,
                                              DataStack<BindC> &ds,
                                              SimpleProcedure &proc)
    {
       if (!confirm_result_number(result_number))
          return;

       size_t count = 0;

       while (proc.fetch(ds))
       {
          if (count<limit-1)
          {
             BindC& bc = ds.object(0);
             size_t slen = bc.data_length();
             char*  buff = static_cast<char*>(alloca(len+1));
          
             // MySQL does not add a terminating '\0', so we can't use strcpy.
             // Instead, use memcpy and append a '\0' for our string:
             memcpy(buff, bc.m_data, len);
             buff[len] = '\0';

             // Save the string and move on:
             wordlist[count] = buff;
             ++count;
          }
          else
            break;
       }

       // Get any remaining rows and results to finish and close the procedure.
       // Failure to do this may result in out-of-sync MySQL errors if the called
       // function needs to access MySQL.
       proc.conclude();

       procedure_that_uses_the_wordlist(wordlist);
    };

    // Package the lambda function in a class to pass to SimpleProcedure::build:
    Result_User_Pre_Fetch<decltype(cb_ruser)> ruser(cb_ruser);

    SimpleProcedure::build(mysql,
                           "CALL App_My_Word_List(?)",
                           "s32",
                           Adhoc_Setter<1>(si_text(seed)),
                           ruser);
 }
 ~~~
 */
template <class Func>
class Result_User_Pre_Fetch : public IResult_User
{
   const Func &m_func;
   MYSQL_STMT *m_stmt;
public:
   Result_User_Pre_Fetch(const Func &f)
      : m_func(f), m_stmt(nullptr)  { }
   virtual ~Result_User_Pre_Fetch() { assert(m_stmt==nullptr); }

   virtual void register_stmt(MYSQL_STMT *stmt) { m_stmt = stmt; }
   EFFC_2(Result_User_Pre_Fetch<Func>)


   virtual void pre_fetch_use_result(int result_number,
                                     DataStack<BindC> &ds,
                                     SimpleProcedure &proc)
                                                      { m_func(result_number,ds,proc); }
   /** @brief No-op pure virtual function. */
   virtual void result_complete(int result_number,
                                DataStack<BindC> &ds) { }
   
   /** @brief No-op pure virtual function. */
   virtual void use_result_row(int result_number,
                               DataStack<BindC> &ds)  { }

   virtual void report_error(const char *str)         { ifputs(str, stderr); }

};

/**
 * @brief Common base class to handle register_stmt for IResult_User implementations.
 *
 * This class asserts an error if the m_stmt has not been cleared before
 * going out of scope.  The procedure should assign the value once the
 * MYSQL_STMT has been initialized, and the procedure should set it to
 * nullptr when it (the procedure) goes out of scope.
 */
class Result_User_Base : public IResult_User
{
protected:
   FILE       *m_out;
   MYSQL_STMT *m_stmt;
public:
   Result_User_Base(FILE *out) : m_out(out), m_stmt(nullptr) { }
   virtual ~Result_User_Base()                               { assert(m_stmt==nullptr); }

   virtual void register_stmt(MYSQL_STMT *stmt) { m_stmt = stmt; }
   
   void print_string(BindC &bc, unsigned column);
   void set_output_stream(FILE* strm) { assert(m_out==nullptr); m_out=strm; }

   EFFC_2(Result_User_Base)
};

/** @brief Convenient class for outputing simple XML. */
class Result_As_XML : public Result_User_Base
{
public:
   Result_As_XML(FILE *out=stdout) : Result_User_Base(out) { }
   
   virtual void pre_fetch_use_result(int result_number, DataStack<BindC> &dsresult, SimpleProcedure &proc);
   virtual void use_result_row(int result_number, DataStack<BindC> &dsresult);
   virtual void result_complete(int result_number, DataStack<BindC> &dsresult);

   virtual void report_error(const char *str);
};



/**
 * @brief This class provides an easy way to execute prepared statements and use the
 *        results.
 *
 * See @ref TheSimpleProcedure for description and practical examples.
 * @sa IParam_Setter
 * @sa IResult_User
 * @sa StoredProc
 */
class SimpleProcedure
{
   const char       *m_query;
   DataStack<BindC> *m_dstack;
   MYSQL_BIND       *m_binds;

   MYSQL            *m_conn;
   MYSQL_STMT       *m_stmt;
   IResult_User     *m_user;

   int              m_result_number;
   bool             m_truncated_row;

public:
   /**
    * @brief Used to report errors and progress while processing a procedure.
    * @sa message_reporter
    * @sa default_procedure_message_reporter
    * @sa Schema::procedure_message_reporter
    */
   static message_reporter   s_message_reporter;

public:
   SimpleProcedure(const char *query, DataStack<BindC> *ds=nullptr, MYSQL_BIND *binds=nullptr)
      : m_query(query), m_dstack(ds), m_binds(binds),
        m_conn(nullptr), m_stmt(nullptr), m_user(nullptr),
        m_result_number(0), m_truncated_row(false)
   { }

   SimpleProcedure(const char *query, BindStack *bs)
      : m_query(query), m_dstack(bs), m_binds(bs?bs->binds():nullptr),
        m_conn(nullptr), m_stmt(nullptr), m_user(nullptr),
        m_result_number(0), m_truncated_row(false)
   { }

   EFFC_2(SimpleProcedure)
   ~SimpleProcedure()      { conclude(); }
   
   void run(MYSQL *conn, const IParam_Setter *ps, IResult_User *ru);
   /** @brief Inline override of run(MYSQL*, const IParam_Setter* IResult_User*). */
   inline void run(MYSQL *conn, const IParam_Setter &ps, IResult_User *ru=nullptr)
   {
      run(conn, &ps, ru);
   }
   /** @brief Inline override of run(MYSQL*, const IParam_Setter* IResult_User*). */
   inline void run(MYSQL *conn, IResult_User &ru)
   {
      run(conn, nullptr, &ru);
   };
      
   bool fetch(DataStack<BindC> &ds);
   void conclude(void);

   int number_of_results_processed(void) const { return m_result_number; }

   /** Like bleeding a brake line, get and ignore the remainder of the procedure. */
   inline void bleed_procedure(void) { conclude(); }

   static void t_build_query_string(const char *procname,
                                    int count_params,
                                    const IGeneric_String_Callback &gsb);

   template <class F>
   static void build_query_string(const char *procname,
                                  int count_params,
                                  const F &f)
   {
      Generic_String_User<F> gsu(f);
      t_build_query_string(procname, count_params, gsu);
   }

   /**
    * @name Build functions that construct their own BindStack
    * @{
    */
  static void build(MYSQL *conn,
                     const char *query,
                     MYSQL_RES *result,
                     const IParam_Setter *ps,
                     IResult_User  *ru);

   static void build(MYSQL *conn,
                     const char *query,
                     const char *typestr,
                     const IParam_Setter *ps,
                     IResult_User *ru);
   /**@}*/

   /**
    * @name
    *
    * There should be a collection of build functions that work without
    * an IBindInfo parameter to continue the process of elimintaing the
    * recursive build for DataStacks.
    * @{
    */
   
   /** @brief Build with no input parameters, use result. */
   inline static void build(MYSQL *conn, const char *query, IResult_User &ru)
   {
      SimpleProcedure sp(query);
      sp.run(conn, nullptr, &ru);
   }

   /** @brief Build with no input parameters, ignore result. */
   inline static void build(MYSQL *conn, const char *query)
   {
      SimpleProcedure sp(query);
      sp.run(conn, nullptr, nullptr);
   }

   /** @brief Build with parameters set on typestr-based BindStack, use result. */
   inline static void build(MYSQL *conn, const char *query,
                            const char *typestr,
                            const IParam_Setter &ps, IResult_User &ru)
   {
      build(conn, query, typestr, &ps, &ru);
   }

   /** @brief Build with parameters set on typestr-based BindStack, ignore result. */
   inline static void build(MYSQL *conn, const char *query,
                            const char *typestr,
                            const IParam_Setter &ps)
   {
      build(conn, query, typestr, &ps, nullptr);
   }


   /** @brief Build with parameters set on MYSQL_RES-based BindStack, use result. */
   inline static void build(MYSQL *conn, const char *query,
                            MYSQL_RES *res,
                            const IParam_Setter &ps, IResult_User &ru)
   {
      build(conn, query, res, &ps, &ru);
   }

   /** @brief Build with parameters set on MYSQL_REs-based BindStack, ignore result. */
   inline static void build(MYSQL *conn, const char *query,
                            MYSQL_RES *res,
                            const IParam_Setter &ps)
   {
      build(conn, query, res, &ps, nullptr);
   }
   /** @} */


   /**
    * @brief Make the BindC data available for detective work upon procedure failure.
    *
    * This function is primarily added for Result_User_Build_Schema::pre_fetch_use_result,
    * where I want to provide the requested procedure name in an error message if the
    * procedure is not found.
    */
   const DataStack<BindC>* get_bindstack(void) const { return m_dstack; }



// Internal functions:
protected:
   bool running(void) const { return m_stmt!=nullptr; }

   void bind_result_and_run(BindStack &bs);
   void loop_through_results(void);
   void process_current_result(void);

   void report_error(const char *message, const char *where) const;
   void report_error(int result, const char *where) const;
};


/**
 * @brief Build and run a named procedure given an Adhoc_Setter.
 *
 * This function uses t_make_typestr() and SimpleProcedure::build_query_string() to
 * construct the necessary parts to run SimpleProcedure::build().
 *
 * @todo I should probably refactor so that the SimpleProcedure build functions
 *       take a pointer to IResult_User instead of a reference.  It is possible
 *       to run a procedure that returns no results, so preparing a IResult_User
 *       is a waste of time.
 */
template <int array_size>
void Adhoc_Setter<array_size>::build(MYSQL *conn, const char *procname, IResult_User &ru) const
{
   const char *qstring = nullptr;

   // Second callback executes the procedure:
   auto fType = [&conn, &procname, &ru, &qstring, this](const char *tstring)
      {
         SimpleProcedure::build(conn, qstring, tstring, *this, ru);
      };

   // First callback saves the query string:
   auto fQuery = [this, &qstring, &fType](const char *result)
   {
      qstring = result;
      make_typestr(fType);
   };

   // Start the process
   SimpleProcedure::build_query_string(procname, array_size, fQuery);
}



#endif
