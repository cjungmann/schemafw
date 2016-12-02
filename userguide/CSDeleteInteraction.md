# Case Study Delete Interaction

The *d*elete interaction requires some more advanced techniques.  These techniques
will be included in the samples with minimal explanation, but you will find links
to more complete coverage near their introduction.

The advanced techniques are necessary to provide useful confirmation dialogs before
executing an unrecoverable *d*elete interaction.

## MySQL Stored Procedure

Because the *d*elete interaction is unrecoverable, having a confirming parameter
in addition to the _id_ parameter can help prevent accidental or mischievious deletion.
In the following example, having to know the matching _fname_ value prevents someone
from successfully deleting a record a simple integer value.

Another safeguard against URL-only record deleting is to use single-factor record
identification (ie _id_), but to prevent unauthorized access through the use of
[Sessions and Authorization](SessionsAndAuthorization.md).

~~~sql
CREATE PROCEDURE App_Contact_Delete(id INT UNSIGNED, fname VARCHAR(32))
BEGIN
   DELETE
     FROM c USING ContactList AS c
    WHERE c.id = id and c.fname = fname;

   -- The standard client-side framework will recognise the following
   -- result and will remove the table row if the delete was successful.
   SELECT ROW_COUNT() AS deleted;
END $$
~~~

Note the use of _USING_ in the procedure code.  The MySQL _DELETE_ command does
not support the simple table alias syntax.  _USING_ provides the same function
as the simple table alias, allowing for consistency in procedure design with
respect to disambiguating the parameter names and table columns.

## Response Modes

### Extending the Form Request Mode from the *U*pdate Interaction

We will update the _form request_ response mode from
[Update Interaction](CSUpdateInteraction.md) to include instructions for
adding a _delete_ button.  The _delete_ button, as configured below, will
present a confirmation dialog before sending the delete request.

Note in particular the _confirm_ and _task_ instructions.  Both include
[context references](ContactReferences.md) that provide safety to the interaction.
In this case, the attribute (_@_) token means that the bracketed references will
be replaced with field values coming from the _App_Contact_Value_ procedure.
[Context References](ContextReferences.md) will describe other sources of information
and how to access them in response modes.

In the following response mode, the _task_ instruction is particularly important,
given our two-parameter delete procedure.  The _id_ value will be taken from the
_data-id_ attribute of the table row, and appended to the _=_ character of the URL.
The other parameter, _fname_, must be taken from the *u*pdate interaction context.

~~~srm
edit
   type        : form-edit
   schema-proc : App_Contact_Update
   procedure   : App_Contact_Value
   form-action : contacts.srm?edit_submit

   ## added instructions:
   schema
      button
         type    : delete
         label   : Delete
         confirm : Delete {@fname} {@lname} from contacts?
         task    : contacts.srm?delete&id={@id}&fname={@fname}
~~~

Note in the _task_ instruction that both the _id_ and _fname_ parameters are
pulled out of the values.  In table interactions, the _id_ value is taken from
the _data-id_ attribute of the _tr_ element, but that value isn't available
here.

### The Delete Response Mode

The confirmation setup is done in the _form request_ mode and the delete stored
procedure, so the *d*elete interaction response mode is short and simple:

Upon the successful execution of the procedure in the _delete_ response mode,
the software will read the _type_ value, set to _delete_ in this case, from the
returned file.  This will prompt the software to remove the indicated record
from the client-side representation of the data, to make it match the server
table contents.

~~~srm
delete
   type      : delete
   procedure : App_Contact_Delete
~~~

## Links

[L-CRUD Interactions](LCRUDInteractions.md)

[Introduction to SchemaFW](IntroductionToSchemaFW.md)

[Main Page](UserGuide.md)

