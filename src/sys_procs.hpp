/**
 * @file
 * @brief Documents the contents sys_procs.sql.
 *
 * This file exists only to provide Doxygen with code that
 * allows generation of documentation of the MySQL code.  It is
 * not included in the Schema build.  If it was included, it
 * would fail to compile due to the undefined MySQL datatypes.
 */

/**
 * @page SchemaFW_MySQL_Overview SchemaFW MySQL Overview
 *
 * See @ref SchemaFW_System_MySQL_Procedures
 *
 * This code base includes a few procedures that are used
 * to run general operations in SchemaFW. These are
 *
 * - ssys_get_procedure_params()
 * - ssys_get_procedures()
 * - ssys_confirm_table()
 * - ssys_clear_for_request()
 * - ssys_session_confirm()
 *
 * The remainder of the code relates to managing sessions.
 *
 * The SSYS_SESSION table handles the basic session bookkeeping.  New
 * sessions are recorded in this table, and the expires field tracks
 * the session lifetime.
 *
 * The framework depends on severl application-replacable procedures to
 * simplify session management.  These are:
 *
 * - __App_Request_Cleanup__ is called when a new Schema object is created
 *   for a new request.  As I write this, I am thinking about MySQL session
 *   variables (names preceded with _@_ like `@confirmed_session_id`) that,
 *   if leftover from a previous request, might reveal private information
 *   from another user.  This procedure will be called by the SchemaFW
 *   procedure ssys_clear_for_request.
 *
 * - __App_Session_Start__ is called by ssys_session_create.  App_Session_Start
 *   should initialize application session values.  Typically, this means to
 *   create or update a app-specific session table.  It's a good idea to clear
 *   fields to protect a previous uses data in case App_Session_Abandon
 *   neglected to do this.
 * 
 * - __App_Session_Restore__ is called by ssys_session_confirm is the session
 *   is confirmed.  In the typical case, it wil read from an app-specific table
 *   and set connection session variables according to its contents.
 
 * - __App_Session_Abandon__ is called by the trigger procedure
 *   ssys_sync_session_update when a SSYS_SESSION record is updated and
 *   the available field has changed from 0 to 1 (unavailable to available).
 *   It should typically clear data from the app-specific table to prevent
 *   private data from being visible by a user who uses a recycled session id.
 *
 * @note The App_Request_Cleanup and App_Session_xxxxx procedures are
 *       defined as do-nothing procedures by sys_procs.sql to ensure their
 *       availablility.  In most cases, applications will define replacements
 *       for these functions.  Each procedure should be preceeded by a
 *       `DROP PROCEDURE IF EXISTS xxx` with the _xxx_ replaced with the
 *       actual procedure name.  __IMPORTANT__ do not replace these
 *       procedure definitions in _sys_procs.sql_ or they will be lost
 *       if a software update replaces the _sys_procs.sql_ file.  Instead,
 *       put the redefinitions in an application-specific MySQL script file.
 *
 * Sessions are restored upon matching cookie values for SessionId and
 * SessionHash from a request header. If the session is confirmed and
 * has not expired, the session variable @session_confirmed_id will
 * contain an integer value that can and should be used to save or
 * retrieve additional session information from related tables.
 *
 * ## MySQL Session Variables
 *
 * MySQL session variable, initialized with a '@'-prefixed name, are
 * valid from when it's first named until the connection is terminated.
 * The session variable is a global variable while in the connection,
 * available to all procedures run while the connection is open.
 *
 * For our purposes, assuming that we will eventually be running in
 * FASTCGI and thus reusing the connection, the session variables
 * should be considered to be global and persistent.  Because of
 * this, the programmer must clear \@session_confirmed and
 * \@session_seed_string before the end of the response.
 *
 * The system-defined session variables are:
 * - __\@session_confirmed_id__\n
 *   Value of current session, NULL if session not found or has expired.
 * - \@**session_seed_string**\n
 *   32-character hash value to use when initiating a new session.  This
 *   value is set by calling ssys_seed_session_string from SchemaFW before
 *   calling ssys_session_create.
 */

