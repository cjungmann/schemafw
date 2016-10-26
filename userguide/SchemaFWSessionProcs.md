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
-- Assume this simple Email table for user registration
CREATE TABLE IF NOT EXISTS Email
(
   id    INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
   email VARCHAR(128) NOT NULL,
   salt  VARCHAR(32) NOT NULL,
   hash  BINARY(16) NOT NULL,
   INDEX(email)
);

-- This table is an simplest case example. 
CREATE TABLE IF NOT EXISTS Session_Info
(
   id_session INT UNSIGNED NOT NULL,
   
   -- Make sure all fields after id_session) are initialized,
   -- either as NULL or with a default value.
   id_user    INT UNSIGNED NULL,
   
   -- Index the linking field for performace:
   INDEX(id_session)
);

-- Required SchemaFW session handling procedures:

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

-- Sample Login Procedures

-- ----------------------------------------
DROP PROCEDURE IF EXISTS App_User_Login $$
CREATE PROCEDURE App_User_Login(email VARCHAR(128), pword VARCHAR(32))
BEGIN
  DECLARE rec_id INT UNSIGNED;
  DECLARE rec_salt CHAR(32);
  DECLARE rec_hash BINARY(16);
  DECLARE scount INT;

  -- Confirm session record exists.  This is necessary because ROW_COUNT()
  -- will be 0 after UPDATE if there are no changes.  SIGNAL/throw if error.
  SELECT COUNT(*) INTO scount
    FROM Session_Info
   WHERE id_session = @session_confirmed_id;
  IF scount=0 THEN
    -- A missing session record is a fatal, must-be-fixed problem:
    SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'Missing session record.';
  END IF;

  -- Get information to confirm credentials:
  SELECT e.id_ruser, e.salt, e.hash INTO rec_id, rec_salt, rec_hash
    FROM Email e
   WHERE e.email = email;

  IF ssys_confirm_salted_hash(rec_hash, rec_salt, pword) THEN
     -- Save session information to table for persistence:
     UPDATE Session_Info
        SET id_ruser = rec_id
      WHERE id_session = @session_confirmed_id;

     SELECT 0 AS error, 'Successful login' AS msg;
  ELSE
     CALL App_Abandon_Session(@session_confirmed_id);
     SELECT 1 AS error, 'Login credentials do not match' AS msg;
  END IF;
END $$

-- This is a simplest case.  Typically, there you will save more registration info.
-- Note the proc_block: label.  It may be used for early termination upon a failure.

-- -------------------------------------------
DROP PROCEDURE IF EXISTS App_User_Register $$
CREATE PROCEDURE App_User_Register(email VARCHAR(128),
                                    pword1 VARCHAR(32),
                                    pword2 VARCHAR(32))
proc_block : BEGIN
   DECLARE email_count INT UNSIGNED;

   -- Passwords must match, early terminate if not
   IF STRCMP(pword1,pword2) THEN
      SELECT 1 AS error, 'Mismatched passwords' AS msg;
      LEAVE proc_block;
   END IF;

   -- Email must be unique.  early terminate if not
   SELECT COUNT(*) INTO email_count
     FROM Email e
    WHERE e.email = email;
   IF email_count>0 THEN
      SELECT 1 AS error, 'Email already in use' AS msg;
      LEAVE proc_block;
   END IF;

   -- Salt must have been dropped.  Fatal if not, throw SIGNAL:
   IF @dropped_salt IS NULL THEN
       -- Missing salt string is fatal, throw SIGNAL if missing
       SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'Missing salt string.';
   END IF;

   -- Passwords match, email is unique...save the salt and salt-modified hash value:
   INSERT INTO Email (email, salt, hash)
          VALUES(email, @dropped_salt,
                 ssys_hash_password_with_salt(pword1, @dropped_salt));

   -- Indicate result
   IF ROW_COUNT()=1 THEN                 
      SELECT 0 AS error, 'Registration successful' AS msg;
   ELSE
      SELECT 1 AS error, 'Registration failed' AS msg;
   END IF;
END $$
~~~

### SRM Response Modes

~~~srm
# Excerpt of session.srm

login
   type : form-new
   session-type : establish
   schema-proc  : App_User_Login
   form-action  : session.srm?login_submit
   schema
      title : Login
      field : email
         Label : Email
      field : pword
         Label : Password
         html-type  : password

login_submit
   type : form-result
   session-type : establish
   procedure    : App_User_Login
   jump         : session.srm?home

register
   type         : form-new
   session-type : establish
   schema-proc  : App_User_Register
   form-action  : session.srm?register_submit
   schema
      title : Register New User
      field : email
         label : Email
      field : pword1
         label : Password
         html-type : password
      field : pword2
         label : Password again
         html-type : password
   
register_submit
   type         : form-result
   session-type : establish
   procedure    : App_User_Register
   jump         : session.srm?home

~~~

--------------------------------------------------------------------------------

Related: [Session and Login Case Study](SessionLoginCaseStudy.md)
&nbsp;
&nbsp;
Top: [Main Page](UserGuide.md)

