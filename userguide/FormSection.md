# Form Section

A form section is a subsection of a multipart form.  The section is
not editable within the form, but rather it presents a button to
open an editing context.

## Example

The typical case is a list of emails associated with a person.  A
person's view might be a multipart form with an email section.  The
email section might list all the emails associated with the person,
and a button in the section would open a table view of the associated
emails with the typical CRUD options.

## SQL Requirements

To handle all the tasks for a table-view section, the following
procedures should be defined.  The italicized name at the end of each
task identifies the name of the procedure that handles the task.
The example code listing with these procedures is found at the end
of this page.

1. **Summary** procedure to display the section contents in the
   multipart form.  This procedure should be called in by the
   procedure that displays the full form, so the output will be
   consistent. *App_Person_Email_Summary*
2. **Context** procedure to make changes to the data presented
   in the summary section.  Typically, this will be a table
   view with an _add_ button.  *App_Person_Email_Table*
3. **Value** procedure for editing one line of the _context view_.
   This procedure fills the form fields when editing a row, so its
   fields should match the input fields of the procedure that saves
   the update.  *App_Person_Email_Value*
4. **Create** procedure to add a new row. *App_Person_Email_Summary*
5. **Update** procedure to save changes made to en edited row.
   *App_Person_Email_Update*
6. **Delete** procedure for discarding a row.  Some models may not
   need a _delete_ procedure if deleting records should not be allowed.
   *App_Person_Email_Delete*

See the bottom of this page for a code listing of the above functions.

## Multipart Setup

There are several unique characteristics of a Multipart form, some
in the MySQL code, and two main differences in the SRM response mode.

### MySQL for Multipart Form

The procedure that builds the multipart form is unusual in that it
must leave NULL placeholders for secondary results.  This is easiest
explained by example:

~~~sql
CREATE PROCEDURE App_Person_View()
BEGIN
   SELECT CONCAT(fname, ' ', lname) AS pname,
          NULL AS email_list
     FROM Person
    WHERE id = @person_id;

   -- This result will be given a name in the SRM (not necessarily email_list)
      and will replace the NULL email_list field in the primary query.
   CALL App_Person_Email_Summary();
END $$
~~~

Please notice three things in the above code excerpt.

1. The named NULL value is a placeholder for the schema to attach another
   query's results.
2. The result of the second query will be named for the NULL placeholder.
3. Less obvious, but it is recommended to use a discrete call to a procedure
   that will also be used to update the field upon changes to its content.
   This ensures consistency.

### SRM for the Multipart Form

The multipart form requires more setup that other basic interactions.  Since
the section field is a portal to another editing screen, it needs to know that
the alternate editing method is required, how to access it, and how to display
changes.  These are all described in the more complicated field instruction
in the SRM response mode.  Again, an example is the best starting point.

~~~srm
# excerpt of person.srm

home
   type      : form-view
   procedure : App_Person_View
   result : 1
      schema
         field : pname
            label : Name
         field : email_list
            label  : Emails
            type   : block
            result : elist  # matches name of second result
            manage : person.srm?emails_manage
            update : person.srm?emails_refresh
   result : 2
      name : elist          # matches result instruction of email_list field

emails_manage
   type      : table
   procedure : App_Person_Email_Table
   result : 1
      schema
         title         : Manage Emails for {$pname}
         on_line_click : person.srm?person_email_edit
         button
            type  : add
            label : Add Email
            task  : person.srm?add_email
   # Make a variables result to provide info for {$pname} in title.
   result : 2
      type : variables

emails_update
   type      : update
   procedure : App_Person_Email_Summary
   name      : elist

# Omitted from this excerpt are several response modes that help run the
# table-view of the _emails_manage_ response mode.  While the MySQL is a
# little different, the supporting response modes are identical to those
# of any table-view.  The following empty modes list the missing response
# modes:

add_email
edit_email
submit_email
delete_email
~~~


## MySQL Code Listing

The following code excerpt includes prototypes of the procedures
described above.  It is assumed that these procedures will be called
from a session-enabled application, with the person ID needed to filter
the email list is a session variable set when the session is restored.
Because of this, a person ID parameter is not included in the parameter
list of any of the following procedures.

~~~sql
CREATE PROCEDURE App_Person_Email_Summary()
BEGIN
   SELECT email
     FROM Email
    WHERE id_person = @person_id;
END $$

CREATE PROCEDURE App_Person_Email_Table(id INT UNSIGNED)
BEGIN
   SELECT e.id, e.email
     FROM Email e
    WHERE (id IS NULL OR e.id = id)
      AND e.id_person = @person_id;

   -- Include a variables result to provide {$pname} field for the page title:
   SELECT CONCAT(fname,' ',lname) AS pname
     FROM Person
    WHERE id = @person_id;
END $$

CREATE PROCEDURE App_Person_Email_Value(id INT UNSIGNED)
BEGIN
   SELECT e.id, e.email
     FROM Email e
    WHERE e.id = id
      AND e.id_person = @person_id;
END $$

CREATE PROCEDURE App_Person_Email_Update(id INT UNSIGNED,
                                         email VARCHAR(128))
BEGIN
   UPDATE Email e
      SET e.email = email
    WHERE e.id = id
      AND e.id_person = @person_id;

   IF ROW_COUNT() > 0 THEN
      CALL App_Person_Email_Table(id);
   END IF;
END $$

CREATE PROCEDURE App_Person_Email_Add(email VARCHAR(128))
BEGIN
   INSERT
     INTO Email (id_person, email)
          VALUES(@person_id, email);

   IF ROW_COUNT() > 0 THEN
      CALL App_Person_Email_Table(LAST_INSERT_ROW());
   END IF;
END $$

-- Including the email value to prevent accidental or malicious
-- deleting of a row.  Note the MySQL-specific form of aliasing
-- the target table in DELETE.
CREATE PROCEDURE App_Person_Email_Delete(id INT UNSIGNED,
                                         email VARCHAR(128))
BEGIN
   DELETE
     FROM e USING Email AS e
    WHERE e.id = id
      AND e.id_person = @person_id
      AND e.email = email;

   -- Special, simple query to indicate DELETE success or failure:
   SELECT ROW_COUNT() AS deleted;
END $$
~~~