/**
 * @defgroup SchemaFW_System_MySQL_Procedures SchemaFW System MySQL Procedures
 * @{
 */

/**
 * @brief SchemaFW-generated string variable \@dropped_salt for salting passwords.
 *
 * MySQL doesn't have a method for generating a reliable random number, so the
 * reponsibility for this falls to SchemaFW, which already makes a similar string
 * for a session hash.
 *
 * This variable is set when a SRM response mode includes a _drop-salt_
 * instruction.  The default name and length are "dropped-salt" and 32, which
 * will be available in MySQL as `@dropped-salt`, but these settings can
 * be changed if necessary.  The salt length is limited to 255 characters or
 * less.
 *
 ~~~srm
 create_login
    type        : form-new
    schema-proc : App_Create_Login
    drop-salt
       name   : table-salt  # optional value, default=dropped-salt
       length : 48          # optional value, default=32
 ~~~
 *
 */
const char *dropped_salt;

/**
 * @brief Session variable \@session_string_seed for preparing to create a new session.
 *
 * @note use as \@session_string_seed
 *
 * This session variable is set by ssys_seed_session_string and must
 * be set before calling ssys_session_create.
 */
int session_string_seed;

/**
 * @brief Session variable \@session_confirmed_id for confirming and
 * identifying the current session.
 *
 * @note use as \@session_confirmed_id
 *
 * This session variable should be set as soon as possible to make it
 * available for app-specific procedures to get or set other session
 * information.
 */
int session_confirmed_id;

/**
 * @brief System table for tracking sessions
 */
class SSYS_SESSION
{
public:
   unsigned  id;      /**< session identifier */
   char[32]  hash;    /**< random string to authorize session access */
   DATETIME  expires; /**< time the session expires */
   bool    available; /**< indexed value to help find abandoned session records */
};

/**
 * @brief First result of @ref ssys_get_procedure_params.
 *
 * Informs caller of the number of records in the second result so
 * an appropriatly-sized array can be created to save the parameters'
 * characteristics.
 *
 * 2016-03-30: Added a means to report a missing named procedure.  Previously,
 * the procedure returns the number of parameters in the first result.  If the
 * procedure was missing, the number would be 0, just as if it was a parameterless
 * procedure.  Now, if the procedure doesn't exist, the reported number of
 * parameters is -1.  The storedproc.cpp code checks for the value and throws an
 * exception if the procedure isn't found.
 */
struct ssys_get_procedure_params_result_1
{
   unsigned long parameter_count /**< Number of parameters in named procedure. */
};

/**
 * @brief Second result of @ref ssys_get_procedure_params.
 *
 * The names and types of parameters for the named procedure.
 */
struct ssys_get_procedure_params_result_2
{
   char[5]  mode;    /**< direction of data: 'in', 'out', or 'inout' */
   char[64] name;    /**< parameter name */
   char[64] dtype;   /**< data type of parameter */
   int    len;       /**< length, in characters, of string representation of value */
   int    num_prec;  /**< number of significant digits in number value */
   int    num_scale; /**< number of digits after the decimal point in number value */
   char*   dtdid;    /**< full type description to discern unsigned int value */
};

/**
 * @brief Result of @ref ssys_session_start
 *
 * The values needed to create the session cookie values.
 */
struct ssys_session_start_result
{
   int      id;   /**< session ID of new session */
   char[32] hash; /**< session hash of new session */
};

/**
 * @brief Leaves a dropped_salt (@dropped_salt) session variable.
 *
 * This function leaves a string value to be picked up by a later
 * stored procedure as a way of allowing the SchemaFW C++ code to
 * use system resources to generate a random string that can be used
 * to salt a password hash.
 */
