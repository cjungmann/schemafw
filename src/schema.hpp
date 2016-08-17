#ifndef SCHEMA_HPP
#define SCHEMA_HPP

#include "istdio.hpp"

#include "qstringer.hpp"
#include "storedproc.hpp"
#include "adbranch.hpp"
#include "specsreader.hpp"
#include "multipart_pull.hpp"
#include "mysql_loaddata.hpp"
#include <mysql.h>

inline void print_field_attribute(FILE *out, const ab_handle *handle)
{
   if (handle->has_value())
      print_xml_attribute(out, handle->tag(), handle->value());
}

void print_adhoc_attributes(FILE *out,
                            const ab_handle *handle,
                            const char **skip=nullptr);

int print_adhoc_elements(FILE *out,
                         const ab_handle *handle,
                         const char **skip=nullptr,
                         bool terminate = false);

void print_field_attributes(FILE *out,
                      const ab_handle *priority_one,
                      const ab_handle *priority_two = nullptr,
                      const ab_handle *priority_three = nullptr);


void make_session_string(char *buff, size_t copylen);
void set_session_seed(MYSQL *mysql, const char* seed);


void t_make_field_exclusion_list(const ab_handle *mode,
                                 const IGeneric_Callback<const char**> &user);

template <class Func>
void make_field_exclusion_list(const ab_handle *mode, const Func &cb)
{
   Generic_User<const char**, Func> user(cb);
   t_make_field_exclusion_list(mode, user);
}

/**
 * @brief Rebranded runtime_error to identify specific execptions that
 *        a Schema object is prepared to handle.
 */
class schema_exception : public std::runtime_error
{
public:
   schema_exception(const char *str) : std::runtime_error(str) { }
};


/**
 * @brief Sets parameters with query string values.  The typical use would be
 *        for GET forms.
 *
 ~~~c++
 void demo_qstringer(MYSQL *mysql, const char *procname)
 {
    QStringer_Setter setter(qstringer);
    Result_As_XML    user(stdout);

    auto f = [mysql, &setter, &user](StoredProc &sp)
    {
       SimpleProcedure::build(mysql,
                              sp.querystr(),
                              sp.bindstack(),
                              &setter,
                              &user);
    };
    Generic_User<StoredProc,decltype(f)> sp_user(f);

    StoredProc::build(mysql, procname, sp_user);
 }
 ~~~
 */
class QStringer_Setter : public IParam_Setter
{
protected:
   const QStringer &m_qstringer;
public:
   QStringer_Setter(const QStringer &qs) : m_qstringer(qs) { }
   virtual bool set_binds(DataStack<BindC> &ds) const;
};

/** Qbrief For cases with no POST or GET values and no procedure parameters.
 *
 * An alternative to using NoNothing_Setter would be to use the SimpleProcedure
 * constructor
 */
class DoNothing_Setter : public IParam_Setter
{
protected:
   bool m_return;
public:
   DoNothing_Setter(bool set_binds_return)
      : m_return(set_binds_return)                    { }
   virtual bool set_binds(DataStack<BindC> &ds) const { return m_return; }    
};

/** @brief We make a reinterpret_cast from DataStack<BindC> to BindStack.  This
 * assertion should guard against forgetting this relationship if BindStack
 * is modified so that it contains new data members.
 */
static_assert(sizeof(BindStack) == sizeof(DataStack<BindC>), "BindStack has grown: it no longer can be reinterpret_cast-ed");

/**
 * @brief Print a Schema from a BindStack and schema ab_handle
 */
class Schema_Printer
{
protected:
   static const char *s_field_ignore_list[];
   
   BindStack       &m_bindstack;
   SpecsReader     &m_specsreader;
   const ab_handle &m_mode;        /**< The current operating mode. */

   FILE        *m_out;
public:
   Schema_Printer(BindStack &bs,
                  SpecsReader &sr,
                  const ab_handle &mode,
                  FILE* out)
      : m_bindstack(bs), m_specsreader(sr), m_mode(mode), m_out(out)          { }
   
