# Troubleshooting SchemaFW Messages

This guide helps explain how to fix various problems whose solution might
be not be readily apparent.  In order to facilitate debugging framework
applications, simple new messages will be added for easy-to-make mistakes,
and more complete explanations for each error will appear in this guide.

Currently, the following error messages are explained below:

- Unable to find a result element
- The update row name (xxx) does not match the result's row-name (XXX)
- Don't know what to do with the update row
- Table with GROUP BY field is missing xrow_id
- Failed to find a usable id value for row named "xxx"
- update_associations failed to find a data-id attribute for "*field_name*."
- The import query failed (Lost connection to MySQL server during query)
- Couldn't find attribute "deleted" in delete row.
- Call button type without url or task.

## Unable to find a result element

### Problem

The framework is unable to find the appropriate result element into which
an updated row should be inserted.

### Solution

Explicitly name the target result in the result instruction:

~~~srm
add_submit
   procedure : App_Item_Add
   result
      type   : update
      target : items
~~~

### Explanation

This problem generally occurs when adding a new element to a group because there
is no reference element whose parent node is the target result.  For simple table
or calendar pages, there is only one result, and it's easy to find it.  However,
for merged pages with multiple group results, the target result may not be obvious.
In these cases, the developer can help the framework identify the appropriate
result by explicitly naming the result, as show in the example in the Solution
above.

This error message will also be displayed if no result is found whose name matches
the  *target_name* value.


## The update row name (xxx) does not match the result's row-name (XXX)

### Problem

The new or updated row that is to be inserted into a result does not match
the rows that should be in the target result.

### Solution

Change the response modes in your page to agree on what a row is named.

### Explanation

The Schema Framework allows the developer to change the names of most elements
to application-specific names.  If a custom name is not specified, all data
elements in a result will be named *row*.

When a submitted [update](CSUpdateInteraction.md) or [add](CSCreateInteraction.md)
returns the new or updated row, it must match the name of the other elements in
the hosting result element.  That means if the developer has renamed the data
element in the multi-record result, the update row for the new or update interaction
must give the same name to the row inthe update element.

## Don't know what to do with the update row

### Problem

The function tbase::update_row() doesn't know how to handle the update request.
It _tbase::update_row() is only prepared to handle:
- a _cfobj_ with a **cmd** property
- a _cfobj_ with **mtype** set to **delete**, or
- a _cfobj_ with **rtype** set to **update**.

The error message is presented when none of the default actions are requested.

### Solution

There are two options:
- Change the result type of the response mode to _update_ for adding or updating
   ~~~srm
   add_submit
      procedure : App_Item_Add
      result
         type   : update
         target : items
   ~~~
- Change the response mode type to _delete_
   ~~~srm
   delete
      type      : delete
      procedure : App_Item_Delete
   ~~~
- Prepare a custom update_row() handler for your particular result type.

### Explanation

The function _tbase::update_row() is the *update_row* function that is automatically
used for table and calendar updates.  This default function should be enough for most
applications, and the problem will usually best fixed by preparing the response modes.

Consult the following guides for more reminders about setting up these interactions:

- [Adding Records](CSCreateInteraction.md)
- [Updating Records](CSUpdateInteraction.md)
- [Deleting Records](CSDeleteInteraction.md)


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

### Explanation

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

## Failed to find a usable id value for row named "xxx"

For a given XML data row, the framework was unable to determine the id value.

### The Solution

The framework has searched the schema fields to find one that indicates it is
an identity field.  Generally, this is a primary key field, but in the absence
of a primary key (in a GROUP_BY query, for example), a developer should mark
the identity field with an *xrow_id* instruction.

see *Table with GROUP BY field is missing xrow_id*

## update_associations failed to find a data-id attribute for "*field_name*."

This error occurs if a table table row (tr) element hosting a *assoc* cell (td)
does not include a *data-id* attribute.

### The Solution

If this error ever occurs, it will likely require an additional method for the
framework to retrieve the source XML element for the table row.  This is necessary
for the XSL stylesheet to find the updated data that should go in the cell.

## The import query failed (Lost connection to MySQL server during query)

This error is shown if *ssconvert* has not been installed to aid with
importing.  Other errors may also trigger this error, so don't give up
if the following solution does not solve your problem.

### The Solution

Open a terminal window and install (www.gnumeric.org)[Gnumeric].

~~~sh
sudo apt-get install gnumeric
~~~

### Explanation

In otrder to allow data import, the Schema Framework uses a utility from
a third-party open-source application.  (www.gnumeric.org)[Gnumeric] was,
at the time of development, the easiest to install and use.  It can convert
several spreadsheet format to CSV, which is in turn is used to bulk load
the data in a Quarantine table.

See also, (Building the Framework)[BuildingTheFramework.md] and
(Importing Data)[ImportingData.md] for more information.

## Couldn't Find Attribute "deleted" in Delete Row.

This message can be ambiguous.  It's designed for a delete operation and
should not occur in other cases.  Extra work on the framework will be needed
if this message occurs after a non-delete operation.

However, assuming a delete operation:

For the default framework, a delete procedure is expected to run a query
that returns the number of records deleted.  It should look like this (simplified):

~~~sql
CREATE PROCEDURE App_Delete_Person(id INT UNSIGNED)
BEGIN
   DELETE FROM p USING Person AS p
    WHERE p.id = id
      AND p.id_account = @session_confirmed_account;

   -- Note how the row's single attribute will be "deleted"
   SELECT ROW_COUNT() AS deleted;
END $$
~~~

If the procedure saves the ROW_COUNT() value for subsequent queries, make sure
the variable name in which the value is stored is *deleted* or that it is
renamed with *AS* in the *SELECT*.

## Call button type without url or task.

In addition to standard button types _open_, _close_, _jump_, etc, the **call**
button type calls a named function.  In general, a _call_ button instruction
should look like:

~~~srm
   button
      type : call
      label : Calendar Mode
      task  : switch_to_calendar_mode
~~~

Note that a _url_ instruction is treated as if it were a _task_ function.

The error message _Call button type without url or task_ is generated when
a _call_ button lacks _url_ or _task_.

See [Adding Buttons](AddingButtons.md) for information.