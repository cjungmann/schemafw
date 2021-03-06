# Adding Buttons

HTML buttons are a common way to provide access to features in web pages.  SchemaFW
provides the easy addition of some default buttons in special contexts.  This guide
will list these default button types below, but also remember that you can create
custom pages to enhance your web page with other interaction models.

There are several sections in this topic:
- **Button Locations**: location on page
- **Adding Buttons**: how to include buttons
- **Button Types**:  add, delete, call, jump, and open
- **Skipping Buttons**: how to conditionally omit buttons
- [Keystroke-matched Buttons](KeyStrokeButtons.md) is an advanced button topic
  on another page.

## Button Locations

The default location for additional buttons is at the top of a page or dialog/form.
The buttons will be listed in-line in the same order as they are listed in the SRM
file.

## Adding Buttons to SRM Response Modes

Buttons can be added to schemas either individually or as a group.  Grouped buttons
are rendered together and separated with vertical space from buttons or button groups
that come before or after.

Sets of individual buttons will be rendered as groups.  An intervening button group
within a set of individual buttons will result in the buttons before and/or after
the button group to be rendered as separate button groups.

### Button Instructions

Each button type includes at least two instructions, the **type** and an action
instruction to perform upon clicking the button.  In practice, buttons also include
a **label** instruction to alert the user to the purpose of the button.

The action instruction can be of these types,
- **url** when a page is to be loaded (_jump_ and _open_ button types)
- **task** for a function call OR to name a response mode for a new context (_call_, _  add_, or _delete_ button types)

~~~srm
demo_mode
   procedure : App_Demo
   schema
      # An individual button
      button
         label : Individual Button
         type  : jump
         url   : www.cnn.com
      # A button group
      buttons
         button
            label : Task 1
            type  : call
            task  : task1_function
         button
            label : Task 2
            type  : call
            task  : task2_function
~~~


## Button Types

Buttons can be added to tables or to forms and dialogs.

### Table-view Related Buttons

While the __r__ and __u__ parts of CRUD are handled in table views by clicking on
a table line, adding and deleting require another trigger which is handled by framework
buttons.

The unique characteristic of a CRUD operation is that the underlying table should
show the change when the operation is successful.  For this reason, the use of the
following _add_ and _delete_ buttons is restricted to table-view dialogs.

#### Button Type _add_

Presents a dialog for adding a record to a table.

~~~
button
   label : Add Record
   type  : add
   task  : mysrm.srm?add_record
~~~

#### Button Type _delete_

Like the _add_ button, the _delete_ button is closely tied to a table view, though
in this case the button is included in an edit dialog generated from the table view.

This is a special button type that can be set up to ask a confirming question before
executing the delete.  Note how the _confirm_ instruction provides a confirmation
question.

Both the _confirm_ and _task_ instructions benefit from
[Context References](ContextReferences.md).  The confirmation question should include
some context information to help the user make a good deccision, and the task URL will
typically need at least a record number, if not also some additional corroborating data
to prevent accidental erasure.  The example below includes context references in the
_confirm_ and _task_ instructions.

~~~
button
   label   : Delete Record
   type    : delete
   confirm : Do you really want to delete {$fname} {$lname}?
   task    : mysrm.srm?delete_record&person_id={$id}&lname={$lname}
~~~

### General Purpose Buttons

These buttons can be added to either table views or forms and dialogs.  Where request
parameters are required, they must be provided using
[Context References](ContextReferences.md) in the URL.

#### Button Type _call_

This button type will call the function named in the _task_ instruction, though it will
recognize _url_ as if it were a _task_ value.  The framework will look for the function
first as a method of the current object, or, failing that, as a global function.

The first version calls the named function with two parameters, the button that
triggered the cal, and the callback function if defined.  The called function is not
required to use either of the arguments.

NOTE: The _task_ or _url_ instruction should only include the function's name, not
any parameters.  See _Future Features_ below for how this might change.

~~~srm
button
   label : Do Strange Thing
   type  : call
   task  : my_do_strange_thing_function
~~~

##### Future Features

In the future, the _task_ instruction may include a parameter list.  As of 2017/12/23,
the plan is to process _param_ instructions under the _task_ instruction, like these:

