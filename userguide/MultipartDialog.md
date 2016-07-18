# Multipart Dialog

## Introduction

A multipart dialog is a dialog that combines the results of
multiple queries into a single display object.  For example,
we will be looking at a dialog that shows a person and a list
of phone numbers.

## Design Goal

Using [The Common Data Definition](Examples_Common_Data_Def.html) as our
starting point, we will build a _dashboard_-type dialog with two editable
sections, the person's name and the person's phone number list.

We will create a stored procedure that contains two result queries
(ie SELECT queries that create result branches in a resultset), and
show the client-side setup required to render and operate the
resulting dialog.

## Server-Side

### The Dialog Query

~~~sql
CREATE PROCEDURE App_Person_Dashboard(id INT UNSIGNED)
BEGIN
   SELECT p.fname, p.lname
     FROM Person p
    WHERE p.id = id;

  SELECT p.id, p.phone, p.sms
    FROM Person2Phone p2p
     
END $$
~~~
