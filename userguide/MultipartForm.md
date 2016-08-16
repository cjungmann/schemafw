# Multipart Form

## Introduction

A multipart form combines several sections of related data into a
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

### The Multipart Form Response Mode

Unlike other examples, we will start here with the SRM response mode
to show the framework that the queries must fill in.

~~~srm
person_view
   type      : form-view
   procedure : App_Person_View
   result
      schema
         field : fullname
            label    : Full Name
            readOnly : true
         field : phones
            label    : Phone List
            type     : block
            
            # name of result to be integrated into the form:
            result   : phonelist
            # URL with which the section will be edited:
            manage   : person.srm?manage_person_phones
            # URL to call to replot the section after changes made:
            update   : person.srm?phones_section_refresh
            
   # Make sure the name of the result matches the value of the result instruction:
   result
      name : phonelist

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

~~~sql
CREATE PROCEDURE App_Person_Phones_Summary(person_id INT UNSIGNED)
BEGIN
   SELECT 
END $$
~~~



Compared with a standard form, several additional queries are required
to run a multipart form.  
## Server-Side

### The Form Query

~~~sql
CREATE PROCEDURE App_Person_Phone_List(id INT UNSIGNED)
BEGIN
   SELECT 
END $$

CREATE PROCEDURE App_Person_Dashboard(id INT UNSIGNED)
BEGIN
   SELECT p.fname, p.lname
     FROM Person p
    WHERE p.id = id;

  SELECT p.id, p.phone, p.sms
    FROM Person2Phone p2p
     
END $$
~~~