~~~srm
## PARAM instructions not yet implemented!  Preview Only ##
button
   label : Set Calendar Mode
   type  : call
   task  : set_obj_mode
      param : calendar

button
   label : Enter Edit Mode
   type  : call
   task  : set_form_mode
      param : {@form-mode}
~~~

#### Button Type _jump_

This will leave the current page and open the page referenced by the _url_
instruction.  The URL can be for another page on the same site but more typically
it means

~~~
button
   label  : Look at News
   type   : jump
   url    : www.cnn.com
~~~

#### Button Type _open_

The finaly built-in button type is the _open_ button.  It will cover the current
context with the response generated by calling the _url_ instruction of the button.

In the following example, I didn't use a context reference for the delete record,
but rather had the procedure get that data from session information.  That makes it
more clear that changing a password requires an valid, authenticated session.

~~~
button
   label   : Change Password
   type    : open
   url     : mysrm.srm?change_password
~~~   
     
## Skipping Buttons

Buttons can be conditionally skipped by including a template that matches a button
with the mode="skip_check".  The template that constructs buttons attempts to match
the *skip_check* template, and will ignore the button if the template returns a
value that evaluates as a non-zero number.  See the *Why Not Return True or False?*
section near the bottom of this topic.

### An Example: Import Review

An example is an import of items with keywords.  The application can be programmed
so the import review page indicates any previously unregistered keywords.  It would
also be a nice feature to allow the user to decide if the new keywords should be
registered while adding the new items.

Import pages generally have two buttons to _Accept_ and _Abandon_ the import.
If the default action for _Aceept_ is to register the new keywords, another button
might be added to incorporate the new items, but skip the new keywords.  However,
this additional button wouldn't make sense if there are no unregistered keywords,
so it should be omitted in that case.

A good skip_check template will selectively match the context of the skippable
button to avoid unexpected skips in other contexts.  In the example skip_check
template, this is done by checking for the existence of another element.  Other
applications might do it with an extra attribute in the button, schema, or
result that hosts the button.

#### Simple Example

~~~xsl
<xsl:template match="button[../import_item_review]" mode="skip_check">
   <xsl:variable name="count" select="count(../keywords/row[not(@id_keyword)])" />
   <xsl:choose>
      <xsl:when test="$count=0 and not(@label='Accept') and not(@label='Abandon')">1</xsl:iwhen>
      <otherwise>0</otherwise>
   </xsl:choose>
</xsl:template>
~~~

#### Alternative Example (recommended)

Another means of deciding to button is to add an attribute to the button in the
SRM file, which would then be checked in the template.  Note how the second button
includes the instruction **no_add : true**.  It doesn't matter what value is
assigned to *no_add*, the framework only checks if it is defined.

~~~srm
import-review
   type : import-review
   procedure : App_Item_Import_Review
   button
      type : jump
      label : Accept
      url   : ?accept
   button
      type   : jump
      label  : Accept but Skip New Keywords
      url    : ?accept_no_new
      no_add : true
   button
      type   : jump
      label  : Abandon
      url    : abandon
   result
      name : import_item_review
   result
      name : keywords
~~~

~~~xsl
<xsl:template match="button[../import_item_review]" mode="skip_check">
   <xsl:variable name="count" select="count(../keywords/row[not(@id_keyword)])" />
   <xsl:choose>
      <xsl:when test="$count=0 and @no_add">1</xsl:when>
      <otherwise>0</otherwise>
   </xsl:choose>
</xsl:template>
~~~

Notice how the *no_add* instruction simplifies how the template decides whether
or not to skip the button.  It provides a direct identification of the skippable
button that will not fail if the button labels are changed in the future.

#### Why Not Return True or False?

The only way to use the result of the *skip_check* template is to use its results
in a variable.  The code at the top of the *construct_button* template is:

~~~xsl
   <xsl:variable name="skip">
      <xsl:apply-templates select="." mode="skip_check" />
   </xsl:variable>

   <xsl:if test="not(number($skip))">
   .
   .
   .
~~~

The result of the *skip_check* template will always be a string, and will thus
always evaluate to *true*.  Since variable will always be a string, the only
option to evaluate as false or true is to return 0 or non-zero, respectively.