   Schema_Printer(DataStack<BindC> &ds,
                  SpecsReader &sr,
                  const ab_handle &mode,
                  FILE* out)
      : m_bindstack(reinterpret_cast<BindStack&>(ds)),
        m_specsreader(sr), m_mode(mode), m_out(out)                           { }
   
   void print(const ab_handle *schema, const char *default_name=nullptr);

protected:
   bool allow_field_attribute(const ab_handle *attribute)                   { return !string_in_list(attribute->tag(), s_field_ignore_list); }
   inline void print_field_attribute(const char *name, unsigned long value) { print_xml_attribute(m_out, name, value); }
   inline void print_field_attribute(const char *name, const char *value)   { print_xml_attribute(m_out, name, value); }
   inline void print_field_attribute(const ab_handle *attribute)            { print_xml_attribute(m_out, attribute->tag(), attribute->value()); }

   void find_and_print_schema_fields(const ab_handle *schema);
   void print_schema_fields(const ab_handle *fields);

   // void print_as_child(const ab_handle *branch) const;
};


/**
 * @brief Pointer to function that dictates if more requests should be processed.
 *
 * For now, this returns BOOL as the more simple and clear result, but
 * it had previously been an int return value to pass MySQL errors to
 * the loop.  Keep an open mind about whether that's important, but I
 * don't think it's necessary at this point.
 */
typedef bool(*loop_sentry)(void);

/**
 * @name Global sentry functions.
 *
 * These functions are used by Schema::wait_for_requests to determine
 * whether or not to wait for additional requests.
 * @{
 */
bool cli_sentry(void);
bool cgi_sentry(void);
/**@}*/


/** @brief Keeping this typedef around: it might be useful. */
typedef void(*error_notifier)(const char *msg, const char *type, FILE* out);

/**
 * @name Two functions that match the error_notifier signature.
 *
 * Once per main() call, Decide how your program will report errors,
 * setting static function pointer Schema::s_notifier to your choice.
o * @{
 */
void notify_as_xml(const char *msg, const char *type, FILE *out);
void notify_stderr(const char *msg, const char *type, FILE *out);
/**@}*/

/**
 * @brief Ultimate director of web interactions.
 *
 * This is the class where everything comes together.  A new Schema object is
 * constructed with every http request, and the Schema object determines what
 * must be done and how to do it.
 *
 * After the Schema has been created, the process continues with
 * Schema::collect_resources(), which begins a chain of function calls that
 * eventually arrives at install_response_mode.  During install_response_mode,
 * all the instructions are available, the Query String, Cookies, and the
 * request mode from the Specs file.  The process branches from here,
 * depending on the request type:
 *
 * -# __session_submit__ Will run the standard session initialization, the
 *    result of which will determine if we continue to the home page.
 *
 * -# __session_logout__ will run the standard session abandon, deleting the
 *    session table entry and clearing the session cookies.
 *
 * -# An unspecified type will continue with POST or GET variables to fill
 *    parameters of the named procedure.
 *
 * -# __import__ runs a LOAD DATA LOCAL query to import a file into the
 *    target table.  The _confirm_procedure_ instruction identifies
 *    the procedure used to display the uploaded file to give the user
 *    a change to accept or abandon the import.  The _confirm_procedure_
 *    should be a procedure that takes only one parameter, the session id.
 *
 *    The __import__ type is unique in that it doesn't look for parameters
 *    in the POST or GET, it runs different code to import the submitted
 *    file.
 */
class Schema : public IParam_Setter
{
public:
   struct pipe_cl_struct;

   // public member functions:
   Schema(SpecsReader *sr, FILE *out);
   Schema(SpecsReader *sr, IParam_Setter *setter, FILE *out)
      : Schema(sr, out) { m_setter=setter; }
   Schema(SpecsReader *sr, Multipart_Pull *mpp, FILE *out)
      : Schema(sr, out) { m_puller = mpp; }

   static void set_header_out(FILE *out) { s_header_out = out; }

   
   static int get_fake_stdin_from_command_args(int argc, char **argv,
                                               pipe_cl_struct &pcs);


   static bool confirm_mysql_connection(void);
   static int wait_for_requests(loop_sentry sentry, FILE *out);
   static int run_single_request(SpecsReader &sr, FILE *out, char type);

