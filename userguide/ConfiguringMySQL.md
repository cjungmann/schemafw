# Configuring MySQL

This guide will walk you through three MySQL configuration tasks:
- setting a root password,
- enabling event scheduling, and
- creating a MySQL user account for the web user.

## Set the Root Password

Replace _YourPassword_ in the `-p` parameter with your own root password:

    mysql -u root -p*YourPassword*

## Prepare The Server for Event Scheduling    

The Schema framework uses the MySQL Event Scheduler to periodically check a list
of sessions to clean up after sessions that have expired.   An event object that
cleans up old sessions in *sys_procs.sql*.

### /etc/mysql/my.cnf

To ensure that the event scheduler is running when MySQL restarts, it is necessary
to set the option in a MySQL options file.  I put the following option line under
the `[mysqld]` section:

~~~
event_scheduler=ON
~~~

## Create MySQL User Account _WebUser_ for Database Access

This guide will walk you through creating a new MySQL User, granting
limited privileges to the new user, and setting up the server environment
so the credentials for this user are automatically loaded when invoking
procedures in a SchemaFW script.

## Create the MySQL User Account _webuser_

    mysql> CREATE USER 'webuser'@'localhost' IDENTIFIED BY 'webuser';

The statement above creates my 'webuser' account and assigns 'webuser'
as the password.  We could use a complicated password, but since it must
be readable by SchemaFW from somewhere, it is not completely secure.  We
will protect the data by restricting privileges, instead.

    mysql> GRANT EXECUTE ON *.* to 'webuser'@'localhost';
    mysql> FLUSH PRIVILEGES;

(FLUSH PRIVILEGES activiates the new grants.)

Now the only thing that _webuser_ can do is execute stored procedures.
This is pretty limiting, and security can be further enhanced by using
logins with cookies, then including a CALL to a security procedure at
the beginning of each stored procedure.

### Support Importing Files

SchemaFW uses `LOAD DATA LOCAL INFILE` to read imported data.  This requires
granting _webuser_ `FILE` privileges.

    mysql> GRANT FILE ON *.* to 'webuser'@'localhost';

, which has some security issues.  Read
[Using LOAD DATA LOCAL INFILE](LoadDataLocalInfile.md) for links to MySQL documentation
about the security issues and how SchemaFW addresses them.

## Enable _webuser_

Schema will need to create a working connection to MySQL using the _webuser_
account.  There are two ways to do this.  _I recommend the **second** method below._

1. Prepare environments variables MYS_HOST, MYS_USER, MYS_PASS, and MYS_DB
   with the host, user name, password, and database values, respectively.
   If set, the schema.fcgi program will use these environment variables to
   establish a connection.  These environment variables can be set in the
   virtual host configuration file.
   
2. Add the following to _/etc/mysql/my.cnf_.  Although MySQL might search other places
   for the configuration file, SchemaFW specifically looks to _/etc/mysql/my.cnf_
   for connection credentials.
   ~~~
   [client]
   host = localhost
   user = webuser
   password = webuser
   ~~~
   
   Adding the `[client]` section to _my.cnf_ is only appropriate for a
   strictly-limited MySQL user like webuser.  Do not use root or another
   highly-privileged user for a default client.
   
## Conclusion

With the completion of the steps above, your system will be able to use a
password-protected user account without having to include the password in
any file on your website.

--------------------------------------------------------------------------------

Back: [Configuring Apache](ConfiguringApache.md)
&nbsp;
&nbsp;
Up: [Preparing to Use SchemaFW](PreparingToUseSchemaFW.md)
&nbsp;
&nbsp;
Top: [Main Page](UserGuide.md)

