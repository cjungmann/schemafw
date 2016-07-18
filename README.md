# SchemaFW

**schema** an underlying organizational pattern or structure; conceptual framework
[dictionary.com](http://dictionary.reference.com/browse/schema)

The Schema Framework (SchemaFW) includes several parts that, working together and
using MySQL as the database, define a protocol by which the server workload is
substantially reduced and the client assumes most of the responsibility for
rendering and running a web application.

The SchemaFW server interpreter reads a SRM (Schema Response Mode) script file to
determine how to respond to a user request.  The server returns an XML file to the
client, which will be interpreted by the framework's tools to render web pages.

Most server responses will include _schema_ elements that include not only data types
and names of the data but also other developer hints, supplied in the SRM script, 
that inform the browser how to render pages, respond to user input, and send requests
back to the server.

The framework uses XSL extensively for running the application.  While much can be
accomplished with minimal knowledge of XSL, using XSL and AJAX makes it easy
to extend the framework with custom templates for unique interactions.

Browse the [User Guide](userguide/UserGuide.md) to see what SchemaFW is all about.

Look at the [Case Study](userguide/SchemaFWCaseStudy.md) for an overview of how
SchemaFW is used to develop a database-driven web application.

Consult [Building the Framework](userguide/BuildingTheFramework.md) for instructions
on how to download, build, and install SchemaFW.


