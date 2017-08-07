# Importing Files

In order to allow clients to efficiently establish an account, they will almost
certainly appreciate having a usable import function to initialize their tables.

## Importing Data

When a user imports a spreadsheet, he or she should have an opportunity
to confirm the interpretation of the data before it's committed to
a permanent table.  SchemaFW does provide this feature.

## System Setup

Importing data requires several configuration steps, both for the system
and in scripting

### Spreadsheet Converter

SchemaFW uses a [Gnumeric](www.gnumeric.org) utility, _ssconvert_, to convert
spreadsheet formats into a consistent CSV format that MySQL can import.  
refer to the Gnumeric [importing documentation](https://help.gnome.org/users/gnumeric/stable/gnumeric.html#sect-files-ssconvert)
for information that SchemaFW uses to use _ssconvert_ and for how to determine
the supported file formats (i.e. `ssconvert --list-importers`).

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

The SchemaFW user must be given _FILE_ privileges.  Normally, this is a dangerous
privilege.  However, in the context of running SchemaFW, it should be less
dangerous because the LOAD DATA INFILE is disabled via the initialization callback
sent to the _mysql_set_local_infile_handler_ function.  

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

Refer to [LOAD DATA LOCAL INFILE](LoadDataLocalInfile.md) to a discussion
of security issues surrounding LOAD DATA INFILE, and how SchemaFW
protects against the issues.

Also refer to [Configuring MySQL](ConfiguringMySQL.md) to see how to
configure the SchemaFW MySQL client for FILE privileges.

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

