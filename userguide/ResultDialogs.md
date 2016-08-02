# The Result Dialog

The Result Dialog is the result of a special response mode type _form-result_.
It is meant to be called by the _action_ attribute of a form (the _form-action_
instruction in a response mode).

## SchemaFW Dialog Review

SchemaFW dialogs are forms that are most often generated from from another page,
typically a table view.  When the user clicks to edit or create a record, a dialog
opens on top of the table.  If the dialog is submitted, possible errors are
announced and the results are incorporated into the table without a page change
or refresh.  These dialogs are the result of a selection on a web page.

## The Problem With Standalone Forms

Certain kinds of forms do not logically arise from a table view, but rather are
a user's first contact with an application.  Account creation and login forms are
obvious examples where there is no prior context from which a dialog can be
generated.  A survey is another example of a form that stands alone.

The problem with standalone forms is that the direct landing page, the `action`
attribute of the HTML `form` element cannot easily get a copy of the XML document
from the posted request.  The form fields are lost, so when the framework attempts
to get a copy for reference, it can only send the query string portion of the
request.  This creates havoc.

Take the login form.  The landing page of a posted login form is usually some sort
of a table view.  SchemaFW immediately requests a reference copy of the response
XML document whenever it opens a page.  This second request will be issued with
only the query string, the server then receives a login request without a username
and password, which shuts down the session, and returns the user to the login page.

The server could be programmed to ignore empty login pages if an authorized session
is in force, but this results in a security loophole.  If the landing page is the
result of a login form, someone could simply refresh the page to resubmit the
login credentials, even if the session has timed out.  It is better to replace
the history entry of the login response with another page, so someone can't search
the browser history and resubmit the form.

## The Result Dialog

The solution is a result dialog.  The result dialog is a form tagged as
`type : form-result` whose response contains a single result with a single
row indicating the result of the HTTP request.  The default usage of the
result dialog is to jump when the error is 0, otherwise to present the included
message with second opportunity to complete the form.

The login example below will illustrate the required elements of a result dialog.

## Login Example

A login page is the definitive result dialog situation.  There may be no logical
landing page before a user logs into an application, so the login dialog stands
alone.  While a login page could be written directly in HTML, with the result
read with Javascript, it is easier to use the SchemaFW in a manner consistent with
other transactions.

This example will show how a login transaction could be designed, and the peculiar
aspects of the result dialog will be explained along the way.  The stored procedure
will be used in two response modes: first to generate the form, then to evaluate
the result.

We'll use a not-particularly-secure MD5 hash to obscure the passwords from casual
viewing, thus the 32-character fixed-length _pword_ field.

There are several login-specific settings below:
- The MySQL login procedure (App_Login_Submit below) should
  - Test the login credentials
    - prepare app-specific session record if credentials confirmed
    - abandon any previous session if credentials not confirmed
  - Leave a result for the response
    - if authorized, prepare app-specific session record and SELECT 0
    - if not authorized, SELECT 1, include a message, and abandon
      any previous session.
- The SRM file response mode must
   - be `type : form-result`
   - have `session-type : establish` create a new session with but relax
     authorization requirements.
   - have a `jump` instruction to name the destination if login is successful
    

~~~sql
CREATE TABLE EmailList
(
   id    INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
   email VARCHAR(128),
   pword CHAR(32),

   INDEX(email)
);

CREATE TABLE Session_Info
(
   id_session INT UNSIGNED NULL,
   id_email   INT UNSIGNED NULL,

   INDEX(id_session)
);

CREATE PROCEDURE App_Login_Submit(email VARCHAR(128), pword VARCHAR(24))
BEGIN
   DECLARE email_id INT UNSIGNED;
   
   -- Test the login credentials:
   SELECT e.id INTO email_id
     FROM EmailList e
    WHERE e.email = email
      AND e.pword = md5(pword)
    LIMIT 1;

   -- One of three SELECT queries, depending on test outcome:
   IF email_id IS NOT NULL THEN
      INSERT INTO Session_Info(id_session, id_email)
             VALUES(@session_confirmed_id, id_email);

      -- (also checking for database error)
      IF ROW_COUNT() = 1 THEN
         SELECT 0 AS error, 'Success' AS msg;
      ELSE
         SELECT 1 AS error, 'Database error' AS msg;
      END IF;
   ELSE
      -- A failed login attempt should also invalidate any current session.
      -- This is a lazy abandon, the system's session record is left to expire.
      CALL App_Session_Abandon(@session_confirmed_id);
      SELECT 1 AS error, 'email/password mismatch' AS msg;
   END IF;
END $$
~~~

Here are the login procedure modes from an extract of a SRM file called `email.srm'.
The SRM file name is important because it's referenced in the _form-action_
instruction.

A few notes about what follows.

- The _session-type : establish_ will create a session but bypass the
  authorization check.  Otherwise, authorization failure jumps out to
  the _$jump-not-authorized_ value.

~~~srm
# Together, these two instructions make it so no page will be
# displayed unless there is an authorized session.
$test-authorized     : App_Check_Authorized;
$jump-not-authorized : email.srm?login

# The session-type : establish overrides the authorization jump
# response from the previous two instructions.
login
   type         : form-new
   session-type : establish
   schema-proc  : App_Login_Submit
   form-action  : email.srm?login_submit

# In this response mode, the type, session-type, and jump instructions,
# together cause a unique response from schema.fcgi.
login_submit
   type         : form-result
   session-type : establish
   procedure    : App_Login_Submit
   jump         : email.srm?home
~~~

Notes on the above modes:

- `type : form-new` renders an empty form where the login can be entered.

- `type : form-result` will jump if the response contains a row with an attribute
  named _error_ whose value is _0_.  It will follow the `jump` instruction.
  
- `session-type : establish` instructs _schema.fcgi_ to use a session, creating a new
  session if necessary, but to not require authorization.  Otherwise, authorization
  security would prevent even the login form from being sent.

--------------------------------------------------------------------------------

Top: [Main](UserGuide.md)