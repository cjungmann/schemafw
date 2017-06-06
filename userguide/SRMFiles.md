# SRM Files

SRM stands for **S**chema **R**esponse **M**ode, which describes the contents
of an SRM file.  An SRM file is a formatted text file that contains instructions
about how to interpret URLs by the _schema.fcgi_ server application.

## Format

An SRM file is a hierarchical document, with each line being either an
instruction or a node.  The relationship between lines is defined by the
relative level of indentation between the lines.  A line that is more
indented than its predecessor is interpreted to be a child, and lines
that match the indentation of a previous line is considered to be a sibling.

~~~
mode
   child1 : Mark
   child2 : Judy
      grandchild1 : Ellen
      grandchild2 : Ralph
   child3 :
~~~

## Include SRM Files

Use `$include` to centralize the setting of common values.  In the following
example, the file _common.srm_ contains some instructions that should be used
in every SRM file in the application.   The second (partial) file listing
has the `$include` instruction.  When _schema.fcgi_ reads a line starting with
`$include`, the line is replaced with contents of the referenced file.

common.srm:
~~~
$database       : CaseStudy
$xml-stylesheet : default.xsl
~~~

contacts.srm
~~~srm
$include      : common.srm
$default-mode : main
~~~

## Shared Modes

To allow a branch to be shared between multiple modes, make a named **$shared** node.
This example creates a shared schema branch named _day_table_schema_.

~~~srm
# For named share example below
$shared : schema_parts
   on_line_click : cal.srm?edit
   button
      type  : add
      label : Add Event
      task  : cal.srm?add
   button
      type  : close
      label : Close

# For sibling share example below

$shared : buttons
   button
      type  : add
      label : Add Event
      task  : cal.srm?add
   button
      type  : close
      label : Close
         
~~~

There are two ways to include a shared mode, either a **named share** or a
**sibling share**.

### Named Share

A named share adds a branch with the name, and the contents of the share are
descendents of the new branch.

Using the shares modes above, a named share is invoked like this:

~~~srm
day
   type       : table
   procedure  : App_Calendar_Day
   schema     : $schema_parts
~~~

with the following result:

~~~srm
day
   type       : table
   procedure  : App_Calendar_Day
   schema
      on_line_click : cal.srm?edit
      button
         type  : add
         label : Add Event
         task  : cal.srm?add
      button
         type  : close
         label : Close
~~~

The name of the named share is used as a branch node, with the contents of the
share as descendents of the branch.

### Sibling Share

~~~srm
day
   type   : table
   procedure : App_Calendar_Day
   siblings : $buttons
~~~

results in:

~~~srm
day
   type       : table
   procedure  : App_Calendar_Day
   button
      type  : add
      label : Add Event
      task  : cal.srm?add
   button
      type  : close
      label : Close
~~~

Notice that the contents of the share are included at the same indentation level as
the share invocation to includes it.

Use a named shares to add a section to a response mode, use a sibling share to 



--------------------------------------------------------------------------------

Next: [Definitions](Definitions.md)
&nbsp;
&nbsp;
Back: [Using MySQL Procedures](UsingMySQLProcedures.md)
&nbsp;
&nbsp;
Up: [SchemaFW Basics](SchemaFWBasics.md)
&nbsp;
&nbsp;
Top: [Main Page](UserGuide.md)