void ssys_drop_salt_string(const char *salt_string);

/**
 * @brief Combines salt and password to make a MD5 hash
 *
 * This simple function exists to enforce a consistent order for hashing so
 * that ssys_confirm_salted_hash() can apply the same algorithm when comparing
 * passwords.
 */
const char* ssys_hash_password_with_salt(const char* password,
                                         const char* salt_string);

/**
 * @brief Confirms hashed password with constant string comparison.
 *
 * This function performs a constant-time string comparison of hashed
 * passwords in order to thwart timing-based password cracking.  This
 * strategy follows some advice on this CrackStation [article](https://crackstation.net/hashing-security.htm)
 */
bool ssys_confirm_salted_hash(const char* hash,
                              const char* salt_string,
                              const char* password);

/**
 * @brief Returns parameter information of named procedure.
 *
 * @param p_proc_name Name of procedure
 *
 * @return ssys_get_procedure_params_result_1
 * @return ssys_get_procedure_params_result_2
 *
 * Use the procedure to get stored procedure parameter information
 * for creating a schema and interpreting http form requests.
 *
 * SchemaFW calls this procedure to create a DataStack of parameter
 * information.  The first result tells SchemaFW how many parameters
 * will be returned in the second result, so an appropriatly-sized
 * array can be prepared in which to store the results of the second
 * result.
 *
 * The parameter list is used as the starting point for generating a
 * schema and to collect http form request fields into running the stored
 * procedure.
 */
void ssys_get_procedure_params(char[64] p_proc_name);

/**
 * @brief Get information_schema.COLUMNS.COLUMN_TYPE value of specified columns.
 *
 * Primarily used for getting SET and ENUM datatype value lists from a table
 * definition, this procedure collects the COLUMN_TYPE (also known as the DTD
 * type) values from the information_schema for table:column pairs in a string.
 *
 * This procedure is used when a Specs-file response mode instruction
 * indicates that a text form field should render as an ENUM or SET type by
 * using the ENUM or SET information from a specific column of a table.
 * Typically, the reference table:column will ultimately be where the
 * form value will be stored, and using an ENUM restricts the text entry
 * to allowed values.
 *
 * @param column_list A comma-separated list of paired table:column name pairs
 *                    for which to return the COLUMN_TYPE values.
 *
 ~~~sql
 SET @list = 'Person:honorific,Phone:location,';
 CALL ssys_get_column_dtds(@list);
 ~~~
 */
const  char **ssys_get_column_dtds(const char *column_list)

/**
 * @brief Returns the set of procedure names in the current database.
 *
 * @return char[64][] ROUTINE_NAME
 */
char[64][] ssys_get_procedures(void);

/**
 * @brief Confirms existence of a table in the current database.
 *
 * @param p_table_name name of table to seek
 * @return number of tables whose name matches p_table_name
 *
 * Confirm the existence of a table in preparation for LOAD DATA.
 * A return value should be either 1 or 0 (duplicate table names
 * are not possible), with 1 confirming and 0 denying the existence
 * of p_table_name.
 *
 * The primary purpose of this procedure is to confirm the existance
 * of the quarantine table before attempting to load data into it.
 *
 * @sa @ref SDG_Importing_Data
 * @sa @ref Schema::import_table()
 */
int ssys_confirm_table(char[64] p_table_name);


/**
 * @brief Remove current session-tagged records from named the table.
 *
 * Delete records from the named table whose id_session field value
 * matches the @session_confirmed_id value.
 *
 * This procedure (or its replacement if provided) removes records
 * leftover from a previous session.  This procedure (or its replacement)
 * will be called
 *
 * - Prior to importing new records,
 * - After the import set is either accepted in incorporated into the application, or
 * - After an import set is abandoned/rejected.
 *
 * @sa @ref SDG_Importing_Data
 * @sa @ref Schema::import_table()
 */
void ssys_default_import_removal(char[64] p_table_name);

