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

It would be convenient if SchemaFW would also lookup the column definition
for results, but it isn't possible without significant changes.  The problem
is that it requires running a separate query while the procedure is running.
This will cause an *out-of-sync* error.

For now, if a form schema requires an enum lookup, it must be done by including
a *schema-proc* instruction with an explicit field instruction to map the
procedure parameter to a specific table column.