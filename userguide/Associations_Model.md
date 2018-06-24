# Associations Example Data Model

## Example Data

When modelling data, a table consists of a collection of data records.  A data record
will consist of fields that save property attributes of the object the data record
represents.  Some attributes of an object should saved with a primary record, and
others should be referenced indirectly.

The purpose of this example is to illustrate when and how to create one-to-many or
many-to-many relationships in a database.  

### Example Item Tables

In this very simple model, we'll track three of things, Households, People, and
Phone numbers. Each household can consist of zero to many persons, and each
person can have zero to many phone numbers.  Additionally, phone numbers may also
apply to multiple people, where the members of a household might share a land-line
while also individually using personal cell-phones.

The object table definitions are:

~~~sql
CREATE TABLE Household
(
   id   INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
   name VARCHAR(40)
);

CREATE TABLE Person
(
   id       INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
   fname    VARCHAR(20),
   lname    VARCHAR(40),
   birthday DATE,
   INDEX (birthday)
);

CREATE TABLE Phone
(
   id    INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
   phone VARCHAR(20)  
);
~~~

### Example Item Association Tables

When associating one object to another, the connection can be direct or indirect.

A direct connection is made when the primary record includes fields that point to
an instance of another object.  For example, if we knew that each household has
exactly one phone number, it might make sense to include a field with that phone
number in the household record.  Likewise, if we assumed that all households had
five or fewer members, there might be five person fields to record the relationships.

An indirect connection is made when two tables are connected by a third table that
maps the relationships.  In our household example, this could be a Household2Person
table.

~~~sql
CREATE TABLE Household2Person
(
   id_household INT UNSIGNED NOT NULL,
   id_person    INT UNSIGNED NOT NULL,
   INDEX(id_household),
   INDEX(id_person)
)

CREATE TABLE Person2Phone
(
   id_person INT UNSIGNED NOT NULL,
   id_phone  INT UNSIGNED NOT NULL,
   INDEX(id_person),
   INDEX(id_phone) 
);
~~~

back: [Introduction](Associations_Intro.md)
next: [Using Associations](Associations_Connecting.md)