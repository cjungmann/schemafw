# Troubleshooting

This document is an effort to preserve learning associated with solving
problems encountered when using the Schema Server and Schema Framework.
This document began with one solution and should grow as I encounter and
solve additional problems.

### Schema Server Installation Problems


### Apache Errors

#### Site Configuration Failures

If errors occur when loading site *.conf* file while loading Apache,
consider the following items:

- Ensure the directory of the **DocumentRoot** value is registered
  for use in */etc/apache2/apache2.conf*.  Search *apache2.conf* for
  a line beginning `<Directory /var/www/>`, make a copy of the content
  of that entry (up to and including `</Directory>` and change the
  `/var/www/` to a directory that contains the host of your new site.

  