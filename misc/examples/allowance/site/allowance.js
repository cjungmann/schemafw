function show_person_info(udf_bundle)
{
   var url = "?default:person_info=" + udf_bundle.xml_node.getAttribute("id");
   SchemaFW.new_context(url, udf_bundle);
}

function delete_person(udf_bundle)
{
   var node = udf_bundle.xml_node;
   if (node)
   {
      // Courteous warning about the deletion:
      var name = node.getAttribute("nom");
      if (window.confirm("Are you sure you want to delete " + name + "?"))
      {
         // This is the basic usage to delete
         var url = "?default:delete=" + node.getAttribute("id");
         SchemaFW.default_delete(url, udf_bundle);
      }
   }
}

function delete_account(udf_bundle)
{
   var node = udf_bundle.xml_node;
   if (node)
   {
      // Courteous warning about the deletion:
      var title = node.getAttribute("title");
      if (window.confirm("Delete the \"" + title + "\" account?"))
      {
         // This is the basic usage to delete
         var url = "?default:account_delete"
                + "&id=" + node.getAttribute("id")
                + "&id_person=" + node.getAttribute("id_person");
         
         SchemaFW.default_delete(url, udf_bundle);
      }
      
      
   }
}
