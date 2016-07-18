# Case Study List Interaction

This is the second-easiest interaction.  The stored procedure for this type serves
two purposes:

1. to return the entire list for the initial rendering of the list
2. to return a single row to add to a displayed list after a record has been
   created or updated.  The procedure is called in this role by the **U**pdate
   and **C**reate procedures.

## MySQL Stored Procedure   

This design is strongly recommended because it helps ensure that the contents of
updated/created rows are in the same format as the displayed list.  Pay attention
to the `WHERE` clause in the procedure: 

~~~sql
-- "L" of L-CRUD
DELIMITER $$
CREATE PROCEDURE App_Contact_List(id INT UNSIGNED)
BEGIN
   SELECT c.id, c.fname, c.lname, c.phone
     FROM ContactList c
    WHERE id IS NULL OR c.id = id;
END $$
~~~

## Response Mode Instructions

~~~srm
# Excepted from contacts.srm
list
   type          : table
   procedure     : App_Contact_List
   on_line_click : contacts.srm?edit
   button
      type  : add
      label : Create Contact
      task  : contacts.srm?add
~~~

This response mode includes two extra instructions,

1. **on_line_click**: URL to call when a table row is clicked.
   Idiomatically, this should trigger an update interaction, presenting
   a form prefilled with the content of the record represented by the
   table row.  See [Update Interaction](CSUpdateInteraction.md).

2. An add-type button labelled **Create Contact** that, when clicked, will call
   the URL in the _task_ instruction.  See
   [Create Interaction](CSCreateInteraction.md).

## Client-side Use

When the client receives the response from SchemaFW, it will render a table
because of the `type : table` instruction.  All table responses will include a
schema with the result data to help format the table cells.

The following things will occur when SchemaFW renders the table on the client:

1. SchemaFW will use the schema to try to determine an appropriate field to
   use as a record identifier. The first choice would be a field that contains
   the _primary-key_ and/or the _auto-increment_ attributes.  Ideally, all
   table views will include an _id_ field with those attributes.

2. The client framework will create a table with one _tr_ element for each
   data element in the result.

3. When rendering the _tr_ elements, the framework will take the value of the
   data element's attribute as identified in step 1 above.

When a user clicks on a table row, it looks for the URL defined by _on_line_click_,
appends to the URL the value of the _data-id_ attribute of the _tr_ element,
then sends that request to the SchemaFW server to run the named mode of the
SRM file.

You won't see this URL, but if you clicked on the table row of the first-created
contact record, the URL would look like this:

    contacts.src?edit=1

The response mode name, followed by an _=1_ will be passed to the first parameter
of the procedure named in the _edit_ response mode.  This shortcut allows the
interaction to proceed without knowing the name of the parameter.



## Links

[L-CRUD Interactions](LCRUDInteractions.md)

[Introduction to SchemaFW](IntroductionToSchemaFW.md)

[Main Page](UserGuide.md)

