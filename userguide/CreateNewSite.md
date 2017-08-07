# Establish a New SchemaFW Site

At some point, there will be a script that automates some of this, but for the
purpose of explanation, this guide will detail all the steps necessary to establish
a new SchemaFW site.

## Prerequisites

This is a list of things that must be working before it will be possible
to create a new SchemaFW site.

1. The SchemaFW software must be installed.  If not, see
   [Building the Framework](BuildingTheFramework.md).

2. Apache must be configured to use _schema.fcgi_ as an SRM-file handler.
   If not, see [Configuring Apache](ConfiguringApache.md).

3. SchemaFW must have a limited-access MySQL user account, set up to be
   called with implicit credentials.  If not, see
   [Configuring MySQL](ConfiguringMySQL.md).

## Set up the Site

1. Prepare a Directory to Host the Code
   
   Create a new directory, either in the default location under _/var/www/_,
   or in a directory under your home directory for ease of development
   (no need for `sudo`).  For our examples, we'll assume the new directory
   is _~/www/CaseStudy_ so avoid needing `sudo` to copy or write file.
   Change _CaseStudy_ for your own directory name in the examples that follow.

   ~~~
   cd ~/www/
   mkdir CaseStudy
   ~~~
   

2. Under the new directory, create two subdirectories, _setup_ and _site_.
   
   _setup_ will have mostly MySQL scripts for creating the tables and loading
   the stored procedures.  _site_ is the directory that the Apache virtual
   host will point to.

   ~~~
   mkdir setup site
   ~~~

3. Pull some resources from _/usr/local/lib/schemafw/_.  The second line below
   (after _cd site_ ) creates a link to a directory with files that support the
   framework.  The XSL file in the third line is optional but also recommended
   as a fully-working boilerplate for delivering a _Schemafw_ web application.

   ~~~
   cd site
   ln -s /usr/local/lib/schemafw/web_includes includes
   cp /usr/local/lib/schemafw/default.xsl .
   ~~~

   Note that the second line makes a directory link.  The linked directory
   cannot be added to or deleted from, nor can one edit any of the files
   contained in the directory.  Against these inconveniences, having a link
   makes it easy to apply framework updates.  Custom Javascript or XSL files
   can be added to the _site_ directory or a subdirectory of _site_.

4. Prepare MySQL with a new database and loading the system stored procedures.
   
   To facilitate copy-and-paste of the commands, define the _MyUser_, _MyPassword_,
   and _MyDatabase_ in your shell environment like this, replacing the sample
   values with your own values.

   ~~~
   MyUser=root
   MyPassword=MyRootPassword
   MyDatabase=CaseStudy
   ~~~

   Then copy the following to your copy buffer, and insert them into your shell
   environment with `Ctrl-Shift-V`;

   ~~~
   mysql -u ${MyUser} -p${MyPassword} -e "CREATE DATABASE ${MyDatabase}"
   mysql -f -u ${MyUser} -p${MyPassword} ${MyDatabase} < /usr/local/lib/schemafw/sys_procs.sql
   ~~~

   Don't worry about _Already exists_ messages, if you see any.  These are related
   to session-handling stored procedures.  They are explained in
   [Sessions and Authorization](SessionsAndAuthorization.md).

5. Install and run the scripts to create the application's tables and stored
   procedures in _~/www/CaseStudy/setup_, and the SRM file(s) to
   _~/www/CaseStudy/site_
   
   Consider copying the file listings from [Case Study](LCRUDInteractions.md)
   for testing.

6. Install the site Apache configuration file.

   Use the following command to open/create the Apache configuration file.
   Replace _cs_ according to your own specification.

   Note that to create or edit the _.conf_ file, you will need to invoke
   root privileges, either with _sudo_ or by changing the login with _su_.
   
   ~~~
   sudo emacs /etc/apache2/sites-available/cs.conf
   ~~~

   With the file open, copy in the following contents, remembering to
   replace _your_user_name_ with your username and _www.your_domain.com_
   with an appropriate server name. Look at [Running Websites on Localhost](RunningOnLocalhost.md)
   for a discussion of the issues running your site and hosting Apache
   on your workstation.

   ~~~apache
   <VirtualHost *:80> 
     ServerName www.your_domain.com
     DocumentRoot /home/your_user_name/www/CaseStudy/site
     DirectoryIndex default.srm
   </VirtualHost>
   ~~~

   See [Running Websites on Localhost](RunningOnLocalhost.md) if you prefer to
   run your site on localhost, particularly while in development.  In particular,
   remember to add an entry to _etc/hosts_ for a _localhost_ site.

   Save the file, the enable the site and restart Apache

   ~~~
   sudo a2ensite cs
   sudo service apache2 restart
   ~~~

7. Test your setup by calling your browser.

   ~~~
   google-chrome http://localhost/cs
   ~~~

--------------------------------------------------------------------------------

Next: [Using MySQL Procedures](UsingMySQLProcedures.md)
&nbsp;
&nbsp;
Up: [SchemaFW Basics](SchemaFWBasics.md)
&nbsp;
&nbsp;
Top: [Main Page](UserGuide.md)

