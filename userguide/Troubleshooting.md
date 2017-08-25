# Troubleshooting SchemaFW

This guide helps explain how to fix various problems whose solution might
be not be readily apparent.

## Table with GROUP BY field is missing line_id

### Problem

This message is shown when a response mode uses a procedure that includes
 a **GROUP BY** instruction but does not identify a **line_id** instruction.

### Solution

Include a **line_id** instruction in the response mode:

~~~srm
main
   type          : table
   procedure     : App_Items_With_Keywords
   on_line_click : ?edit
   # Include a line_id instruction like this:
   schema
      field : id
         line_id : true
~~~

Note that the framework only checks if the *line_id* attribute exists, it does
not check the value.  That way the value can be *1*, or *true*, or *yes*, but
it also means that a value of *0*, or *false*, or *no* will also be interpreted
as a *line_id* field.

## Explanation

The framework needs an item ID in order to request to appropriate record
from the database.  This value is normally written in the _tr_ table element
as the **data-id** attribute whose value is taken from an attribute of the
XML row element.

Most tables will have an ID column that was defined with **AUTO_INCREMENT**
and **PRIMARY KEY** instructions to ensure unique ID values.  The framework
normally detects the primary key field and uses that value for the *data-id*
attribute without any explicit instructions.

However, tables that use **GROUP_CONCAT** fields necessarily also use the
**GROUP BY** query instruction, and the *GROUP_BY* instruction hides the
*AUTO_INCREMENT* and *PRIMARY KEY* field attributes.  Even if the field has
been declared appropriately in the *CREATE TABLE* command, the *line_id*
instruction must be explicitly declared as shown in the *Solution* above.


