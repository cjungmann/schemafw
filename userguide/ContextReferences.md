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
value of an attribute in the XML document.  For example, 

In all cases, the reference will be resolved from an attribute of the first element
of a result branch in an XML document.  The the following prefixes are used to access
the context values:

- __@__ accesses a form data result
- __$__ accesses an attribute in a variables or qstring result.
- __!__ accesses an attribute of the selected row of a table-view.

The __@__ and __$__ references are available when building the page,
and can be used to modify titles, captions, or URLs. 

The __!__ prefix refers to the selected line context, so it only applies for an
*on_line_click* instruction for a table view (see [Making Sense of *on_line_click*](OnLineClick.md)).
Because it can't be resolved until a selection is made, the curly-brace expression
will remain in the URL to be resolved once the user has chosen a table row.  When
that happens, the {!...} will be replaced with the value of the attribute of the
XML source of the line in the table.

### QString Context Reference

The instruction `qstring : reflect` directs the _schema.fcgi_ server component
to add a qstring pseudo-result to the collection of results.  The pseudo-result
will be a _variables_ type named "qstring" and the _rndx_ value will be set
according to its position as the final result.

The contents of the _qstring_ result are accessible using the variables __$__ prefix.

## Examples

The following example demonstrates several context references.  The context
information comes from a _variables_ query result and from a _qstring reflection_.
The response mode was invoked with a URL that includes the family id number in
order to make it available as a qstring reflection.  Note how the `{$...}` strings
are constructed.

To illustrate usage, this example includes both qstring- and query-type variables
results.  In practice, the *on_line_click* instruction that called this response mode
would probably have included both the row id and the family name in the URL, using the
__!__ notation, for qstring reflection to save the server from having to run the extra
query.  

~~~srm
add_person
   # Context hint in title
   title : Add member of {$family} family

   # Make qstring values available
   qstring : reflect
   
   # For the form schema:
   schema-proc : App_Add_Person
   schema
      field : id
         value : {$id}      # pre-filled field from QString
      field : lname
         value : {$family}  # pre-filled field from a result element
         
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

