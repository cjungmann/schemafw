$database : SessionDemo
$xml-stylesheet : session.xsl
$default-mode : home

$session-type : login
$test_authorized : App_Auth_Test
$jump_not_authorized : login.htm

$root
   tag : resultset

$shared : login_schema
   name : person
   form-action : ?default.spec:login_submit
   fields
      field : handle
         label : Handle
      field : password
         label : Password
      field : password2
         label : Repeat Password

$shared : person_fields
   field : id
      hidden : true
   field : fname
      label : First Name
   field : lname
      label : Last Name


login_submit
   procedure : App_Login_Submit
   type : form-confirm
   jump : ?default:home

signup_submit
   procedure : App_Account_Create
   type : form-confirm
   jump : ?default:home


      
      
login
   xml-stylesheet : session.xsl
   procedure : App_Login_Submit
   form-action : ?default:login_submit
   type : form-login
   schema : $login_schema

signup
   xml-stylesheet : session.xsl
   procedure : App_Account_Create
   form-action : ?default:signup_submit
   type : form-login
   schema : $login_fields

logout
   type : session-abandon
   jump : goodbye.htm


home
   procedure : App_Person_List
   type : table
   # root
   #    tag : resultset
   #    attributes
   #       table : persons

   result : 1
     name : persons
     type : table
     schema : person
        fields : $person_fields
        attributes 
           on_line_click : ?default:person_edit
        buttons
           button
              label : New
              type  : add
              task  : ?default:person_add
   
person_add
   procedure : App_Person_Submit
   type : form-new
   schema : person
      fields : $person_fields
      attributes
         form-action : ?default:person_submit

person_edit
   procedure : App_Person_Values
   type : form-edit
   root
      attributes
         type : collection

   result : 1
      name : form
      schema : info
         form-action : ?default:person_submit
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
            

person_submit
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
