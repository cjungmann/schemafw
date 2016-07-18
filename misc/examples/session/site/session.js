function show_person_info()
{
   // This function is called by a button on a form that
   // was opened by clicking on a line of a table.  The single
   // parameter contains information about that line and the
   // form that was generated to edit the line:
   // 
   // udf_bundle.button   : the button pressed to get here
   // udf_bundle.form     : the form that hosts the bundle (to submit or get info)
   // udf_bundle.xml_node : the xml element associated with the form
   // udf_bundle.html_row : the html table row associated with the form
   //
   // The xml_node contains the data needed to create a URL.
   // the xml_node and html_row are candidates to be replaced if new data is returned.

   alert("We got to show_person_info with " + arguments.length + " arguments.");
}

function delete_person(udf_bundle)
{
   var node = udf_bundle.xml_node;
   if (node)
   {
      // Courteous warning about the deletion:
      var name = node.getAttribute("fname") + " " + node.getAttribute("lname");
      if (window.confirm("Are you sure you want to delete " + name + "?"))
      {
         // This is the basic usage to delete
         var url = "schema?default:delete=" + node.getAttribute("id");
         SchemaFW.default_delete(url, udf_bundle);
      }
   }
}

