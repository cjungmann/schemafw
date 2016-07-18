# Context References

## Introduction

A _context reference_ is a word in an SRM file that will be replaced with data
from the XML document (ie the context) produced by the requested SRM mode.

### Where to Use Context References

There are several situations where context references are useful.  Here are a
few examples.

- __The display title__ often benefits from a contextual hint.  A context reference
  in the title can let the user know whose address is being edited.  A title like
  _Edit Phone Number_ is not as helpful as _Edit Bob's Phone Number_.

- __Pre-filling form fields__ with inferred values can help speed form entry and
  reduce data-entry errors.  Pre-filling the account number in a form can be
  critical for accurate form entry. Pre-filling a family name when adding a new
  member will usually be appreciated.

- __Confirmation Questions__ with context hints can help the user avoid making
  mistakes, especially when performing unrecoverable actions like deleting records.
  _Delete Record_ is not as helpful as _Delete Smith Household_.

## How to Use Context References

A context reference is a word or phrase, in curly-braces, with a `@`, '$' or `!` prefix.
The context reference, including the enclosing braces, will be replaced with the
value of an attribute in the XML document.

In all cases, the reference will be resolved from an attribute of the first element
of a result branch in an XML document.  If the prefix is `@`, the value will be taken
from the form data result.  If the prefix is `$`, the value will be taken from a
_variables_-type result.

The `!` prefix refers to the selected line context, so it only applies for an
_on_line_click_ instruction for a table view.  Because it can't be resolved until
a selection is made, the curly-brace expression will remain in the URL to be
resolved once the user has chosen a table row.  When that happens, the {!...} will be
replaced with the value of the attribute of the XML source of the line in the table.

## Examples

The following example demonstrates, in order of appearance, using a context
reference in a title, as the initial value in a field, and how to include the
data that the context references will get their data.  Note that these examples
use the `$` prefix to retrive data from a _variables_-type result.

~~~srm
add_person
   # Context hint in title
   title : Add member of {$family} family
   
   # For the form schema:
   schema-proc : App_Add_Person
   schema
      field : lname
         value : {$family}  # A pre-filled field
         
   # For the context data:
   procedure   : App_Family_Info
   result
      type : variables
~~~

In the next example, the data will be retrieved from the form data, using the
`@` prefix.  All of the form fields are automatically pre-filled with the values
of the selected record, but the form data can be used for a title or a delete
confirmation message, as well.  

~~~srm
edit_person
   title : Update information for {@fname} {@lname}
   procedure   : App_Person_Record
   result
      form-data
      
   schema-proc : App_Update_Person
   schema
      button
         type    : delete
         label   : Delete
         confirm : Delete {@fname} {@lname}?
         task    : ?person:delete_person&person_id={@id}
      field : id
         hidden : true
      field : fname
         read-only : true
      field : lname
         read-only : true
~~~

