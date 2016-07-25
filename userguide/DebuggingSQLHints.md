# Debugging Hints for SQL Procedures

This cannot be a comprehensive debugging guide, but is, instead, a
collection of hints to help a developer debug MySQL stored procedures
that will run under SchemaFW.

## Debugging Procedures with Sessions

Normally, one can start an interactive MySQL session and invoke procedures
by using the `CALL` command.  When sessions are introduced, it get more
complicated.  Sessions save persistent data in tables, and most procedures
meant to run with sessions only work if a session is established.

Establish a session in interactive MySQL like this, using a 32-character
string as the session string seed.

~~~sql
mysql> CALL ssys_seed_session_string('abcdefghijklmnopqrstuvwxyz123456');
mysql> CALL ssys_session_start();
~~~

It is likely that some application-specific setups will be required, as
well.  It is possible that simply calling the login procedure with
appropriae values should set up the environment.

