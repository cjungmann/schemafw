$database : SchemaDemo

# Use the following for a universal stylesheet.  In this case,
# only the 'collection' requires the stylesheet.  The other modes
# will be rendered on the same page as the collection, so they
# do not use the xml-stylesheet setting:
#
# $xml-stylesheet : demo.xsl

# Mode to use if a specific mode is not requested on the URL.
# $default-mode : list

$root
   tag : resultset

# This shared mode is used by the collection and form modes
# for a consistent presentation of the fields.  It is referenced
# in the 'list', 'new', and 'submit' modes.
$shared : person_fields
   field : id
     readOnly : true
     hidden : true
   field : handle
      label : Handle
   field : fname
      label : First Name
   field : lname
      label : Last Name

$shared : person_schema_attributes
   form-action : ?default:submit
   title : Person Dialog

# debugging
#    xml-stylesheet : demo.xsl
#    procedure : App_Person_Collection
#    row-name : person  # ignored when a tagged schema is defined
#    schema
   

# The first mode is the default mode and will be used if
# a mode is not specified in the URL or in a $default-mode
# global instruction.
list
   xml-stylesheet : demo.xsl
   procedure : App_Person_Collection
   type : table
   root-procedure : App_Person_Root

   root
      tag : resultset
      attributes
         table : persons

   result : 1
      type : table
      name : persons
      schema : person
         fields : $person_fields
         attributes
            on_line_click : ?default:edit
            title : Edit People
         buttons
            button
               label : New
               type  : add
               task  : ?default:new

new
   procedure : App_Person_Submit
   type : form-new
   schema : person
      fields : $person_fields
      attributes : $person_schema_attributes

edit
   procedure : App_Person_Values
   type : form-edit
   root
      attributes
         type : collection

   result : 1
      name : form
      schema : info
         fields : $person_fields
         attributes : $person_schema_attributes
         buttons
            button
               label : Delete
               type : call
               task : delete_person
            button
               label : Info
               type : call
               task : show_person_info
            

submit
   procedure : App_Person_Submit
   type : form-submit
   schema : person
      fields : $person

   result : 1
      name : result
      type : update
         
      # The row-name of the submit result must match the
      # the row-name of the table (as defined in the table
      # by the schema or its own row-name setting).
      row-name : person

delete
   # To use the default SchemaFW deletion handling code, the following
   # SQL command should be included in the procedure as the first query
   # that returns a result:
   # SELECT ROW_COUNT() AS deleted;
   #
   # Then the udf_function in the javascript can call default_delete()
   # with the document returned by xhr_get() to remove the xml and html
   # data and remove the dialog.
   procedure : App_Person_Delete
   type : delete
