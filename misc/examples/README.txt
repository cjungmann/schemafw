These instructions are somewhat general, but since I'm
working in an Apache environment, the instructions are most
explicitly for Apache.  There is no reason I know that
other environments would not be suitable, but I'm not
knowledgable about other web servers, so I have to leave
setup in those environments up to you.

Under each directory is a setup and a site directory.
Use the following steps to create a site from an example:

1. Create a new MySQL database.
2. Run the sql scripts in the setup directory.
3. Create a directory for the Apache site.
4. Copy the contents of the example site folder into your site folder.
5. Create symbolic links to the schema executable and the web_includes folder.
6. Copy and rename the ApacheSite file for /etc/apache2/sites-enabled.
7. Modify the renamed ApacheSite file with your particulars.
8. Restart Apache2 (sudo /etc/init.d/apache2 restart).

Some of these steps will change as I update the software.
In particular, I will provide a script to do the copying
in steps 4 and 5.  For now (2015-07-26), there is a make
command (make sitesetup path=/the/path/to/your/site) that
makes the symbolic links in step 5.  When I figure out
where the most appropriate place for it, I will write a
different script to do step 5, and another script to do
step 4.

Apache Setup.

As mentioned above, the assumed environment is Apache, 
and I've included an ApacheSite example to put
in your /etc/apache/sites-enabled directory.  This site
file assumes that your application runs entirely with
browser-side XSL templates from SchemaFW-generated XML
documents.

You will have to review and most likely change the
VirutalHost values for DocumentRoot and ServerName.  You
will also have to deal with the MySQL credentials, which
in this example are handled elsewhere, but could be put
here as well.  Just uncomment and assign appropriate values
for MYS_HOST, MYS_USER, MYS_PASS.

For further instructions, see the online documentation.
