# Running Websites on LocalHost

You may like to develop your applications on a workstation that is also running
Apache.  This makes it easy to edit and run your application.  However, there are
several steps that must be taken to make this possible.

## Setup

Lets assume you use the following site configuration file for an internet-facing
website:

~~~apache
<VirtualHost *:80>
   ServerName www.your_domain.com
   DocumentRoot /home/your_user_name/www/cs/site/
   DirectoryIndex index.htm
</VirtualHost>
~~~

If you want to run it on your workstation, having prepared your computer by installing
a LAMP server and following the [Preparing to Use SchemaFW](PreparingToUseSchemaFW.md),
guide, these following steps may be neccesary as well.

1. Change the ServerName to the local name.  Using _cs_ (for Case Study) as an example,
   the modified config file will look like this:

   ~~~apache
   <VirtualHost *:80>
      ServerName cs
      DocumentRoot /home/your_user_name/www/cs/site/
      DirectoryIndex index.htm
   </VirtualHost>
   ~~~

   Step 3 includes a complete config file listing.

2. Make the site visible with an entry in your _/etc/hosts_ file.  Open _/etc/hosts_
   and add the following line near the bottom of the file:

   ~~~
   127.0.0.1    cs
   ~~~

   This entry will replace _cs_ in a URL with 127.0.0.1 (localhost).  Apache will direct
   requests to your site based on the _ServerName_ directive of the virtual host.

   The example above will allow you to navigate to `http://cs` with your browser.

3. Apache includes a _Directory_ directive in _/etc/apache2/apache2.conf_ for the
   default web directory, _/var/www_ that makes files accessible in directories
   under _/var/www_.  If you are hosting your sites in another directory, say
   _/home/your_user_name/www_, you could add a _Directory_ directive to make files
   accessible for all sites under the directory.

   ### Once for All

   Here is an example, added to _etc/apache2/apache2.conf_.  I added it just under
   the _/var/www_ _Directory_ directive.

   ~~~apache
   <Directory /home/your_user_name/www/>
      Options -Indexes +FollowSymLinks
      AllowOverride None
      Require all granted
   </Directory>
   ~~~

   ### Per-site Directory Permissions

   Alternatively, the _Directory_ directive could left out of
   _/etc/apache2/apache2.conf_ and included, instead, in each site configuration
   file.  This is an example of the configuration file above, modified to run
   on localhost;

   ~~~apache
   <VirtualHost *:80>
      ServerName cs
      DocumentRoot /home/your_user_name/www/cs/site/
      DirectoryIndex index.htm
      
      <Directory /home/your_user_name/www/>
         Options -Indexes +FollowSymLinks
         AllowOverride None
         Require all granted
      </Directory>
   </VirtualHost>
   ~~~

--------------------------------------------------------------------------------

Top: [Main Page](UserGuide.md)
