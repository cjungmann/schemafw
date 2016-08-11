# Session Login Procedure

The procedure used for confirming login credentials should continue or complete
the process of initializing the session.  The framework will have already established
a session and called the application-specific procedure App_Session_Start (see
[Sessions and Authorization](SessionsAndAuthorization.md)), but the user account
to be used is not known until the login procedure has been run.

I had some trouble with a recent login procedure, and in fixing the problems, I
have an initial draft of best practices for a login form.

## The Response Modes

~~~srm
# Excerpt from session.srm

login
   type : form-new
   session-type : establish
   schema-proc  : App_Session_Login
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
   procedure    : App_Session_Login
   jump         : session.srm?home
   
~~~

## The Stored Procedure

The following example procedure includes redundant parts for the purpose of
illustration.  The code comments will explain the interesting parts.

~~~sql
DROP PROCEDURE IF EXISTS App_Session_Login $$
CREATE PROCEDURE App_Email_Login(email VARCHAR(128), pword VARCHAR(32))
BEGIN
   DECLARE eid INT UNSIGNED;              -- to indicate good credentials
   DECLARE scount INT UNSIGNED DEFAULT 0; -- to check for missing session record

   -- Acts like an assertion if the session is not properly prepared.
   -- Redundant, the `ROW_COUNT()=0` check below warns of same problem.
   IF @session_confirmed_id IS NULL THEN
      SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'No Session to log into.';
   END IF;

   -- Check credentials.  Use LIMIT 1 since there should only be one.
   SELECT id INTO eid
     FROM Email e
    WHERE e.email = email
      AND e.pword = md5(pword)
    LIMIT 1;

   IF ROW_COUNT()=1 THEN
   
      -- Credentials established, prepare already-allocated session record.
      -- LIMIT 1 because there should only by one matching record.
      UPDATE Session_Info
         SET id_email = eid
       WHERE id_session = @session_confirmed_id
       LIMIT 1;

      -- Check and notify if UPDATE failed
      IF ROW_COUNT()=0 THEN
         -- UPDATE with no changes leaves ROW_COUNT()==0, so,
         -- in that case, also confirm that the record exists.
         SELECT COUNT(*) INTO scount
           FROM Session_Info
          WHERE id_session = @session_confirmed_id;

         IF scount=0 THEN
            SELECT 1 AS error, 'Failed to update Session_Info: record not found.' AS msg;
         ELSE
            SELECT 0 AS error, 'Successful login.' AS msg;
         END IF;
      ELSE
         SELECT 0 AS error, 'Successful login.' AS msg;
      END IF;
   ELSE
      -- Invalid credentials, clean up and send message
      CALL App_Session_Abandon(@session_confirmed_id);
      SELECT 1 AS error, 'email or password not valid.' AS msg;
   END IF;
END $$
~~~


