# Response Mode Types

To be clear, the following predefined response mode types are not required,
but are associated with built-in behaviors.  Custom response modes can
trigger custom XSL templates and scripts.  However, the predefined response
modes handle most situations, and are useful examples if customization is
required.

The _type_ instruction of a response mode will be included as an attribute
of the root of the XML document.  Some built-in mode types will signal the
schema.fcgi server program to treat the request differently, others simply
signal how the client-side data handling should proceed.  Others do nothing,
but by being included in the resulting document, can facilitate custom handling.

## Client-side Handling

The server program _schema.fcgi_ copies the contents of the _info_ instruction
to the _mode-type_ attribute of the documentElement of the returned XML file.
In particular, this is used by the _child_finished_ function prototype of the
default _table_ object in the client-side software to determine how to process
the result.

Developers are encouraged to use the _info_ instruction to trigger custom
events in custom collection objects derived from _table_.  Look to the example
of _child_finished_ the unminified version of _sfw_table.js_.

## List Response Mode Types

These are the predefined response mode types:
- View Modes
  - **[table](#mode-type-table)** for tabular display of data
- Form Presentation Modes  
  - **[form-new](#mode-type-form-new)** to present a (mostly) empty form
  - **[form-edit](#mode-type-form-edit)** to present a row-related form
  - **[form-view](#mode-type-form-view)** special form view, initially intended
    for forms with multiple sections.
  - **[form-import](#mode-type-form-import)** special mode that creates a
    multipart/form-data encoded form with a _file_ input field for importing data.
- Form Submit Modes    
  - **[form-submit](#mode-type-form-submit)** submits the form data to MySQL and
    returns the query result(s) of the stored procedure that processes the data.
  - **[form-result](#mode-type-form-result)** submits the form data to MySQL and
    waits for an indication of success or failure.
  - **[import](#mode-type-import)** parses the submitted file and stages it
    in a quarantine table from which the user can review the uploaded data.
  - **[save-post](#mode-type-save-post)** a special form-handling response mode
    that writes post data to a target file for debugging.
- Action Modes
  - abandon-session
  - delete
  - info
  - lookup

  - export
  - import-verdict
  - import-review

### Mode Type: table

This mode type is for displaying tabular data, generally a single query
result.  It usually includes
- *on_line_click* instruction to define the action to take when a user clicks
   on a table row.  Typically, this should open a form to edit the indicated row.
- _button_ of type _add_ to add new records.

The other table action that might be expected is a _delete_ action.  This should
be handled in an edit page with a Delete button.

### Mode Type: form-view

Despite the _form-_ prefix of its name, this form type is a data presentation
type.  What distinguishes this mode type from _table_ is that it is intended to
display multiple query results on a single view.  An example would be a form to
view and manage a person's email addresses and phone numbers.

This mode type often includes a button next to a section to manage its contents.

### Mode Type: form-new

Displays a form intended to send new data to an application.  This mode type
is used typically an empty form, but some fields may be pre-filled for convenience.

This form is widely used for submitting new data or for login dialogs.

This form should include a _form-action_ instruction to direct the form to the
response mode (mode types _form-submit_ or _form-result_) that will accept the data.

### Mode Type: form-edit

Displays a form for making updates to existing data.  It will usually have
prefilled fields that contain the current values of the fields to which they
are associated.

This form should include a _form-action_ instruction to direct the form to the
response mode (mode types _form-submit_ or _form-result_) that will accept the data.

### Mode Type: form-import

This is a special mode type for importing data.  It generates a
[multipart/form-data](https://www.w3.org/TR/html401/interact/forms.html#h-17.13.4.2)
form with a file input field.

This entry needs work: I deleted an working example of importing, so I need to work
this out again to make appropriate suggestions.

The general idea is that the form-import will have a form-action directing to a
response mode of type _import_ which will jump to an _import_review_ response mode.
The user can see the how SchemaFW has interpreted the uploaded data and choose to
accept or abandon the data.

## Form Submission Types

SchemaFW issues a second request for a URL when it jumps to a new page.  SchemaFW
pages are generated from an XSL-transformed XML document, but current browsers no
longer make the source XML available, so SchemaFW asks the server again for the
data using the page's URL.  Unfortunately, for POSTed requests, the request must
be sent as a GET request, without the field data that was previously submitted.

As a result of this behavoir, SchemaFW provides two submission modes, _form-submit_
directly submits and receives XML in return for pop-up forms that do not change
the window location, and the _form-result_ mode type that returns only a notice
of the success or failure of a form submission (with a message) and a _jump_
instruction to direct the client to another page, generated by a GET request,
that displays the results of the form submission.

### Mode Type: form-submit

This mode type should return data from a POSTed form.  This mode should only
be used for popup form dialogs that maintain a constant window.location.

When the window location changes to a new URL, SchemaFW rereads the URL to get the
raw data for managing a table view.

### Mode Type: form-result

This mode type should return a single row in a single result of a resultset.  The
The row should be generated with code like this, where error=0 will immediately
invoke the _jump_.  

~~~sql
   IF succeeded THEN
      SELECT 0 AS error, 'success' AS msg;
   ELSE
      SELECT 1 AS error, 'failure' AS msg;
   END IF;
~~~

### Mode Type: import

This name is slightly confusing in that it doesn't include _submit_, but is named
for its task of performing the import of the data.  It converts the submitted
file to and INSERTs it into a quarantine table.  This mode should jump to an
_import-review_ mode that presents the staged data and gives the user the option
to accept or abandon it.

### Mode Type: save-post

This special response mode is designed to aid in debugging forms.  If a _form-new_
or _form-edit_ mode directs the form action to a _save-post_ mode type, SchemaFW
will generate a text file that can be used with the **-i** _schema.fcgi_ command
line option to duplicate the http request.  This is particulary useful for debugging
import files that cannot be done with the **-v** command line option, but is also
convenient for debugging long posted forms.

It is important to know that the _save-post_ mode **does not** submit data to MySQL,
so no changes will occur as a result of this mode type.