/**
 * @brief Display the session-tagged records from the named table.
 *
 * This procedure (or its replacement, if provided) should be called after
 * a table import so the client can confirm that the column data matches the
 * column heads.  This allows the named table to serve as a data quarantine.
 *
 * @param p_table_name The name of the quarantine table.
 * @param p_limit Maximum number of rows of the quarantine table to return.
 *                Use 0 or less to return all rows.
 *
 * @return All columns in the table, and as many rows as specified.
 *
 * @sa @ref SDG_Importing_Data
 * @sa @ref Schema::import_table()
 */
table ssys_default_import_confirm(char[64] p_table_name, int p_limit);

/**
 * @brief Makes single-row table for building a calendar with sfw_calendar.xsl
 *
 * This procedure returns a single-row table with values used by sfw_calendar.xsl
 * to construct a calendar.  The values in the single row are:
 * - **month** string of form YYYY-MM
 * - **initialDay** the day-of-week of the first of the month (Sunday=0)
 * - **countOfDays** the number of the last day of the month.
 */
table ssys_month_into_result(DATE mdate);

/**
 * @brief Calculates DATE values for the first and last days of the month.
 *
 * @param mdate     Date value of the month to process.
 * @param first_day OUTPUT parameter in which is returned the date value
 *                  of the first day of the month.
 * @param last_day  OUTPUT parameter in which is returned the date value
 *                  of the last day of the month.
 *
 * @return void
 *
 * These values are used by sfw_calendar.xsl to build a calendar.  It does
 * not create a result, running it does not affect the position of other
 * result queries in a procedure.
 */
void ssys_month_get_first_and_last(DATE mdate, DATE &first_day, DATE &last_day);


/**
 * @brief SchemaFW System procedure that is called with each new http request.
 *
 * This procedure prepares the MySQL session variable associated with
 * SchemaFW before each request to ensure that the use cannot get access
 * to the information provided by the previous request.
 */
void ssys_clear_for_request(void);



/**
 * @brief Loads a random hash string prior to establishing a new session.
 *
 * @param session_string 32-character random hash string
 *
 * Simple procedure that saves session_string to the session variable
 * @session_string_seed.  This value will be used when creating a new
 * session.
 *
 * @sa @ref ssys_session_create
 */
void ssys_seed_session_string(char[32] session_string);

/**
 * @brief Function to calculate time of moment 20 minutes in the future.
 *
 * @note MySQL Functions are effectively private because they cannot be
 * directly accessed with the C API.
 *
 * This function is called by sys_session_create and ssys_session_confirm
 * to set the SSYS_SESSION expires value.
 *
 * Be aware that this function can be replaced with a custom version in
 * order to set a different time limit.
 */
DATETIME ssys_calc_session_expires(void);

/**
 * @brief Function that adds or updates a SSYS_SESSION record.
 *
 * @return id value of new session
 *
 * @throw SIGNAL Throws an error if @ref ssys_seed_session_string was not
 * previously called.
 *
 * @note Is this function even being called?
 *
 * @note MySQL Functions are effectively private because they cannot be
 * directly accessed with the C API.
 *
 * This function searches for an available record in SSYS_SESSION to use,
 * creating a new record if necessary.  Sets @ref session_confirmed_id to
 * the id of the new session record and returns the id from the function.
 */
int ssys_session_create(void);


/**
 * @brief Procedure to start a session.
 *
 * This procedure calls the @ref ssys_session_create function to initialize the
 * SchemaFW session.  If @ref ssys_session_create returns successfully, this
 * procedure proceeds to call App_Session_Start to perform application-specific
 * initalization, then queries for the session id and hash value.
 *
 * @note Like the @ref ssys_session_create function it calls, app_session_create
 *       requires that @ref session_string_seed is set by a call to
 *       @ref ssys_seed_session_string.
 *
 * @return ssys_session_start_result
 */
