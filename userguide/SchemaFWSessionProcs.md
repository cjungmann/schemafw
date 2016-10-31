# Session Processing Procedures

When an SRM response mode contains a **$session_type** instruction that is not
set to "none", either as a global mode or as an discrete line in the response mode,
SchemaFW enters the session mode, where it will restore a previous session or
establish a new one if no session is in force.

See also [SchemaFW Authorizations](SchemaFWAuthorizations.md) for other MySQL and
SRM samples that affect sessions.

## Explanation

A session is identified with a pair of cookies, an integer and a string.  If a page
requires a session that doesn't exist, the server will create a new session integer
and string before calling specifically-named procedures that the application developer
will have prepared.

These procedures are called by the framework to accomplish certain things.  Although
these procedures are created during the site install, developers will typically
replace these procedures with custom versions that handle the unique needs of their
own applications.

These procedures are the minimum required to support anonymous sessions.  They are
called from within the SchemaFW C++ code, so empty versions have been included
in *sys_setup.sql* to ensure they exist.  Do not drop the procedures without
creating the replacement.

Password protected nonanonymous sessions require additional procedures.  Look at
these [boilerplate examples](SchemaFWAuthorizations.md) for help getting started
and as templates to copy and modify.

### The Overwritten Procedures

The procedures are

- **App_Session_Cleanup()**
  Called before, and most importantly, just after processing a http request,
  this procedure should, at a minimum, protect sensitive MySQL session
  variables.  The session variables should be set to NULL, an empty string,
  or, in some cases, a default value.

  #### Session Variables

  MySQL connection-based session variables are characterised by having a _@_ prefix,
  like `@session_confirmed_id` or `@dropped_salt`.  They are persistent global
  variables that retain their values outside of function or procedure calls.

  They can be a security issue for SchemaFW because, as a FASTCGI application,
  each instance of the Schema server opens and keeps reusing its own MySQL
  connection.  Session variables that are set while processing a request and left
  uncleared can be read by subsequent requests, possibly exposing sensitive
  information allowing unauthorized access to an account.

  It is important to carefully track and manage MySQL session variables.  Use
  procedure- or function-local variables (with the `DECLARE` MySQL statement)
  variables wherever possible to avoid accidentally leaving a session variable
  exposed.
  
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
  obsolete because of the *$test_authorized* instruction that names a procedure
  the schema.fcgi should call to get permission to continue.  A weasly statement,
  I know, but I am reserving the right to go back to this.

### The Session Initialization Procedure

There is no specific _session initialization_ procedure, but the name refers to a
procedure that sets the persisting session values after the *App_Session_Start*
procedure is called.

### Schema Procedures Template

The following partial file listing contains empty versions of the overwritten
that can be pasted into applications to serve as a kind of check-list to ensure
all the procedures are properly defined.

~~~sql
-- --------------------------------------------
DROP PROCEDURE IF EXISTS App_Session_Cleanup $$
CREATE PROCEDURE App_Session_Cleanup()
BEGIN
   -- Clear/NULL session variables ahead of next user
   SELECT NULL, NULL INTO @id_session, @id_user;
END $$

-- ------------------------------------------
DROP PROCEDURE IF EXISTS App_Session_Start $$
CREATE PROCEDURE App_Session_Start(session_id INT UNSIGNED)
BEGIN
   -- Ensure an, usually empty, linked session record is available
   INSERT INTO Session_Info (id_session) VALUES(session_id);
END $$

-- --------------------------------------------
DROP PROCEDURE IF EXISTS App_Session_Restore $$
CREATE PROCEDURE App_Session_Restore(session_id INT UNSIGNED)
BEGIN
   -- Set session variables from session table record:
   SELECT s.id_session, s.id_user INTO @id_session, @id_user;
     FROM Session_Info s
    WHERE id_session = session_id;
END $$

-- --------------------------------------------
DROP PROCEDURE IF EXISTS App_Session_Abandon $$
CREATE PROCEDURE App_Session_Abandon(session_id INT UNSIGNED)
BEGIN
  -- Remove or otherwise cleanup session record(s):
  DELETE FROM Session_Info
   WHERE id_session = session_id;
END $$
~~~

--------------------------------------------------------------------------------

Related: [Session and Login Case Study](SessionLoginCaseStudy.md)
&nbsp;
&nbsp;
Top: [Main Page](UserGuide.md)

