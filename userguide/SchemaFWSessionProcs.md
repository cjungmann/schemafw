# Session Processing Procedures

When an SRM response mode contains a **$session_type** instruction that is not
set to "none", either as a global mode or as an discrete line in the response mode,
SchemaFW enters the session mode, where it will restore a previous session or
establish a new one if no session is in force.

## Explanation

A session is identified with a pair of cookies, an integer and a string.  If a page
requires a session that doesn't exist, the server will create a new session integer
and string before calling specifically-named procedures that the application developer
will have prepared.

### The Overwritten Procedures

These procedures are called by the framework to accomplish certain things.  Although
these procedures are created during the site install, developers will typically
replace these procedures with custom versions that handle the unique needs of their
own applications.

The procedures are

- **App_Session_Cleanup()**  
  Called at the beginning and end of a response.  Its job is
  to clear any session variables used in the response.  This is to prevent data from
  on request bleeding over to the next request.
  
- **App_Session_Start(session_id INT UNSIGNED)**  
  Called when a session is established.  The procedure should create space in the
  database to hold session variables.  Typically, these means creating a record in
  a table, using the *session_id* value for an index.

  After this procedure runs, the server will run the procedure named by the
  _procedure_ instruction in the SRM file.  In a login response mode, this procedure
  should by a _session initialization_ (see below) procedure that updates the session
  record with the session values that should persist during the session.
  
- **App_Session_Restore(session_id INT UNSIGNED**
  This procedure is called at the beginning of responses when valid session cookies
  are found.  It should prepare the MySQL connection for subsequent queries, typically
  by setting session variables according to the values in the space created by
  *App_Session_Start*.
  
  This procedure is called for response modes where the mode type **is not** either
  form-login
  

- **App_Session_Abandon(session_id INT UNSIGNED)**  
  Called when a session expires or a response mode calls for the session to be
  abandoned.  This procedure should undo whatever was done in *App_Session_Start*
  by either clearing the record for reuse or simply deleting it.
  
- _perhaps_ App_Session_Confirm_Authorization.  This may have been made
  obsolete  because of the *$test_authorized* instruction that names a procedure
  the schema.fcgi should call to get permission to continue.

### The Session Initialization Procedure

There is no specific _session initialization_ procedure, but the name refers to a
procedure that sets the persisting session values after the *App_Session_Start*
procedure is called.

### Schema Procedures Template

The following partial file listing contains empty versions of the overwritten
that can be pasted into applications to serve as a kind of check-list to ensure
all the procedures are properly defined.

~~~sql
DROP PROCEDURE IF EXISTS App_Session_Cleanup $$
CREATE PROCEDURE App_Session_Cleanup()
BEGIN
END $$

DROP PROCEDURE IF EXISTS App_Session_Start $$
CREATE PROCEDURE App_Session_Start(session_id INT UNSIGNED)
BEGIN
END $$

DROP PROCEDURE IF EXISTS App_Session_Restore $$
CREATE PROCEDURE App_Session_Restore(session_id INT UNSIGNED)
BEGIN
END $$

DROP PROCEDURE IF EXISTS App_Session_Abandon $$
CREATE PROCEDURE App_Session_Abandon(session_id INT UNSIGNED)
BEGIN
END $$
~~~

--------------------------------------------------------------------------------

Related: [Session and Login Case Study](SessionLoginCaseStudy.md)
&nbsp;
&nbsp;
Top: [Main Page](UserGuide.md)

