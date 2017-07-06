# Enum Field Settings

When a stored procedure parameter maps to an ENUM datatype in a MySQL table,
the most logical input type is a select from the limited set of options.  While
the developer can explicitly declare the enumeration in the SRM file (even for
non-ENUM datatypes (Confirm this, it may no longer be true)), SchemaFW includes
a special field instruction to look up a column definition to create an enum list
of options.

## Enums and the **schema_proc**

~~~srm
add
   type        : form-new
   schema_proc : App_Item_New
   form-action : ?add_submit
   schema
      field : itype
         enum : Item:itype
~~~

Notice **enum** instruction for the field.  The value that follows is the
table and column names, separated by a single colon.

## Enum Lookup Not Available For Result Schemas

### Workaround

If an enum field is required for a form, use a *schema_proc* instruction,
even if the result fields are carefully matched to the procedure parameters.
An ENUM field in a result cannot be adequately identified.

Read the following section for a more complete explanation

### Explanation of Why Result Enums Cannot Work

It would be convenient if SchemaFW would also lookup the column definition
for results, but it isn't possible without significant changes.  The problem
is that the DTD_ID value of the ENUM field is not included in the field information
that is used to build the schema from a result.  The DTD_ID can only be accessed
through a separate query, which cannot be run without a second connection, as the
current connection is only at the beginning of processing the result.  Attempting
to use the current connection will result in an out-of-sync error.

You can see that this is the case by debugging schema.fcgi, putting a breakpoint
in method BindStack::t_build() in bindstack.cpp.  MySQL misidentifies the datatype
as MYSQL_TYPE_STRING, but even if the flags are consulted to confirm that it's an
ENUM type, the information is not available.

For now, if a form schema requires an enum lookup, it must be done by including
a *schema-proc* instruction with an explicit field instruction to map the
procedure parameter to a specific table column.