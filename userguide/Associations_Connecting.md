# Using Associations

Given the relationships described in [Associations Example Data Model](Associations_Model.md),
this guide will illustrate how to use the Schema Framework tools to work with the data.

## Client-side Table Joins

Part of the design goal of associations in the Schema Framework is to reduce the server
load.  Using the association methods here will prepare the client-side framework to make
the table joins.

## Sample SRM File

~~~srm
.
.
page
   procedure : App_House_Page
   type      : table
   on_line_click : ?manage
   result
      name     : houses
      row-name : house
   result
      name     : people
      row-name : person
   result
      name     : phones
      row-name : number
   result  # person list
      name     : plist
      type     : association
      result   : people
   result  # (phone) number list
      name     : nlist
      type     : association
      result   : phones
   
~~~

## Sample Resultset

~~~xml
<resultset>
   <houses rndx="1" row-name="house">
      <house id="1" name="Smith" />
      <house id="2" name="Jones" />
      .
   </houses>
   <people rndx="2" row-name="person">
      <person id="1" fname="Robert" lname="Smith" birthday="1980-01-01" />
      <person id="2" fname="Sally" lname="Smith" birthday="1980-06-06" />
      <person id="3" fname="Roger" lname="Jones" birthday="1975-03-15" />
      <person id="4" fname="Thomas" lname="Jones" birthday="1997-09-10" />
      .
   </people>
   <phones rndx="3" row-name="number">
      <number id="1" val="5555551212" />
      <number id="2" val="5555552121" />
      .
   </phones>
   <plist rndx="4" row-name="row" result="people">
      <row id="1" plist="1,2" />
      <row id="2" plist="3,4" />
      .
   </plist>
   <nlist rndx="5" row-name="row" result="phones">
      <row id="1" nlist="1,2" />
   </nlist>
</resultset>
~~~


## MySQL Procedures

~~~sql
CREATE PROCEDURE App_Household_Create(name VARCHAR(40))
BEGIN
END $$

