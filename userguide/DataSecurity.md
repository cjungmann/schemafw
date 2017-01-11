# Data Security Measures

## Data Security and Prepared Statements

SchemaFW makes it easy to use
[Prepared Statements](https://en.wikipedia.org/wiki/Prepared_statement) for
database access.  In fact, the default configuration even prevents MySQL from
directly running a query: running stored procedures through prepared statements
are the only option for accessing the database.  This secures your data in two ways:

- Preventing _SQL Injection_ attacks, and
- Hiding all business code from the web site.

### Preventing SQL Injection 

[SQL Injection](https://en.wikipedia.org/wiki/SQL_injection) can expose a website
to attacks on its data.  The online comic XKCD presented an example of the danger in
[Exploits of a Mom](https://www.explainxkcd.com/wiki/index.php/327:_Exploits_of_a_Mom)
The best way to avoid SQL injection attacks is to use parameterized statements, where
the query and its values are transmitted separately.

### Hiding Business Code

There are times when the contents of your server script files can be accidentally
exposed due to configuration mistakes or a temporary failure of the web server
itself.  If sensitive queries are included in the script files, the exposed queries
may reveal company secrets and details about the structure of the data tables
that could inform directed attacks on the data.