   static void report_error(FILE *out, const char *str);

   // Static error-reporting functions to enfore consistent composition.
   static void print_message_as_xml(FILE *out,
                                    const char *type,
                                    const char *message,
                                    const char *detail=nullptr);

   inline static void print_error_as_xml(FILE *out,
                                         const char *message,
                                         const char *where=nullptr)
   {
      print_message_as_xml(out, "fatal", message, where);
   }
   static void pre_doc_failure(const char *message, const char *where);

   /**
    * @brief Function template version of pre_doc_failure.  Directly use message buffers.
    *
    * @param fmessage Callback (lambda)? function void(char *buffer, int bufflen)
    *                 into which the error message can be copied.
    * @param fdetail Callback (lambda)? function void(char *buffer, int bufflen)
    *                into which the detail string can be copied. 
    */
   template <class FMessage, class FDetail>
   static void pre_doc_failure(FILE *out, const FMessage *fmessage, const FDetail *fdetail=nullptr)
   {
      (*fmessage)(s_failure_message, sizeof(s_failure_message));
      if (fdetail)
         (*fdetail)(s_failure_detail, sizeof(s_failure_detail));
   }
   



protected:
   static MYSQL          s_mysql;
   static error_notifier s_notifier;
   static bool           s_is_web_request; /**< Set by is_web_request to detect command line processing. */

   static const uint32_t s_invalid_session;
   static const char     *s_default_import_confirm_proc;
   const static char     *s_ok_status;
   const static char     *s_jump_status;

   static char           s_failure_message[200];
   static char           s_failure_detail[80];

   static FILE*          s_header_out;

   inline static void    print_Status_200(void) { ifputs(s_ok_status, s_header_out); }
   inline static void    print_Status_303(void) { ifputs(s_jump_status, s_header_out); }
public:
   enum Debug_Action
   {
      DEBUG_ACTION_IGNORE = 0,
      DEBUG_ACTION_PRINT_MODE,
      DEBUG_ACTION_PRINT_RESPONSE_MODES,
      DEBUG_ACTION_PRINT_ALL_MODES,
      DEBUG_ACTION_PRINT_MODE_TYPES,
      DEBUG_ACTION_LINT
   };
   /** <b>d</b>ebug-<b>a</b>ction pair */
   struct dapair { const char *name; Debug_Action action; };
   
   const static dapair *s_debug_action_types;
   static Debug_Action s_debug_action;


   static bool set_debug_action_mode(const char *action);
   

   enum MODE_ACTION
   {
      MACTION_NULL = 0, /**< Initial value of Schema::m_mode_action to indicate
                         *   that the type value has not been retrieved.  This is
                         *   to help ensure that get_mode_type() is called before
                         *   get_session_status().
                         */
      MACTION_NONE,
      MACTION_INFO,
      MACTION_TABLE,
      MACTION_DISPLAY,
      MACTION_LOOKUP,
      MACTION_DELETE,
      MACTION_FORM_EDIT,
      MACTION_FORM_NEW,
      MACTION_FORM_SUBMIT,
      MACTION_FORM_TRY,
      MACTION_FORM_RESULT,
      MACTION_FORM_VIEW,
      MACTION_ABANDON_SESSION,
      MACTION_IMPORT
   };

   struct struct_mode_action
   {
      const char  *str;
      MODE_ACTION value;
   };

   static const struct_mode_action map_mode_actions[];
   static const struct_mode_action *end_map_mode_actions;

   static void print_mode_types(FILE *f);
   static void print_lint(FILE *f, const SpecsReader *reader);
   static MODE_ACTION get_mode_type(const char *str);

   /**
    * @brief Quick test if form type requiring parameters.  Does not include form-view.
    *
    * A _form-view_ should probably have another name to distiguish it from other forms,
    * except that it displays data in a form-like layout.  However, it is different
    * because the field data is not changeable on the _form-view_ page, there is no
    * _Submit_ button.  A _form-view_ form is expected to offer editing by jumping
    * to an editing page for the selected field.
    */
   static bool is_form_type(MODE_ACTION a)
   {
      return a>=MACTION_FORM_EDIT && a<=MACTION_FORM_RESULT;
   }

