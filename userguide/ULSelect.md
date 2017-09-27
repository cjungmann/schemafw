# ULSelect: A Flexible Select Widget

This is a placeholder page into which I will document obscure feature of this
widget will it is being developed.  It is an attempt to safeguard against forgetting
obscure special-case features that are only used once during development.

Until the widget is complete, the information may be disorganized.

## Calling for **ulselect**

*ulselect* is a field-type designation.  It only works in a merged document
because it collects the information it uses from lookup results in the
current document.

~~~srm
edit
   type : merge
   schema-proc : App_House_Edit
   procedure   : App_House_Read
   schema
      merge-type : form-edit
      form-action : edit_submit
      field : plist
         type   : ulselect
         style  : multiple
         result : people
         on_add : ?person_add
~~~

## Required Response Mode Instructions
- `type : ulselect` causes the form to render a ulselect object for the column value.
- `result : people` identifies the result name where the data for the widget is located.



## Optional Response Mode Instructions
- `style : multiple` allows a multiple select.  For now, there is no single select,
  but it will come.
- `on_add : ?person_add` names the response mode that is used to add an unrecognized
  option to the option list.  The procedure that services this response mode must return
  the new option after saving, as with a form servicing a table.
- `options : off` prevents the options list from being generated.  This is used when any
  new options added by *on_add* are unique, even if its name matches another entered option.
  An example of this would be adding family members, where several families may have a
  child named Jennifer.  The *Jennifer* records should not be shared because they refer
  to unique individuals.
