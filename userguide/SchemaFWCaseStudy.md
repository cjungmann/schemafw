# A SchemaFW Case Study

The Schema Framework makes it easy to develop a basic CRUD (**C**reate, **R**ead,
**U**pdate, and **D**elete) application, and not much more difficult to extend
the basics.

## Case Study Preview

### Simplest Case Study

This case study starts with an introduction to building a simple CRUD web
application.  It will create a simple table called **ContactList** of people with
phone numbers.  The initial application will simply create, read, update and
delete records in this table.

### Adding Advanced Features

We will add case study features like import/export and authenticated access that
can be difficult and time-consuming to add otherwise.  After adding these features,
we'll go on to deriving a calendar display from the basic table, then adding form
widgets to handle restricted input, and client-side table joins.

## Download Case Study Files

The case study files are available as a [repository](https://github.com/cjungmann/sfw_casestudy).
The repository contains the full case study, including all the enhancements that we cover
here.

In these examples, the SQL files that add the tables and procedures are in a *setup*
directory.  The initial tables and procedures are found in *setup/tables_contacts.sql*
and *setup/procs_contacts.sql*.  The repository also includes several other SQL script
files that support advanced features that will be covered later.

The repository also includes a *site* directory from which Apache runs the application.
We will add to this directory the SRM script files that run the framework's interactions.
All of the *modes* that run the initial case study are found in *site/contacts.srm*

For each type of interaction, we will look at SQL code that is found in
*setup/procs_contacts.sql* and SRM code that is found in *site/contacts.srm*.

## Case Study Foundation 

To start, we will create a simple table of contacts:

~~~sql
-- setup/tables_contacts.sql

SET storage_engine=InnoDB;
CREATE TABLE IF NOT EXISTS ContactList
(
   id    INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
   fname VARCHAR(32),
   lname VARCHAR(32),
   phone VARCHAR(25)
);
~~~


The final section contains continguous listings of the MySQL table definitions,
the procedures code, and the SRM file.  Use these listings as boilerplate for
new applications.

- [A Read Interaction](CSReadInteraction.md)
- [A List Interaction](CSListInteraction.md)
- [A Create Interaction](CSCreateInteraction.md)
- [An Update Interaction](CSUpdateInteraction.md)
- [A Delete Interaction](CSDeleteInteraction.md)

Go to [L-CRUD Files Listings](LCRUDInteractions.md) to see the entire application.




