# SRM Schema Definitions

Schemas are fundamental to the Schema Framework.  In their most basic form, they
define the names and data types of the columns of a table or the parameters of a
stored procedure.  The schemas are then used by the client-side framework to
build tables, forms, or custom HTML.

A basic schema can be generated without any schema instructions, but by defining
a schema, the developer can activate many of the more useful features of the framework.

## Basic Schema Format

~~~srm
list
   procedure : App_Person_List
   type      : table
   schema
      name : person
      field : fname
         label : First Name
      field : lname
         label : Last Name
~~~

## Advanced Schema Definitions

### The Field Format Instruction

The default output of a column value is as an attribute in a row element.  In some
cases, it may be preferable to write a column's value as a child element or as the
text of the row element.  To output a column as the element text can be useful
for a generic template that may not know the name of an attribute, like an
integer-indexed lookup table.

~~~srm
list
   procedure : App_Person_List
   type      : table
   result : 1
      schema
         name : person
         field : fname
            label : First Name
         field : lname
            label : Last Name
   result : 2
      name : lookup
      schema
         name : term
         schema
            field  : kname
               format : text
~~~

Consider the second result in the example above.  Without the schema, the second result
would look like this:

~~~xml
<resultset ...>
   <result rndx="1" ...>
   .
   .
   </result>
   <result rndx="2" ...>
      <row id="1" kname="smart" />
      <row id="2" kname="attractive" />
      <row id="3" kname="wealthy" />
   </result>
</resultset>
~~~

With the schema:

~~~xml
<resultset ...>
   <result rndx="1" ...>
   .
   .
   </result>
   <lookup rndx="2" ...>
      <term id="1">smart</term>
      <term id="2">attractive</term>
      <term id="3">wealthy</term>
   </result>
</resultset>
~~~

Note that in the second example, the result and rows are renamed _lookup_ and _term_,
respectively.  Comparing the first and second examples, in the first example, the
column name must be used to access the _kname_ value, but in the second example, the
name is not used, the value is the value of the _term_ element.
