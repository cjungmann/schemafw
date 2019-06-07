# Using _LOAD DATA LOCAL INFILE_

SchemaFW uses
[LOAD DATA LOCAL INFILE](https://dev.mysql.com/doc/refman/5.6/en/load-data.html),
hereafter referred to as **LDLI**, to efficiently load imported files into MySQL.
There are [security concerns](https://dev.mysql.com/doc/refman/5.6/en/load-data-local.html)
associated with its use.  This section will explain how you can secure your server
against LDLI exploits.

## How SchemaFW Protects Against LDLI Exploits

### Setup Assumptions

- Using a MySQL account with only EXECUTE and FILE privileges.

  Limiting privileges to EXECUTE prevents any adhoc queries at all, much less
    use INSERT, UPDATE, DELETE, or any other command to modify data.  This should
      be sufficient to prevent any mischief.

  This also means that if you host a PHP site on another virtual host, you
    should use another MySQL account than the one you use to SchemaFW.

- Disable any other server-side script-processing like PHP, either globally
  or for the virtual host.

  SchemaFW can be a complete solution for delivering a website.  If your application
    can run within the capabilities of SchemaFW, you won't need PHP, and your security
      will be enhanced by disabling it.

### SchemaFW Prevents Unauthorized LDLI

SchemaFW disables FILE access by replacing the default local inline handlers with
the MySQL function [mysql_set_local_infile_handler]
(https://dev.mysql.com/doc/refman/5.6/en/mysql-set-local-infile-handler.html).
In particular, the _local_infile_init_ parameter is set to point to a function
that always return an error condition when another user attempts to use
LDLI.

When SchemaFW needs to use LDLI, it calls `mysql_set_local_infile_handler` with
its own handlers just before calling LDLI, then restoring the disabled handler
functions immediately after importing the file.

The SchemaFW source code includes further documentation that explains how it
secures against LDLI exploits.