   const ab_handle *get_first_result(void) const;

   /**
    * @defgroup Schema_PerRequest Schema Per-request Variables
    * @brief Schema data members that are cleared and re-acquired with each new request.
    *
    * With the exception of two flags, these are pointers to objects that are
    * responsible for processing the current request.
    *
    * The objects, if not null, will have been created in functions higher on the
    * stack and handed down through function parameters.  As such, they remain
    * valid for the current request, and any destructors will be called as the
    * stack unwinds.
    *
    * The flags serve to indicate that the item, after whose name the flag is named,
    * has been sought.  This information is useful to the recursive collect_resources
    * function that won't otherwise know that it is futile to continue trying to
    * fill a null pointer value.
    * @{
    */



   FILE             *m_out;           /**< Stream to which output is directed. */
   SpecsReader      *m_specsreader;   /**< Required for building a Schema. */
   IParam_Setter    *m_setter;        /**< For POST, GET or command line data, this object sets the
                                       * parameters for the stored procedure associated with the request.
                                       *
                                       * The value will be NULL if the request is a multipart/form-data,
                                       * in which case we'll be importing a table.
                                       */
   
   Multipart_Pull  *m_puller;         /**< For a multipart/form-data POST request.
                                       *   This object will read field information
                                       *   and translate imported spreadsheets in
                                       *   CSV for importing into MySQL.
                                       */

   bool m_reqd_CStringer;    /**< Flag to indicate that Cookies have been requested. */
   bool m_reqd_QStringer;    /**< Flag to indicate that the QueryString has been requested. */
   bool m_reqd_ResponseMode; /**< Flag to indicate that the response mode has been requested. */

   bool m_database_confirmed; /**< Flag indicating that set_requested_database()
                               *   been called.  This is particuarly important
                               *   when confirming a session.
                               */
   
   const BaseStringer *m_cstringer;   /**< May be null if there is no cookie string. */
   const QStringer    *m_qstringer;   /**< May be null if there is no query string. */
   const ab_handle    *m_mode;        /**< The current operating mode. */
   Advisor            *m_advisor;     /**< Will be non-null because a request can't run without an advisor. */
   const char         *m_type_value;  /**< The type value of the mode. */
   MODE_ACTION        m_mode_action;  /**< Action to be taken, set from m_type_value. */
   const char         *m_meta_jump;   /**< Value to be set in install_response_mode()
                                       *   that will be added to the document element
                                       *   as a meta-jump attribute (if not null).
                                       */
   const char         *m_prepage_message; /**< If not NULL, this string should be
                                             displayed in an alert box before allowing
                                             the user to use the page.  Intended as a
                                             way to notify a user of an expired session.
                                          */
   uint32_t           m_session_id;       /**< Indicates active session if
                                             m_session_id!=s_invalid_session.
                                          */
    /**@}*/
   
protected:

   static void* pthread_fake_stdin_start_routine(void *data);

   static void write_error_element(FILE *out,
                                   const char *type,
                                   const char *msg,
                                   const char *detail=nullptr);
   
   static void clear_failure_strings(void);
   static void set_failure_message(const char *msg, const char *detail=nullptr);

   /**
    * @brief This function will be called if a fatal error interrupted Schema setup.
    *
    * For errors that occur before calling finish_header_add_xml_pi() should
    * be recorded using set_failure_message().  When install_response_mode() returns
    * without writing any records, finish_header_add_xml_pi() will be run, and
    * then this function will be called to write the single document element.
    */
   inline static void write_failure_message(FILE *out)
   {
      print_message_as_xml(out,
                           "fatal",
                           s_failure_message,
                           s_failure_detail);
   }
   /**
    * @brief Call this function to immediately output an error.
    *
    * This function should be used after finish_header_add_xml_pi() has been called
    * and probably even after the start of reading parameters from the HTTP request.
    * The element that is written may thus be in another element, so consider where
    * the output is at before calling this function.
    */
   inline static void write_error_message(FILE *out,
                                          const char *msg,
                                          const char* where=nullptr)
   {
      print_message_as_xml(out, "error", msg, where);
   }

