# Configuring a Virtual Host

When SchemaFW has been installed and configured as suggested in
[Configuring Apache](ConfiguringApache.md), SchemaFW will just
work with a simple virtual host configuration.  The program,
_schema.fcgi_ will be called to interpret any file with an extension
of _.srm_.

Here is a sample configuration file.  It should be created in the
/etc/apache2/sites-available directory, and enabled with the Apache
utility `a2ensite`

Create file _/etc/apache2/sites-available/mysite.conf_, and copy
these lines:
~~~apache
<VirtualHost *:80>
   DocumentRoot /var/www/mysite
   DirectoryIndex index.htm
   ServerName mysite.com
   ServerAlias /
</VirtualHost>
~~~

Return to the command line and type:

    sudo a2ensite mysite

## Links

[Introduction to SchemaFW](IntroductionToSchemaFW.md)

[Main Page](UserGuide.md)



