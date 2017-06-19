# Configuring Apache

Once the framework has been built, it is enabled through the use of Apache
configuration files.

## Prerequisites

This guide assumes that _mod_fastcgi_ is enabled in Apache and that the
[Schema](https://chuckj@bitbucket.org/chuckj/schema.git) has been downloaded,
built, and installed.  See [Building the Framework](BuildingTheFramework.md).

## Change XSL Mime Type If Necessary

This is not strictly an Apache issue, but it is a server issue that should be
considered.  Chrome browsers signal an error when XSL files are identified as
_application/xslt+xml_ (which is the default in the current version of Apache.

Sudo-open file /etc/mime.types and search for _xsl_.  If it's not already set,
change the line from

~~~conf
application/xslt+xml          xsl xslt
~~~

to:

~~~conf
text/xsl          xsl xslt
~~~

I also moved the line down to be with the other _text/_ mime types.  The mime
types were alphabetized, so I thought it better to conform to that standard
in case it was not just accidental.

## Global Apache Configuration

The settings in this section prepare Apache to use the _schema.fcgi_ server
to interpret [SRM files](SRMFiles.md), prompted by the file extension.  Use
the following commands:

~~~
sudo a2enmod actions fastcgi
sudo a2enconf schemafw
sudo service apache2 restart
~~~

Optional: If files with no extension should be interpreted as an SRM file,
open _/etc/apache2/conf-available/schemafw.conf_ and uncomment the directives
at the bottom of the VirtualHost definition and save the file.  After any
changes to _schemafw.conf_, Apache will have to be restarted:

    sudo service apache2 restart
   
## Disable PHP Processing

SchemaFW can be a complete website solution.  It is very resource stingy and fast,
pushing the page-rendering task off to the client.  It should be possible to run
a site entirely without PHP, in which case disabling PHP could a considerable
savings of computer resources.

There are two advantages of disabling PHP.  The first is that since PHP is so
powerful and privileged, eliminating its operation will increase a site's
security: even if a hacker could somehow install a PHP script in your site,
it wouldn't run.

The second advantage of disabling PHP, if possible, is that it will leave
more computer resources for running SchemaFW.  My understanding is that
if PHP is enabled, every HTTP request will spawn a PHP interpreter, even
if the target document is not a PHP file.

### Globally Disable PHP

This is the easiest change, but may not be appropriate for webservers that
host other sites that use PHP.

    sudo a2dismod php5
    sudo service apache2 restart

### Per-Host Disable PHP

Some documentation for this can be found at the
[PHP.net site](http://docs.php.net/manual/en/configuration.changes.php#configuration.changes.apache).
I haven't been able to get this to work like I want.

--------------------------------------------------------------------------------

Next: [Create MySQL User Account](CreateWebUser.md)
&nbsp;
&nbsp;
Back: [Building the Framework](BuildingTheFramework.md)
&nbsp;
&nbsp;
Up: [Preparing to Use SchemaFW](PreparingToUseSchemaFW.md)
&nbsp;
&nbsp;
Top: [Main Page](UserGuide.md)
