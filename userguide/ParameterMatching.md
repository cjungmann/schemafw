# Procedure Parameter Matching

The Schema Framework works by running named stored procedures, applying, by
name, the fields of an HTTP POSTed form to parameters of a stored procedure.
This is a pretty simple concept, but there are some considerations to keep in
mind.

## Name Matching

The named data items in the HTTP request are used to fill the matching named
parameters of a specified stored procedure.  The matches are case-sensitive, so
pay attention to the case as well as spelling.

## POST and GET both used

The Schema server uses both POST and GET named values to set parameter values.
This provides some flexibility in setting parameter values without having to prepare
custom code. 

## Special Case: Anonymous Data Item

There is one situation where the strict name-checking is relaxed: when an id
value is appended to the response mode name:

~~~srm
# items.srm (incomplete file listing)

main
   type : table
   procedure : App_Item_List
   on_line_click : ?edit

edit
   type        : form-edit
   schema-proc : App_Item_Update
   procedure   : App_Item_Read
~~~

If the user clicks on the line with `data-id=1` shown by the *main* response mode,
the Schema server will be called thus:

~~~sh
$ schema.fcgi -s items.srm -m edit=1
~~~

This feature saves the developer extra effort when preparing the most common
request, to open a record. In the example above, the value *1* will be applied
to the first parameter of the stored procedure named in the *edit* response mode,
regardless of the name.

This feature works for procedures with one or more parameters, but the unnamed
value is applied only to the first parameter.  Other parameters must be set by
name.

This feature also benefits the developer when debugging response modes.  It is
often not necessary to confirm the parameter name to call a response mode to see
its output.  See [Debugging with Command Line Options](SchemaFCGIOptions.md).