   // The following functions are called in roughly
   // the order they are defined below:

   static bool start_mysql(void);

   /**
    * @defgroup Begin_Request_Processing Begin Request Processing
    *
    * These functions are called at each request.  Either schema_with_post
    * or schema_without_post is called according to is_post_request
    * based on the request method.  Both functions end up calling
    * collect_resources to continue processing.
    * @{
    */

   static void change_to_path_dir(const char *path);

   static void log_new_request(void);
   static void clear_for_new_request(void);
   static bool is_web_request(void)           { return nullptr!=getenv("REQUEST_METHOD"); }
   static bool is_post_request(void);
   static void schema_with_post(SpecsReader &sr, FILE *out);
   static void schema_without_post(SpecsReader &sr, FILE *out);
   static void schema_with_cl(FILE *out);
   /**@}*/

   void collect_resources(void);

   void clear_session_cookies(void);
   bool create_session_records(void);
   void abandon_session_records(uint32_t id, const char *hash);
   void abandon_session_records(void);
   bool confirm_session(uint32_t id, const char *hash) const;

   inline bool session_is_valid(void) const { return m_session_id != s_invalid_session; }
   inline void invalidate_session(void)     { m_session_id = s_invalid_session; }
   bool is_session_authorized(uint32_t id) const;

   /**
    * @brief Types of significant session status.
    *
    * The order is important: both SSTAT_RUNNING and SSTAT_AUTHORIZED are
    * indicators of an active session.  I want to detect an active session
    * with a > (greater-than operator), so any active session status must
    * follow the last inactive session status enumerator.
    *
    * @sa Schema::install_response_mode
    */
   enum SESSION_STATUS
   {
      SSTAT_NONE = 0,   /**< No session cookies */
      SSTAT_EXPIRED,    /**< Session cookies __do not__ match an active session. */
      SSTAT_RUNNING,    /**< Session cookies match an active session. */
      SSTAT_AUTHORIZED  /**< Session running and authorized. */
   };

   inline static bool session_implied(SESSION_STATUS s) { return s>SSTAT_NONE; }
   inline static bool session_running(SESSION_STATUS s) { return s>SSTAT_EXPIRED; }
   inline static bool session_authorized(SESSION_STATUS s) { return s>SSTAT_AUTHORIZED; }

   /**
    * @brief Possible session requirements.
    *
    * An increasing value indicates an increasing restriction.
    *
    * @sa Schema::install_response_mode
    */
   enum SESSION_TYPE
   {
      STYPE_NONE = 0,     /**< Ignore session status (don't start, don't end) */
      STYPE_ESTABLISH,    /**< For Identity session type, allow authorization form to run. */
      STYPE_SIMPLE,       /**< Create session if none in force. Continue with page */
      STYPE_IDENTITY      /**< Don't run anything unless authorized. */
   };
   SESSION_TYPE get_session_type(void) const;
   
   struct struct_session_type
   {
      const char    *str;
      SESSION_TYPE  value;
   };

   static const struct_session_type map_session_types[];

   inline static bool session_required(SESSION_TYPE s) { return s > STYPE_NONE; }


   void log_missing_mode(const char *mode_name) const;
   void set_instance_mode_values(const ab_handle *mode_handle);
   void install_response_mode(const char *mode_name);


   // Called by install_response_mode():
   SESSION_STATUS get_session_status(SESSION_TYPE stype, bool abandon_session);
   void abandon_session(const char *jump_destination, bool clear_cookies);

   

   const char *value_from_mode(const char *name) const;
   const char *value_from_mode_or_global(const char *name) const;

   const char *get_jump_destination(SESSION_TYPE st) const;

   void clear_quarantine_table(const char *tablename);
   bool import_table(const char* tablename);
   bool process_import_form(void);

   /**
    * @defgroup Opening_Specs_And_Mode Specs File and Request Mode Setup
    *
    * These functions are used by install_response_mode to determine which
    * specs file and which mode to open based on the query string or
    * default values if there is no query string.
    * @{
    */
   template <class Func>
   void get_specs_instructions(const Func &cb);
   
