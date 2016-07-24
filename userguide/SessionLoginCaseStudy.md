# Session and Login Case Study

The following is a simple implementation of a login session provided to
provide an example of the concepts mentioned in
[Session Processing Procedures](SchemaFWSessionProcs.md).  This page includes
little explanation, please refer to
[Session Processing Procedures](SchemaFWSessionProcs.md) for a discussion about
when and why these procedures are used.

### The Session Table

For our example, the procedures will be interacting with the following table:

~~~sql
CREATE TABLE IF NOT EXISTS Session_Table
(
   -- search index to find session values:
   id_session INT UNSIGNED,
   
   -- session values:
   id_person  INT UNSIGNED,
   text_field VARCHAR(80),
   int_field  INT UNSIGNED,
   date_field DATE,

   UNIQUE INDEX(id_session)
);
~~~

### Session Initialization Procedure

The following procedure sets the session table values only if the submitted
_email_ and _password_ values match a record.

Some important points about the procedure, general points first, then
application-specific after.

- It uses declared variables rather than session variables (prefixed with _@_) to
  hold temporary values.  This helps prevent polluting the reusable MySQL with
  unnecessary session variables.

- This procedure sets session values from another table, but it could have had
  parameters whose values would be entered into the session table.

- It is not necessary to set _all_ session table fields.  Some might be left blank
  to be filled in during different contexts.  There are no restrictions, this is
  only an example.

- This procedure is called after **App_Session_Start** during the processing of
  a _form-login_-type response mode.  If type proper response type was declared,
  the developer can be confident that *App_Session_Start* has been run before
  this procedure is called.
  
- Because the session is dependent on a valid login, this case study could defer
  creating the session record until the login is confirmed.  In that case, the
  UPDATE query would become an INSERT query, and *App_Session_Start* would be
  left unchanged from the default do-nothing procedure.


~~~sql
DROP PROCEDURE App_Session_Login $$
CREATE PROCEDURE App_Session_Login(email VARCHAR(180), password VARCHAR(24))
BEGIN
   DECLARE idfield INT UNSIGNED;
   DECLARE tfield  VARCHAR(80);
   DECLARE ifield  INT UNSIGNED;
   DECLARE dfield  DATE;

   -- set local variables from verified record
   SELECT p.id, p.nickname, p.age, p.hiredate INTO idfield, tfield, ifield, dfield
     FROM Person p
    WHERE p.email = email
      AND p.pword = md5(password)
    LIMIT 1;

   -- ROW_COUNT()=1 indicates that the email/password values matched a record
   IF ROW_COUNT()=1 THEN
      UPDATE Session_Table
         SET id_person = idfield,
             text_field = tfield,
             int_field  = ifield,
             date_field = dfield
       WHERE id_session = @session_confirmed_id;
   ELSE
     -- Typically, the invalid session will be allowed to expire,
     -- but some implementations with non-expiring sessions may
     -- have to explicitely call *ssys_session_abandon**  here
     -- CALL ssys_session_abandon(@session_confirmed_id,@session_seed_string);
   END IF;
END $$
~~~
                                  
### App_Session_Cleanup

Clean up session variable used in the response

~~~sql
CREATE PROCEDURE App_Session_Cleanup()
BEGIN
   SET @nickname = NULL,
       @age      = NULL,
       @hiredate = NULL;
END $$
~~~

### App_Session_Start

~~~sql
CREATE PROCEDURE App_Session_Start(session_id INT UNSIGNED)
BEGIN
   INSERT INTO Session_Table (id_session)
          VALUES(@session_confirmed_id);
END $$
~~~

### App_Session_Restore(session_id INT UNSIGNED)

Set session variables 

~~~sql
CREATE PROCEDURE App_Session_Restore(session_id INT UNSIGNED)
BEGIN
END $$
~~~


--------------------------------------------------------------------------------

Related: [Session Processing Procedures](SchemaFWSessionProcs.md)
&nbsp;
&nbsp;
Top: [Main Page](UserGuide.md)

