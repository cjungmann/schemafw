# Writing a SchemaFW Application

The hard work of writing a SchemaFW application is designing the database and
writing the stored procedures that give access to the database.

SchemaFW is designed to make it very easy to use prepared statements that access
the stored procedures.  This enhances security in two ways:

- Prepared Statements help eliminate SQL-injection attacks by submitting user
  input through parameters.
- The ease of using prepared statements should make it easier to accept another
  security setting: not allowing the web-host connection to directly execute queries.
  This way, even if a hacker was able to somehow transmit a malicious query to the
  website, it would not be allowed to run.
  