   template <class Func>
   void get_session_cookies(const Func &cb) const;

   long get_first_non_global_mode(void);
   long get_request_mode_position(const char *mode_name, bool keep_looking=false);
   /**@}*/
   /**
    * @defgroup Schema_Header_Functions Schema Header Functions
    *
    * This group of functions and the single static data member
    * write out header information and track the header status.
    *
    * We can get away with a static s_headers_done member because
    * only one Schema element will be active at a time under the
    * FASTCGI method.  If duplexing ever comes to pass (not likely),
    * we'll have to address this differently
    * @{
    */
   static bool s_headers_done;
   static bool s_sfw_xhrequest;

   static inline bool get_sfw_xhrequest(void) { return s_sfw_xhrequest=((getenv("HTTP_SFW_XHREQUEST")) ? true : false); }
   void set_forbidden_header(void);
   void set_cookie(const char *name, const BindC *value=nullptr, int seconds_to_expire=0) const;
   void write_refresh_header(int64_t seconds, const char *url) const;
   void write_location_header(const char *url) const;
   void refresh_to_mode_jump_destination(int seconds=0);
   bool refresh_to_not_authorized_destination(int seconds=0);
   bool refresh_to_no_session_destination(int seconds=0);

   /** @brief Write content type to headers. */
   inline static void print_ContentType() { ifputs("Content-Type: text/xml\n", s_header_out); }
   /** @brief Use this function to end the headers to set the s_headers_done flag. */
   inline static void headers_conclude(FILE *out)   { ifputc('\n', out); s_headers_done = true; }
   /** @brief Alias for headers_conclude to match write_refresh_header() form. */
   inline void write_headers_end(void) const  { headers_conclude(s_header_out); }
   /**@}*/
   
   
   void set_requested_database(void);
   void add_stylesheet_pi(void);
   void finish_header_add_xml_pi(void);

   class CB
   {
   protected:
      Schema &m_s;
   public:
      CB(Schema *s) : m_s(*s) { }
      void operator()(void)   { m_s.print_import_confirm(); }
   };

   void start_special_procedure(void);
   void start_document(void);
   void process_root_branch(const ab_handle *mode_root,
                            const ab_handle *global_root,
                            bool import_confirm = false);
   
   void print_import_confirm(void);
   
   void print_root_attributes_from_procedure(FILE *out, const char *procname);
   /**
    * @brief Print the document element using values from the mode/root or $root.
    */
   void print_document_element(const char *tagname,
                               const ab_handle *mode_root,
                               const ab_handle *global_root);

   /**
    * @defgroup StoredProc_Section Process Desginated Procedure with StoredProc
    *
    * This group of functions process the stored procedure named in the specs file.
    * Once the StoredProc object is created, process_info_procedure will print
    * a form schema if needed, then run_procedure will create a SimpleProcedure
    * object and pass control off to printing one or more results.
    * @{
    */
   typedef void (Schema::*proc_runner)(StoredProc &infoproc);
   void process_info_procedure(StoredProc &infoproc);
   void run_procedure(StoredProc &infoproc);
   
   void open_info_procedure(proc_runner pr = &Schema::process_info_procedure);
   /** @} */

   void print_schema_from_procedure_name(const char *name);
   void resolve_enum_set_references(BindStack *bs);
   void print_schema_from_bindstack(BindStack *bs);

   void set_session_string_if_necessary(DataStack<BindC> &ds) const;

   /** @brief Implementation of IParam_Setter pure virtual function. */
   virtual bool set_binds(DataStack<BindC>&ds) const;

   void modify_schema_binds(BindStack *bs, const ab_handle *schema);

   /**
    * @brief This template function facilitates the call to SpecsReader::get_specs_handle.
    *
    * This function uses a lambda function to create the Generic_User class needed to
    * run SpecsReader::get_specs_handle.
    */
   template <class Func>
   void seek_specs_handle(const char *tag,
                          Func &f,
                          const ab_handle *mode_first,
                          const ab_handle *mode_second = nullptr,
                          const ab_handle *mode_third = nullptr)
   {
      Generic_User_Const_Pointer<ab_handle, Func> user(f);
      m_specsreader->seek_specs_handle(tag, user, mode_first, mode_second, mode_third);
   }

