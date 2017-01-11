# Data Modeling

For the purpose of our discussion, we assume that the web server environment conforms
to the setup described in [Preparing the Framework](PreparingToUseSchemaFW.md).  In
that configuration, the extension *.srm* invokes the SchemaFW web service
(_schema.fcgi_) to produce an XML document.  This page will explain how the
design of your stored procedures affect the XML documents sent to your applications.

## Prepared Statements are Required

SchemaFW, in the default configuration, requires the use of _prepared statements_.
[Data Security Measures](DataSecurity.md) explains why this is a good practice.

## How Procedures are Called

The **procedure** instruction of a _response mode_ in an _srm_ file invokes the named
stored procedure.  The server runs the procedure and produces an XML file that is
sent to the browser.

## Simple Example

The procedure

~~~{sql}
CREATE PROCEDURE App_Show_People()
BEGIN
   SELECT id, fname, lname, email
     FROM People;
END $$
~~~

called with this response mode:

~~~{srm}
list
   type      : table
   procedure : App_Show_People
~~~

will produce the following XML:

~~~{xml}
<?xml version="1.0"?>
<resultset>
   <result rndx="1">
      <schema>
      </schema>
      <row id="1" fname="Adam" lname="Smith" email="asmith@email.com" />
      <row id="2" fname="Betty" lname="Rogers" email="brogers@email.com" />
      <row id="3" fname="Ruth" lname="Jones" email="rjones@email.com" />
   </result>
</resultset>
~~~


