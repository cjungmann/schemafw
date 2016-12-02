# L-CRUD Files Listings

The application will consist of
- One MySQL table
- Five MySQL stored procedures (one each letter of L-CRUD)
- Several Schema Response Modes

## Prerequisites

This guide assumes that the server has been prepared to host the site.
See [Establish a SchemaFW Site](CreateNewSite.md) for a list of steps
to prepare the site.


## Case Study Data Definition

Our sample project is a simplistic contact list that associates a single phone
number to a name.  The database consists of a single table and a few stored
procedures that will perform the L-CRUD interactions.

### Creating MySQL Tables and Procedures

In [Create New Site](CreateNewSite.md), the example database name is
_CaseStudy_.  That is what we'll use here, as well.

#### Create the Script Files

.../setup/tables.sql
~~~sql
SET storage_engine=InnoDB;
CREATE TABLE IF NOT EXISTS ContactList
(
   id    INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
   fname VARCHAR(32),
   lname VARCHAR(32),
   phone VARCHAR(25)
);
~~~

The following stored procedures will be described in the content above, but are
included here to provide a complete view of the MySQL side of the application.

.../setup/procs.sql
~~~sql
DELIMITER $$

-- "L" part of L-CRUD
DROP PROCEDURE IF EXISTS App_Contact_List $$
CREATE PROCEDURE App_Contact_List(id INT UNSIGNED)
BEGIN
   SELECT c.id, c.fname, c.lname, c.phone
     FROM ContactList c
    WHERE id IS NULL OR c.id=id;
END $$

-- "C" part of L-CRUD
DROP PROCEDURE IF EXISTS App_Contact_Create $$
CREATE PROCEDURE App_Contact_Create(fname VARCHAR(32),
                                    lname VARCHAR(32),
                                    phone VARCHAR(25))
BEGIN
   DECLARE new_id INT UNSIGNED;
   
   INSERT
     INTO ContactList (fname, lname, phone)
   VALUES (fname, lname, phone);

   IF ROW_COUNT() > 0 THEN
      SET new_id = LAST_INSERT_ID();
      CALL App_Contact_List(new_id);
   END IF;
END $$

-- "R" part of L-CRUD
DROP PROCEDURE IF EXISTS App_Contact_Value $$
CREATE PROCEDURE App_Contact_Value(id INT UNSIGNED)
BEGIN
   SELECT c.id, c.fname, c.lname, c.phone
     FROM ContactList c
    WHERE c.id = id;
END $$

-- "U" part of L-CRUD
DROP PROCEDURE IF EXISTS App_Contact_Update $$
CREATE PROCEDURE App_Contact_Update(id INT UNSIGNED,
                                    fname VARCHAR(32),
                                    lname VARCHAR(32),
                                    phone VARCHAR(25))
BEGIN
   UPDATE ContactList c
      SET c.fname = fname,
          c.lname = lname,
          c.phone = phone
    WHERE c.id = id;

   IF ROW_COUNT() > 0 THEN
      CALL App_Contact_List(id);
   END IF;
END $$

-- "D" part of L-CRUD
DROP PROCEDURE IF EXISTS App_Contact_Delete $$
CREATE PROCEDURE App_Contact_Delete(id INT UNSIGNED, fname VARCHAR(32))
BEGIN
   DELETE
     FROM c USING ContactList AS c
    WHERE c.id = id and c.fname = fname;

   SELECT ROW_COUNT() AS deleted;
END $$

DELIMITER ;
~~~

#### Run the Script Files

Now we can run the scripts to create the tables and procedures.  The first set
of commands should be typed in by hand to set environment variables for parameters
in the command that _should_ be pasted into a console terminal.

Enter these by hand to set your unique values:
~~~
MyUser=root
MyPassword=MyRootPassword
MyDatabase=CaseStudy
~~~

You can type, or copy and paste these commands to run the scripts.  The commands
assume that you are in the _setup_ directory, and that you have made two files,
_tables.sql_ and _procs.sql_ with pasted code from above.  Make adjustments
according to your changes, if any.

~~~
mysql -u ${MyUser} -p${MyPassword} ${MyDatabase} < tables.sql
mysql -u ${MyUser} -p${MyPassword} ${MyDatabase} < procs.sql
~~~

## Case Study SRM File

Go to the _site_ directory and paste the following code into a new file,
_contacts.srm_.

.../site/contacts.srm
~~~srm
$database       : CaseStudy
$xml-stylesheet : default.xsl
$default-mode   : list

list
   type          : table
   procedure     : App_Contact_List
   on_line_click : contacts.srm?edit
   button
      type  : add
      label : Create Contact
      task  : contacts.srm?create

create
   type        : form-new
   schema-proc : App_Contact_Create
   form-action : contacts.srm?create_submit

create_submit
   type       : form-submit
   procedure  : App_Contact_Create
   result
      type : update


# Not used except as initial example;      
read
   type      : form-view
   procedure : App_Contact_Read

edit
   type        : form-edit
   schema-proc : App_Contact_Update
   procedure   : App_Contact_Value
   form-action : contacts.srm?edit_submit
   schema
      button
         type    : delete
         label   : Delete
         confirm : Delete {@fname} {@lname} from contacts?
         task    : contacts.srm?delete&id={@id}&fname={@fname}

edit_submit
   type      : form-submit
   procedure : App_Contact_Update
   result
      type : update

delete
   type      : delete
   procedure : App_Contact_Delete
~~~

## Test the Site

If set up on the local machine as suggested in [Create New Site](CreateNewSite.md),
you can test the site with the following URL:
~~~
google-chrome http://localhost/cs
~~~


## Links

[Introduction to SchemaFW](IntroductionToSchemaFW.md)

[Main Page](UserGuide.md)

