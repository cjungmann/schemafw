$database : ImportDemo
$xml-stylesheet: import.xsl
$session-type : anonymous

upload-people
   type : import
   target : Session_Import_People
   confirm-procedure : App_Import_People_Preview
   removal-procedure : App_Import_People_Remove
   schema : people
      buttons
         button
            label: Abandon
            type: call
            task: abandon-people
         button
            label: Accept
            type: call
            task: accept-people


abandon-people
   removal-procedure: App_Import_People_Abandon

accept-people
   accept-procedure: App_Import_People_Accept
   removal-procedure : App_Import_People_Abandon

