# A SchemaFW Case Study

The Schema Framework makes it easy to develop a basic CRUD (**C**reate, **R**ead,
**U**pdate, and **D**elete) application, and not much more difficult to extend
the basics.

This case study is intended to introduce you to how the CRUD interactions are
prepared.  The interactions will be presented in order of increasing dependency,
with the first **r**ead interaction being almost completely self-contained,
and the following interactions making provisions for actions available in the
context of the interaction.

The interactions will work with a single table, _ContactList_.  The table
definition for _ContactList_ is as follows:

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


The final section contains continguous listings of the MySQL table definitions,
the procedures code, and the SRM file.  Use these listings as boilerplate for
new applications.

- [A Read Interaction](CSReadInteraction.md)
- [A List Interaction](CSListInteraction.md)
- [A Create Interaction](CSCreateInteraction.md)
- [An Update Interaction](CSUpdateInteraction.md)
- [A Delete Interaction](CSDeleteInteraction.md)

Go to [L-CRUD Files Listings](LCRUDInteractions.md) to see the entire application.




