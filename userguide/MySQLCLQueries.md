# MySQL Command Line Queries

There are times when you will need to query MySQL fro information about your site.
The following snippets of command line code should make it easier to access useful
information.

In order to avoid having to modify long and complicated command line templates, they
will include environment variables which will be set before the template is copied
and run.

In all cases, it is assumed that the following environment variables are defined
with appropriate values.  Any or all of these three variables might have different
values according to your needs.

~~~sh
MyUser=root
MyPassword=MyRootPassword
MyDatabase=CaseStudy
~~~

Other templates may use additional environment variables that will be listed before
the template, if necessary.

** Show Databases **

This template can be used to confirm that an expected database exists.  It doesn't
use the _MyDatabase_ environment variable.

~~~sh
mysql -u ${MyUser} -p${MyPassword} -e show\ databases
~~~

** Show Tables in a Database **

~~~sh
mysql -u ${MyUser} -p${MyPassword} ${MyDatabase} -e show\ tables
~~~


** List Database Procedures **

I use this procedure often when I need to see whether or not a certain procedure
has already been created.

~~~sh
mysql -u ${MyUser} -p${MyPassword} -e SELECT\ SPECIFIC_NAME\ FROM\ information_schema.ROUTINES\ WHERE\ ROUTINE_SCHEMA=\'${MyDatabase}\'
~~~

If the list of procedures is very long, it may be useful to filter the procedures
with a _LIKE_ condition.

~~~sh
MyFilter='App_%'

mysql -u ${MyUser} -p${MyPassword} -e SELECT\ SPECIFIC_NAME\ FROM\ information_schema.ROUTINES\ WHERE\ ROUTINE_SCHEMA=\'${MyDatabase}\'\ AND\ SPECIFIC_NAME\ LIKE\ \'${MyFilter}\'
~~~
