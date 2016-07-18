# Using MySQL Procedures

All of the real work of a SchemaFW application occurs in the MySQL stored
procedures.  The server treats the procedures as black boxes: it only knows
and works with the parameter list and the resultset.  The server response
will always be an XML document, with _resultset_, _result_, _schema_, and _row_
elements, each of which can be renamed under the controls of the
[SRM Files](SRMFiles.md) that caused the procedure to run.

The parameter list is used to

1. create a schema for rendering a form, and

2. apply submitted form data to the procedure for execution.

The resultset is the entire packaged response of the execution of a procedure.
The resultset consists of

1. an optional top-level schema derived from a procedures parameter list

2. zero or more results, each representing the data returned from a query
   in the procedure.
   
Each result member of a resultset contains

1. An optional schema of the result's data.  It can be used to render a
   table or a data view.

2. Zero or more row elements, representing the rows returned by the query.

In most cases, the XML can not only be renamed, but also supplemented with
additional data, again under the control of the [SRM file](SRMFiles.md).




--------------------------------------------------------------------------------

Next: [Understanding SRM Files](SRMFiles.md)
&nbsp;
&nbsp;
Back: [Establish a New SchemaFW Site](CreateNewSite.md)
&nbsp;
&nbsp;
Up: [SchemaFW Basics](SchemaFWBasics.md)
&nbsp;
&nbsp;
Up: [Main Page](UserGuide.md)
