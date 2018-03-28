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
   cp /usr/local/lib/schemafw/default.xsl .
   ln -s /usr/local/lib/schemafw/web_includes includes
   ~~~

   - The second line copies a boilerplate XSLT stylesheet.  Replace the default
     strings in the *title* and *h1* elements with strings appropriate to your
     application.

   - The third line makes a directory link to framework files.  As a link to
     a privileged directory, the target directory cannot be added to or deleted
     from, nor can one edit any of the files contained in the directory.  However,
     using this link makes it easier to apply framework updates.  Custom
     Javascript or XSL files can be added to the _site_ directory or a subdirectory
     of _site_.

4. Prepare MySQL with a new database and loading the system stored procedures.
   
   To facilitate copy-and-paste of the commands, define the _MyUser_, _MyPassword_,
   and _MyDatabase_ in your shell environment like this, replacing the sample
   values with your own values.  Note that these values are not saved anywhere,
   they are simply used to replace placeholders in the copy-and-paste commands
   in the next section.

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
   # -*- mode:conf -*-

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

7. NameServer (DNS or /etc/hosts)

   A connection must be made between the name entered as *ServerName* in the *.conf*
   file and IP address of the computer hosting the site.  How this is done differs
   based on how the site is to be accessed.

   **Public or World-Wide Access** The standard way to do this is to add a record to
   a domain name server (*DNS*).

   - Open your the name server account for the domain name, which is
     *your_domain.com* in the example above.
   - Open the *host records* page for *your_domain.com*
   - Add an *A Record* for each prefix or _host_.  This associates the prefix
     or _host_ name to an IP address.  The hosts of a domain name may be on
     one or serveral servers.  A typical host is *www*, ie *www.your_domain.com*,
     which implies an *A Record* for *www* in the *your_domain.com* account.

   **Local Setup** for access through _localhost_ on a development computer,
   or for an anonymous computer on a local area network (*LAN*), simply add
   a record to the **hosts** file.  In the *hosts* file, the entire name must
   be entered across from the IP address.  That is, *www.your_domain.com*, 
   rather than simply *www*.

   - open */etc/hosts* in linux, *C:\Windows\System32\Drivers\etc\hosts* in Windows.
   - add a line with the IP address:

     - for localhost, `127.0.0.1  www.your_domain.com`.  This is for browser 
       navigating to the site from the computer that is hosting the site.
     - for another computer, replace *127.0.0.1* with the IP address of the computer
       hosting the site.

   For localhost or LAN access using the *host* file, the name can be simple,
   leaving off both the *www* host name and the top-level domain (*TLD*) *.com*.
   This simple name can replace the *ServerName* entry of the virtual host
   record in the Apache site configuration file.  The *hosts* entry name must
   match the *ServerName* entry for access.
   
8. Test your setup by calling your browser.

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