   void report_message(const char *msg);
   void report_error(const char *where);
   void report_missing_error(const char *what, const char *missing);

   class ProcMessageException : public std::exception
   {
   public:
      ProcMessageException(const char *t, const char *m, const char *w)
         { set(t,m,w); }

      ProcMessageException(const ProcMessageException &e)            { }
      ProcMessageException& operator=(const ProcMessageException &e) { return *this; }

      const char *type(void) const  { return s_type; }
      const char *msg(void) const   { return s_msg; }
      const char *where(void) const { return s_where; }
      
   private:
      static char s_type[32];
      static char s_msg[256];
      static char s_where[64];
      
      static void set(char *dest, const char *source, int limit);
      inline void set_type(const char *t)  { set(s_type, t, sizeof(s_type)); }
      inline void set_msg(const char *m)   { set(s_msg, m, sizeof(s_msg)); }
      inline void set_where(const char *w) { set(s_where, w, sizeof(s_where)); }

      inline void set(const char *type, const char *msg, const char *where=nullptr)
      {
         set_type(type);
         set_msg(msg);
         set_where(where);
      }
   };  // end of Schema::ProcMessageException


   /**
    * @defgroup EnumAndSetRefFuncs Enum and Set List Collection Functions
    *
    * This group of functions collects a list of Table.Column references from
    * a response mode.  The end result of these functions is a string to use
    * as a parameter to pass to stored procedure ssys_get_column_dtds.
    *
    * count_enum_set_references() and get_enum_set_references() work together
    * to build an array of pointers to char*.
    *
    * get_string_length_enum_set_list() and set_enum_set_list_string() work
    * together with a null-terminated array constructed with get_enum_set_references()
    * to create a comma-separated list of Table:Column values to pass to
    * the stored procedure ssys_get_column_dtds.
    *
    * The following example illustrates how this group of functions works together.
    * See the source code of Schema::resolve_enum_set_references() for an example
    * of how these functions serve in their designed role.
    ~~~c++
    void demo_enum_funcs(const ab_handle *responseMode)
    {
       int rcount = count_enum_set_references(responseMode);
       if (rcount)
       {
          size_t rlen = (rcount+1) * sizeof(ab_handle*);
          const ab_handle** refs = static_cast<const char**>(alloca(rlen));
          memset(refs,0,rlen);

          get_enum_set_references(responseMode, refs, rcount);

          // In this example, allocate extra byte for terminating '\0'
          // in order to display with printf:
          size_t slen = 1 + get_string_length_enum_set_list(refs);
          char *buff = static_cast<char*>(alloca(slen));
          set_enum_set_list_string(buff, slen, refs);

          // Show the result:
          printf("The finished string: \"%s\"\n", buff);
       }
    }
    ~~~
    * @{
    */

   /**
    * @brief Convenient but not robust struct for saving and searching tags.
    *
    * This is a struct because it does not aspire to class-like utility: proper
    * operation of this class depends on elsewhere allocating the memory on which
    * it works, and especially that one extra empty element follows the content-
    * containing elements.  The empty element is a flag that a search has reached
    * the end of the list.
    */
   struct refnode
   {
      const ab_handle *field;
      const ab_handle *ref;

      inline void set(const ab_handle *hfield, const ab_handle *href)
      { field=hfield; ref=href; }
      
      inline operator bool(void) const               { return field && ref; }
      inline bool is_tag(const char *s) const        { return field->is_tag(s); }
      inline bool is_tag(const base_handle *h) const { return field->is_tag(h->str()); }

      /** This is the useful function: finds refnode whose ref->value()==tag */
      const refnode *find_tablefield(const char *tag) const
      {
         const refnode *ptr = this;
         while (ptr)
         {
            if (ptr->ref->is_value(tag));
               return ptr;
            ++ptr;
         }
         return nullptr;
      }

