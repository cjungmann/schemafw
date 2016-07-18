$database : AllowanceDemo
$default-mode : list

$root
   tag : resultset

$shared : person_fields
   fields
      field : id
        readOnly : true
        hidden : true
      field : nom
         label : Name
      field : birthday
         label : Birthday

$shared : account_fields
   fields
      field : id
         hidden : true;
      field : id_person
         hidden : true
         ref_value : @id_person
      field : title
         label : Account Name
      field : balance
         label : Balance

$shared : person_schema_attributes
   attributes
      form-action : ?default:submit
      title : Person Dialog

list
   xml-stylesheet : allowance.xsl
   procedure : App_Person_List
   type : collection

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
            # Including a button for a new record requires two attributes:
            button-New : add
            on_new_record : ?default:new
         buttons
            button
               label : New
               type : add
               task : ?default:new

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
               label : Accounts
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

      # You don't need a schema for a submit result that gets folded
      # into other data, but you do need to ensure that the result
      # element has the same name as the elements of the schema into
      # which is to be incorporated.  The row-name setting sets the
      # name without creating an entire schema.
      row-name : person

delete
   procedure : App_Person_Delete
   type : delete

person_info
   procedure : App_Person_Info
   root
      attributes
         docflag : person_accounts

   # We'll be using values from the first result to add arguments
   # to the on_line_click URL.  You could use result[@number=1]/row, or
   # be more explicit by naming the result group and the rows:
   result : 1
      type : ref
      row-name : person
      
   result : 2
      name : accounts
         
      schema : account
         attributes
            on_line_click : ?default:account_edit&@id_person
            title : Edit Account
         on_line_click
            url : ?default:account_values
            param
               tag : id_person
               ref : @id
            param
               tag : id
               line: @data-id
            param
               tag : balance
               node : @balance
         buttons
            button
               label : Add Account
               type : add
               task : ?default:account_new&id_person=@id


account_new
   procedure : App_Account_Values
   result : 1
      type : ref
   result : 2
      type : form
      form-action : ?default:account_submit
      name : accounts
      schema : account
         fields : $account_fields

account_values
   procedure : App_Account_Values
   result : 1
      type : ref
      row-name : account
   result : 2
      type : form
      form-action : ?default:account_submit
      schema : person
         fields : $account_fields
         buttons
            button
               label : Delete
               type : call
               task : delete_account

account_submit
   procedure : App_Account_Submit
   result : 2
      type : update
      row-name : account

account_delete
   procedure : App_Account_Delete
   type : delete
