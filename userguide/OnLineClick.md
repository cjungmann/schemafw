# Making Sense of *on_line_click*

When a user clicks on a page element that represents a data item, the
expectation is that the user will be given an opportunity to somehow
interact with the indicated data item.

The most recognizable example of this is a data table where clicking
on a row will open a dialog to edit the properties of the row.

In the SchemaFW, when a user clicks on a table row, the framework looks
for an *on_line_click* instruction to find a template for the URL to be used
to retrieve the data for the interaction.

This is a useage case for [Context References](ContextReferences.md).

The principles discussed here also apply to the *on_day_click* instruction
in a calendar interaction.

## Two *on_line_click* Forms

There are two forms of *on_line_click*, simple and composite.  The simple form
consists of the script name and a mode name (which can be omitted if the desired
mode is the default mode), and the record ID will be automatically appended to
complete the URL.  The second form, the _composite_ form, must specify all data
arguments.  It will not append the record ID, but will resolve context references
to send the arguments.

### Simple *on_line_click* Format

The [List Interaction](CSListInteraction.md) shows how to construct a table
interaction.  Note how the *on_line_click* instruction is used to direct the
response to a click on a table line.

The example shows a simple *on_line_click* instruction that consists of a
script file name and a mode name.  The framework appends the line ID of the
indicated table line to complete the URL.

### Composite *on_line_click* Format

The *on_line_click* value, like many other text values in an SRM file, will
resolve [context references](ContextReferences.md).  The **{$_name_}** and
**{@_name_}** notation pulls values from variable results and form data results,
respectively.  These references are resolved when the page is built.  Another
class of context references is available for table-like interactions.  The
**{!_name_}** reference pulls values from the attributes of the XML element
that built the table line.

Consider the following SchemaFW document.  In particular, note the simple format
 of the *on_line_click* attribute of the outer resultset element:

~~~xml
<?xml version="1.0" ?>
<?xml-stylesheet type="text/xsl" href="default.xsl" ?>
<resultset method="POST"
           mode-type="table"
           on_line_click="contacts.srm?edit"
           title="List of Contacts">
<buttons>
<button type="add" label="Create Contact" task="contacts.srm?create" />
</buttons>
<result rndx="1" row-name="row">
<schema name="row">
<field name="id" type="INT" primary-key="true"
       auto-increment="true" sort="number"
       not-null="true" length="4" />
<field name="fname" type="VARCHAR" length="32" />
<field name="lname" type="VARCHAR" length="32" />
<field name="phone" type="VARCHAR" length="25" />
</schema>
<row id="1" fname="Tony" lname="Start" phone="800-555-1212" />
<row id="2" fname="Pepper" lname="Potts" phone="800-555-1313" />
<row id="3" fname="Nick" lname="Fury" phone="800-555-1414" />
</result>
</resultset>
~~~

Let's assume that people have been misusing the database by accessing contact
records with unqualified id numbers.  In order to prevent this from happening,
we might want to require two data items to confirm the contact.  We would
rewrite the stored procedure to have two parameters, the record ID and the
contact's first name.  In that case, the *on_line_click* instruction would
change to:

~~~
on_line_click : contacts.srm?edit&id={!id}&fname={!fname}
~~~

then, before the URL is submitted to the server, the framework will convert
the *on_line_click* value to

~~~
contacts.srm?edit&id=2&fname=Pepper
~~~