      /**
       * @brief Get string value from @p bc to use for find_tablefield(const char*).
       *
       * MySQL doesn't add a terminating '\0' to a string value, so to do the
       * comparison, we need make a copy of the string.  Technically, we could use
       * strncmp with the length, but that would require adding functions using
       * strncmp to a string of objects.
       */
      const refnode* find_tablefield(const BindC& bc) const
      {
         size_t len = bc.strlength();
         char *buff = static_cast<char*>(alloca(len+1));
         memcpy(buff, bc.m_data, len);
         buff[len] = '\0';

         return find_tablefield(buff);
      }

      const refnode *find(const char *tag) const
      {
         const refnode *ptr = this;
         while (ptr)
         {
            if (ptr->is_tag(tag))
               return ptr;
            ++ptr;
         }
         return nullptr;
      }

      const refnode *find(const base_handle *h) const { return find(h->str()); }
         
   };

   static int count_enum_set_references(const ab_handle *mode);
   static void get_enum_set_references(const ab_handle *mode,
                                       refnode *refs,
                                       int refcount);
   static size_t get_string_length_enum_set_list(refnode* refs);
   static void set_enum_set_list_string(char *buff, size_t bufflen, refnode* refs);
   /** @} */

public:

   static void procedure_message_reporter(const char *type,
                                          const char *msg,
                                          const char *where);

   static void procedure_message_thrower(const char *type,
                                         const char *msg,
                                         const char *where);

   /** @brief Data payload for pthread_fake_stdin_start_routine(). */
   struct pipe_cl_struct
   {
      int  argc;     /**< From argc in main(int argc, char **argc) */
      char **argv;   /**< From argv in main(int argc, char **argc) */
      int  h_write;  /**< Write-end of a pipe through which data will be transfered. */

      pipe_cl_struct(int pc, char **pv, int fh)
         : argc(pc), argv(pv), h_write(fh) { }
      pipe_cl_struct() : argc(0), argv(nullptr), h_write(-1) { }
   };
};   // end of class Schema

class Result_As_SchemaDoc : public Result_User_Base
{
protected:
   SpecsReader               &m_specsreader;
   const ab_handle           *m_mode;
   const Schema::MODE_ACTION m_mode_action;
   const char                *m_group_name; /**< If specified, this value becomes the
                                             *   name of the element that contains the
                                             *   rows of a query result.
                                             */
   const char                *m_row_name;
   bool                      m_schema_prints_early;

   /**
    * @brief Indicates if a schema must be printed even if not explicitly requested.
    *
    * Both a table and a display mode type require a schema to display the results,
    * so this function identifies if we're using one of these mode types.
    */
   inline bool must_print_schema(void) const
   {
      return m_mode_action==Schema::MACTION_DISPLAY
         || m_mode_action==Schema::MACTION_TABLE;
   }

   inline bool schema_prints_early(void) const { return m_schema_prints_early; }

   
public:
   Result_As_SchemaDoc(SpecsReader &sr,
                       const ab_handle *mode,
                       Schema::MODE_ACTION action,
                       FILE *out)
      : Result_User_Base(out), m_specsreader(sr),
        m_mode(mode), m_mode_action(action),
        m_group_name(nullptr), m_row_name(nullptr),
        m_schema_prints_early(m_mode->seek("schema-proc")!=nullptr)
   { }
   
   EFFC_3(Result_As_SchemaDoc)

   virtual void pre_fetch_use_result(int result_number,
                                     DataStack<BindC> &dsresult,
                                     SimpleProcedure &proc);
   virtual void use_result_row(int result_number,
                               DataStack<BindC> &dsresult);
   virtual void result_complete(int result_number,
                                DataStack<BindC> &dsresult);
   virtual void report_error(const char *str) { Schema::report_error(m_out, str); }
};

class Result_As_Headers : public Result_User_Base
{
protected:
   SpecsReader     &m_specsreader;
   const ab_handle *m_mode;

public:
   Result_As_Headers(SpecsReader &sr, const ab_handle *mode, FILE *out)
      : Result_User_Base(out), m_specsreader(sr), m_mode(mode)  { }

   EFFC_3(Result_As_Headers)
   
   virtual void use_result_row(int result_number,
                               DataStack<BindC> &dsresult);
   
   virtual void report_error(const char *str) { Schema::report_error(m_out, "Result_As_Headers"); }
};


#endif
