# Sessions and Authorization

SchemaFW offers server facilities for establishing and managing sessions through
the use of browser cookies.

## Setup

The Schema Framework includes many system stored procedures.  Although there
are many session-associated system procedures, the application developer must
implement several stored procedures that will be called by the framework.

The application session procedures that a developer must write will set up for
or clean up after a session, as well as confirm that a session is still valid.
A summary of the required procedures follows, a detailed explanation for each
procedure comes next, and finally, there will be a single code block that can
be copied into your own application as a template for your own development.

### Procedures and Their Responsibilities

- **App_Request_Cleanup()**: should clear all session variables.
- **App_Session_Start(int id)** : create table records for persistent session
  information.
- **App_Session_Restore(int id)** : read and use the persistent session information.
- **App_Session_Abandon(int id)** : either delete the session records, clear the
  session record and flag as available
- **App_Session_Confirm_Authorization(int id)** : by default returns 1, but can be
  overridden to return 0 if your authorization conditions have not been met.

#### App_Request_Cleanup()

This procedure is called at the _start_ of a session to ensure that session
variables are cleared before use.  It is done at the start to help prevent
private data from a previous session cannot be accessed, in case the previous
session was interrupted before clearing the variables.

#### App_Session_Start(int id)
  
This procedure is called when a session begins to give the application an
opportunity for any needed setup.  Primarily, this means creating one or
more records to hold persistent session information that will be read by
_App_Session_Restore_ when the user returns to the application.

#### App_Session_Restore(int id)
  
This procedure is called when schema.fcgi detects that the request comes from
a previously established session.  Session information should be read from
persistent storage, i.e. MySQL tables.  Typically, one would copy the persistent
data into session variables to make them available to any application procedures
that run during the current response.
  

#### App_Session_Abandon(int id)

This procedure should delete or clear all persistent records associated with the
session identified by _id_.

This procedure is called on by a trigger event when the system detects that the
session is no longer valid.  This will happen if the current time is after the
session expired time, but it may also happen at a user's request with a logout
response mode with;

    type : abandon-session


#### App_Session_Confirm_Authorization(int id)

While the Schema Server automatically checks if a session has expired, an
application may have additional conditions for granting continued authorization.
Overriding this procedure is the opportunity to enforce custom conditions on
session authorization. 



