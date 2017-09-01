# Troubleshooting SchemaFW

This guide helps explain how to fix various problems whose solution might
be not be readily apparent.

## Table with GROUP BY field is missing xrow_id

### Problem

This message is shown when a response mode uses a procedure that includes
 a **GROUP BY** instruction but does not identify a **xrow_id** instruction.

### Solution

Include a **xrow_id** instruction in the response mode:

~~~srm
main
   type          : table
   procedure     : App_Items_With_Keywords
   on_line_click : ?edit
   # Include a xrow_id instruction like this:
   schema
      field : id
         xrow_id : true
~~~

Note that the framework only checks if the *xrow_id* attribute exists, it does
not check the value.  That way the value can be *1*, or *true*, or *yes*, but
it also means that a value of *0*, or *false*, or *no* will also be interpreted
as a *xrow_id* field.

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
been declared appropriately in the *CREATE TABLE* command, the *xrow_id*
instruction must be explicitly declared as shown in the *Solution* above.

# The import query failed (Lost connection to MySQL server during query)

This error is shown if *ssconvert* has not been installed to aid with
importing.  Other errors may also trigger this error, so don't give up
if the following solution does not solve your problem.

## The Solution

Open a terminal window and install (www.gnumeric.org)[Gnumeric].

~~~sh
sudo apt-get install gnumeric
~~~

## Explanation

In otrder to allow data import, the Schema Framework uses a utility from
a third-party open-source application.  (www.gnumeric.org)[Gnumeric] was,
at the time of development, the easiest to install and use.  It can convert
several spreadsheet format to CSV, which is in turn is used to bulk load
the data in a Quarantine table.

See also, (Building the Framework)[BuildingTheFramework.md] and
(Importing Data)[ImportingData.md] for more information.
