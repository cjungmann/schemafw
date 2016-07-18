# Importing Files

In order to allow clients to efficiently establish an account, they will almost
certainly appreciate having a usable import function to initialize their tables.

This page is just a stub until I work through the process again.  Until then,
the following notes were included in a README file that has since been deleted.
They may serve as a reminder.

## Importing Data

Most database-based applications will need to import data.  SchemaFW
does have an import function (in development at the time of this
writing 2015-10-19) that uses the Libre/Open-Office `unoconv` utility
to convert files and `LOAD LOCAL DATA` in MySQL.

When a user imports a spreadsheet, he or she should have an opportunity
to confirm the interpretation of the data before it's committed to
a permanent table.  SchemaFW does provide this feature.

__NOTE:__ This section is only a start to documenting the import function.
At the time of writing (2015-10-19) the import feature is not
yet complete, but some ideas have been worked out, and the intention
of these ideas will be useful for a developer, even if they are not
complete.
      

## Import Requirements

The developer must provide the following for successful import function:

1. A __holding table__, where the first column is an integer  _session field_,
   and the rest of the columns match the columns of the spreadsheet.  Column
   names are not important for importing, but will be used to confirm the
   import data.

1. A __truncation procedure__ that will remove the _holding table_ records
   with a specified integer value matching the _session field_.  This
   procedure will be called before importing new data and after moving
   an _import stage_ to permanent storage.

1. An __import stage review__ procedure for showing the import results before
   committing to permanent storage.

1. An __import stage commit__ procedure for moving the _import stage_ to
   permanent storage.

## Other Setup Hints

The following was copied from the aforementioned, and now deleted, file.
Several things in the following may prove useful when setting up and documenting
the import feature.

##### [mysqld] Section

To help make the FILE privileges less dangerous, we want to restrict
reading and writing files to a single directory.

~~~
[mysqld]
⋮
secure_file_priv=/tmp
~~~

In the unlikely event that a web user somehow manages to send commands
to MySQL through the SchemaFW directory, this setting should prevent
reading unauthorized data or writing unsafe files elsewhere on your
server.  See [LOAD DATA INFILE Setup](#load-data-infile-setup) for
a more complete discussion.

**NOTE** This is important enough to suggest you confirm this setting
before you release your site.  Log in to MySQL and issue the
`SHOW VARIABLES;` command and confirm the _secure_file_priv_ setting.

**NOTE** This recommendation will change when I finish with the design.
the `secure_file_priv` will be set to _/tmp/schemafw/pipes_ and that
directory should only contain pipe files used for importing.  That way
there will be nothing for a hacker to read in the only directory he or
she can read from using FILE privileges.


#### LOAD DATA INFILE Setup

The import feature of SchemaFW uses the _LOAD DATA INFILE_ MySQL
command to import tables.  _LOAD DATA_ and _OUTFILE_ commands are
powerful but very dangerous functions, and the default MySQL
environment makes them difficult to use, especially for our
purposes.


_LOAD DATA INFILE_, at least initially,
balks at loading files from directories that the webserver can
write to.

Refer to this [article](http://ubuntuforums.org/showthread.php?t=822084)
for more information.

The solution is to add an entry to __apparmor__.  Rather than create
a new shared directory, I decided to use /tmp for files and named pipes
that will be imported into MySQL.  I then added an entry to the
appropriate apparmor file.

``` sh
$ sudo emacs /etc/apparmor.d/usr.sbin.mysqld
```

added the line,

~~~
/tmp/** rwk,
~~~

then restarted apparmor:

~~~
sudo /etc/init.d/apparmor restart
~~~

