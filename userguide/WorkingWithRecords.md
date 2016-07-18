# Working With Items

## Introduction

An item transaction is a transaction whose conceptual target is a
single thing.  An item transaction may update, add, or delete many
associated records during its execution.

## What is an Item?

An item is a single thing.  An item may be _atomic_ or _composite_.

An __atomic__ item stands alone, and corresponds to a database record.

Examples of atomic items include:
- A phone number
- An ingredient of a recipe
- A widget in a warehouse

A __composite__ item is an item with related items or groups.  A
composite item is typically based on a single database record, but
displayed with records and tables that are related to the base item.

Examples of composite items include:
-A person with lists of phone numbers and email addresses
- A recipe with a list of ingredients
- A purchase order with a list of widgets and a shipping address

## Types of Item Interaction

There are four types of item interactions, which applies to both atomic and
composite items:

- Add an item
- Update an item
- Delete an item
- View an item

Of the four, the _add item_ and _update item_ interactions are the fundamental
to working with a database and will often be referred to as __transactions__
data exchange is two-way.   The _view item_ interaction typically does not
change anything, and a _delete item_ interaction is usually an option offered
within an _update item_ transaction.



## Working with Atomic Items

The default tool for working with atomic items is the familiar HTML
form.  The Schema Framework provides several tools to make working
with forms easy.

### An Item Transaction

An item transaction consists of two events, presenting the form and
accepting the user's information.  These two events correspond to two mode
entries in an SRM file.

### The Update Query

Typically, an atomic item is one out of group of items displayed in
a table.  That means that any transaction that changes an item should
also update the table from which the item had been selected.  For
_add_, _update_, and _delete_ transactions, the MySQL procedure should
include an update query whose result exactly matches the format of
a row in the source table.  See [The _Add_ Transaction](#the-add-transaction)
for a simple example.


### The _Add_ Transaction

As usual, this transaction has a database part and an SRM file part.

The add transaction is the simplest transaction because it is not required
to display existing information, it simply presents an empty form.  The example
below shows the simple case.  For _add item_ transaction that need prefilled
fields, see [a context reference example](ContextReferences.md#a-simple-example).

~~~sql
CREATE PROCEDURE App_Add_Person(fname VARCHAR(25), lname VARCHAR(30))
BEGIN
   # Unlike other queries, the field names can be the same as the
   # parameter names because the intention is clear from the query
   # syntax.
   INSERT
     INTO Person (fname, lname)
     VALUES (fname, lname);

   # If the INSERT query was successful, ROW_COUNT() will equal 1.
   # Send the new row back to the client, with its new _id_ value.
   IF ROW_COUNT() > 0 THEN
      # Use the same query to get a new row as was used to build the
      # table in which will be inserted.
      CALL App_Person_List(LAST_INSERT_ID());
   END IF;
END $$
~~~

~~~srm
# default.srm
app_person
   procedure : App_Add_Person
   schema
      field : fname
         label : First Name
      field : lname
         label : Last Name
         
   # This result corresponds to the `CALL App_Person_List(LAST_INSERT_ID())`
   # in the query, and will be added to the table if returned.
   result
      type : update
~~~




