# Exporting Data

Exporting data from SchemaFW is a very simple matter of calling a stored procedure
that writes out a table.

For this guide, we will add an export feature to the basic Case Study application
detailed in the previous pages.

## Setup

There are three main steps for making an export feature available,

- A stored procedure
- A response mode that calls the procedure
- Some link to advertise and provide access to the export response mode.

### The Stored Procedure

~~~sql
CREATE PROCEDURE App_Contacts_Export()
BEGIN
   SELECT id, fname, lname, phone
     FROM ContactList;
END $$
~~~

The basic idea of the export procedure is very simple.  Accommodations for session
confirmation or subsets of the data might complicate the procedure in specific
applications.

### The SRM Response Mode

Add the following response mode to the _contacts.srm_ file.

~~~srm
export_contacts
   procedure : App_Contacts_Export
   type      : export
~~~

### Show Link

Unlike the previous steps, this will be a modification of an existing response mode.
We will add a button to the [list view](CSListInteraction.md).  Find the _list_
response mode in the _contacts.srm_ file.

~~~srm
list
   type          : table
   procedure     : App_Contact_List
   on_line_click : contacts.srm?edit
   button
      type  : add
      label : Create Contact
      task  : contacts.srm?create
   button
      type  : 
~~~