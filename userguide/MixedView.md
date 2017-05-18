# Mixed-View Interaction

## Introduction

A mixed-view interaction combines several sections of related data into a
single form.  The sections are modified indirectly, that is, by
opening a new context to make changes.  This guide will attempt to
demonstrate the basic design with a simple example.

## Design Goal

Using [The Advanced Case Study Definition](AdvancedCaseStudyDef.md) as our
starting point, we will build a _dashboard_-type form with two editable
sections, the person's name and the person's phone number list.

Preparation of the page will involve writing several stored procedures
and several SRM response modes (in one or more SRM files).  For this
example, standard built-ins will render and operate the results, but
the guide will conclude with an example of creating a custom section
display.  

## The Parts

### The Mixed-View Response Mode

Unlike other examples, we will start here with the SRM response mode
to show the framework that the queries must fill in.

This interaction is more complicated than other types, with much more
setup required, more parts, and certain required instructions that can
be ignored in other interaction types.

The _mixed view_ response mode must identify its type as _mixed-view_


#### The Directing Schema

1. The schema in the first result is the director of the mixed-view.
   There is no allowance for discovering a mixed-view in other results.
2. At least one field must be a **view field** that includes a _result_
   instruction to identify the named result that will be used to construct
   the subview.
3. Fields without a _result_ instruction will be displayed as a read-only
   form for information.  The contents of the fields can be edited only
   by using a modal edit form.

#### The View Field
1. A **view field** is indicated by a **result** instruction.
2. The result instruction must refer to a named result within the resultset.
3. When constructing the mixed-view, the framework will create a separate
   subview for each view field in the directing schema.
4. The **type** instruction will by the attribute that allows XSL to
   select the appropriate template for constructing the subview.
5. The framework will include an edit button when the view field includes
   a **manage** instruction.  Pressing this button will open a modal
   interaction with the URL value of the _manage_ instruction.
6. The **update** instruction provides a URL that can be called to
   refresh the contents of the subview with the current state.  This
   instruction will typically be included in any view field that also
   includes a _manage_ instruction and will be automatically used
   upon return from the modal interaction.

~~~srm
person_view
   type      : mixed-view       # required type value to trigger mixed-view
   procedure : App_Person_View
   result : 1
      schema
         field : fname
            label : First Name
         field : lname
            label : Last Name
         field : phones
            label    : Phone List
            result   : phonelist
            type     : table
            manage   : person.srm?manage_person_phones
            update   : person.srm?phones_section_refresh
            
   # Make sure the name of the result matches the value of the result instruction.
   # Note that the required *schema* instruction.
   result : 2
      name : phonelist
      schema

# Supporting response modes:

# A table mode with the usual support modes for creating, updating, deleting records.
manage_person_phones   # support URL that follows result instruction
   type          : table
   procedure     : App_Person_Phones_Table
   on_line_click : manage_person_phones_open

# Response mode to replacing the phonelist result if the table mode makes changes:
phone_section_refresh
   procedure     : App_Person_Phones_Summary
   schema
      name : phonelist
~~~

### The MySql Procedures

The mixed-view procedure will include multiple results, with placeholder
fields to which the view fields are attached.  The example should make this
clear.

It is a good practice to CALL procedures to build the subviews.  The SQL code
can then be shared with both the main constructor and the refresh responses
to help maintain consistency.

~~~sql
CREATE PROCEDURE App_Person_View(person_id INT UNSIGNED)
BEGIN
   SELECT p.fname,
          p.lname,
          NULL AS phone_list  -- The placeholder field
     FROM Person p
    WHERE p.id = person_id;

    -- The query that creates the result that the placeholder will reference:
    CALL App_Person_Phone_List(person_id);
END $$
~~~

The following procedures must be defined before the previous procedure will
compile.  Conceptually, however, they follow the mixed view procedure because
they support it.

~~~sql
CREATE PROCEDURE App_Person_Phone_List(person_id INT UNSIGNED)
BEGIN
   SELECT p.id, p.phone, p.sms
     FROM Phone2Person p2p
          INNER JOIN Phone p ON p.id = p2p.id_phone
    WHERE p2p.id_person = person_id;
END $$
~~~

In this example, the edit button of the subview will open a table view of
the phones associated with the person.  Simply implement a normal set of
table queries and response modes to allow changes to the phone list.