ssys_session_start_result* ssys_session_start(void);


/**
 * @brief Determines if a session id and hash are currently active.
 *
 * This procedure compares the hash string with the hash string in
 * the SSYS_SESSION record whose key field matches session_id.  If the
 * session is found, matched, and not expired, the expires value will
 * be extended using ssys_calc_session_expires, @session_confirmed_id will
 * be set to session_id, and "1" will be selected to to return the session
 * status to the calling frame.  Otherwise, @session_confirmed_id will be
 * set to NULL and "0" will be selected to indicate failure.
 *
 * The record is conceptually available as soon as it is determined to
 * be expired, but we don't clear it here.  Rather, we defer clearing to
 * ssys_session_cleanup when it is called by the event ssys_session_event.
 *
 * @param session_id identify session (from http cookie)
 * @param session_string session-confirming hash string (from http cookie)
 * @return 1 if valid session, 0 if expired or nonexistant.
 */
int ssys_session_confirm(unsigned session_id, char[32] session_string);


/**
 * @brief Like an assert, this procedure emits an error SIGNAL if
 * id != \@session_confirmed_id.
 *
 * @param id value from cookie value
 *
 * @throw SIGNAL If id doesn't equal @session_confirmed_id
 *
 * This procedure could be called at the beginning of application procedures.
 *
 * A mismatched id vs @session_confirmed_id should never happen, but in order
 * to protect users against coding mistakes that might expose data from another
 * account, this procedure will emit an error SIGNAL rather than allow data
 * acquisition to continue.
 */
void ssys_assert_session_id(unsigned id);

/**
 * @brief Abandons a session by clearing the SSYS_SESSION record and setting
 * available = true.
 *
 * @param session_id id of the session (from http cookie)
 * @param session_string session-confirming hash string (from http cookie)
 *
 * @return Number of sessions abandoned.
 *
 * If the cookies identifying the session are cleared from the client, the session
 * is abandoned and will be cleared at the next call to ssys_session_cleanup.
 * However, SchemaFW calls this function to at a logout event to explicitly clear
 * the record before its expiration.
 *
 * The procedure returns the number of sessions abandoned, which can probably
 * safely be ignored in most circumstances.
 */
int ssys_session_abandon(unsigned session_id, char[32] session_string);

/**
 * @brief Clears and marks as available every expired SSYS_SESSION record.
 *
 * This procedure is called at regular intervals by the event ssys_session_event.
 * (See sys_procs.sql for ssys_session_event definition.)
 *
 * This procedure dictates the use of triggers for calling app-specific
 * clean up procedure App_Session_Abandon.
 */
void ssys_session_cleanup(void);

/**
 * @brief Artifact debugging function to confirm existence of App_Session_Start
 * and App_Session_Abandon before using the procedures in a trigger procedure.
 *
 * This procedure would only run in the script to confirm trigger procedures
 * before defining the trigger.  The new process of making dummy versions
 * of App_Session_Start and App_Session_Abandon negates the need for this
 * procedure.  I'm keeping it around in case I need it in the future.
 */
void ssys_setup_confirm_trigger_targets(void);

/**
 * @brief This modification trigger for SSYS_SESSION is called when a session
 *        expires or is abandoned.
 */
void ssys_sync_session_update(void);


/**
 * @brief Dummy startup procedure
 */
void App_Session_Start(unsigned id);

/**
 * @brief Dummy restore procedure
 */
void App_Session_Restore(unsigned id);

/**
 * @brief Dummy cleanup procedure
 *
 * This function is called by the ssys_sync_session_update() trigger when a
 * session expires or is abandoned.  Use this function to delete or clear
 * session information that is in the database.
 *
 * It is the app developer's responsibility to ensure that no data from an
 * abandoned session can be accessed by the next user who is assigned the
 * session id of the abandoned session.
 */
void App_Session_Abandon(unsigned id);


/** @} */
