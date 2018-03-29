# Case Study Read Interaction

A read interaction is the simplest interaction.  It is used to display information
and to fill form fields when editing records.  The read interaction will be revisited
with the **U**pdate interaction.

The MySQL stored procedure *App_Contact_Value* will be used as the **r**ead procedure.
We will use _Value_ instead of _Name_ because the procedure will be more commonly used
later in a different context where _Value_ more accurately describes its purpose.

~~~sql
-- "R" of L-CRUD
DELIMITER $$
CREATE PROCEDURE App_Contact_Value(id INT UNSIGNED)
BEGIN
   SELECT c.id, c.fname, c.lname, c.phone
     FROM ContactList c
    WHERE c.id = id;
END $$
~~~

is paired with the following SRM mode:

~~~srm
# Excepted from contacts.srm
read
   type      : form-view
   procedure : App_Contact_Value
~~~

for the simplest use of SchemaFW.  The **R**ead interaction will be called with

    http://clist.com?read.srm&id=1

or more simply;

    http://clist.com?read.srm=1

The output will show all four fields in a stack with the field name in the
left column and the value in the right column.  Since there is no form-action
defined, the dialog will show a **Close** button at the bottom to close 

## Links

[L-CRUD Interactions](LCRUDInteractions.md)

[Introduction to SchemaFW](IntroductionToSchemaFW.md)

[Main Page](UserGuide.md)

