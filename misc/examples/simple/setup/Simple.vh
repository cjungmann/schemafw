<VirtualHost *:80>
   DocumentRoot /home/chuck/work/cgi/examples/simple/site
   DirectoryIndex schema
   ServerName simple
   ServerAlias /

   <Files "schema">
      SetHandler cgi-script
      Options FollowSymLinks
      Options +ExecCGI -MultiViews +SymLinksIfOwnerMatch
      Order allow,deny
      Allow from all
   </Files>

   # Set the following if you need environment-based MySQL credentials:
   # SetEnv MYS_HOST "localhost"
   # SetEnv MYS_USER "user"
   # SetEnv MYS_PASS "password"
   # SetEnv MYS_DB   "SchemaDemo"
</VirtualHost>

# Local Variables:
# mode: shell-script
# End:
  
