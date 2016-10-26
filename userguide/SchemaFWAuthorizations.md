# SchemaFW Authorizations

Simple sessions keep track of users' progress between requests.  For longer term
persistence, most websites create password-protected user accounts.  Setting up a
password-protected website is more complicated, especially with the responsibility
to protect the passwords.

Following are some boilerplate procedures and their associated SRM response modes
that can get you started with developing a password-protected web application.

## MySQL Code

Tables use in this example

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
~~~

Procedures

~~~sql
-- ---------------------------------------
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

  -- Use SchemaFW-provided hashing function to confirm credentials
  -- before saving application-specific data:
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
~~~

The following registration procedure starts with generic queries confirming
matching passwords and that the salt was provided.  These queries should be
copied to a derivative script.

NOTE the `proc_block:` label.  It will be used if early termination is necessary.
~~~sql
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

   -- Salt must have been dropped.  Fatal if not, throw SIGNAL:
   IF @dropped_salt IS NULL THEN
       -- Missing salt string is fatal, throw SIGNAL if missing
       SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'Missing salt string.';
   END IF;

   -- Application-specific code, confirm acceptable data.
   -- Email must be unique.  early terminate if not.
   SELECT COUNT(*) INTO email_count
     FROM Email e
    WHERE e.email = email;
   IF email_count>0 THEN
      SELECT 1 AS error, 'Email already in use' AS msg;
      LEAVE proc_block;
   END IF;

   -- Application-specific code using frameword function to hash the password
   -- Passwords match, email is unique...save the salt and salted hash value:
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

Use the above procedures with the following SRM response modes.  Each procedure
has two response modes, one to show the form, the second to save the form data.

Notice that the submit response modes include a jump instruction.  Generally,
form submission modes are not suitable for landing.

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
         label : Email
      field : pword
         label     : Password
         html-type : password

login_submit
   type         : form-result
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
         label     : Email
      field : pword1
         label     : Password
         html-type : password
      field : pword2
         label     : Password again
         html-type : password
   
register_submit
   type         : form-result
   session-type : establish
   procedure    : App_User_Register
   jump         : session.srm?home
~~~
