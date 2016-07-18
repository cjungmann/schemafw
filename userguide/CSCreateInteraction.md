# Case Study Create Interaction

This interaction is meant to be called as a response to clicking on an **ADD**
button.

This interaction will create a new record and, if successful, will add a
line to the associated table.  It is a little more complicated than our
previous interactions because it requires two response modes, one for
generating the form, and the other to accept the form's data and to make
an INSERT query to create the new record.

## The MySQL stored procedure

~~~sql
-- "C" of L-CRUD
DELIMITER $$
CREATE PROCEDURE App_Contact_Create(fname VARCHAR(32),
                                    lname VARCHAR(32),
                                    phone VARCHAR(10)
BEGIN
   INSERT
     INTO ContactList(fname, lname, phone)
   VALUES (fname, lname, phone);

   -- This secondary CALL uses a previously-defined procedure
   -- to return a new table row if a record was created, as
   -- indicated by the ROW_COUNT() function:
   IF ROW_COUNT() > 0 THEN
      CALL App_Contact_List(LAST_INSERT_ID();
   END IF;
END $$
~~~

## Response Modes

Dialog interactions like **C**reate and **U**pdate require two response modes,
a _form request_ mode  to display the form, and a _form submit_ mode to accept
the submitted values.

### Form Request Response Mode
~~~srm
# Excepted from contacts.srm
create
   type        : form-new
   schema-proc : App_Contact_Create
   form-action : contacts.srm?create_submit
~~~

This _form request_ response mode is unique among the typical response modes in that
it doesn't have a _procedure_ instruction.  The `schema-proc` collects the parameters
of the named procedure to create a schema for rendering the empty form.

In some applications, [context references](ContextReferences.md) can be used to
prefill some fields.  An example of this might be to prefill a field with a name
or id value that can be inferred from the context and would be annoying for the
end user to supply.

### Form Submit Response Mode
~~~srm
# Excepted from contacts.srm
create_submit
   type       : form-submit
   procedure  : App_Contact_Create
   result
      type : update
~~~

Conceptually, one would not access this interaction directly, but rather
through a SchemaFW application

## Links

[L-CRUD Interactions](LCRUDInteractions.md)

[Introduction to SchemaFW](IntroductionToSchemaFW.md)

[Main Page](UserGuide.md)
