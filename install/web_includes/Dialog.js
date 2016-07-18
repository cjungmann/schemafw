function Dialog(form)
{
}

Dialog.process_event = function(e,t)
{
   
};

Dialog.prototype.find_schema = function(node)
{
   var find_matching_child = function(parent, name)
   {
      var n = parent.firstChild;
      while (n && n.nodeType==1)
      {
         if (n.tagName==name)
            return n;
         n = n.nextSibling;
      }
      return null;
   };
   
   var s, n = node;
   while (n && n.nodeType<=3)
   {
      if (n.nodeType==1)
         if ((s=find_matching_child(n,"schema")))
            return s;
      n = n.parentNode;
   }
   return null;
};

Dialog.prototype.unpack_data = function(data)
{
   this.schema = this.find_schema(data);
};

Dialog.prototype.tag_targets = function(data_xml, data_html)
{
   var label = String((new Date()).time());
   if (data_xml)
   {
      if ("id" in data_xml)
         this.target_xml = data_xml.id;
      else
         data_xml.id = (this.target_xml = "xml_" + label);
   }

   if (data_html)
   {
      if ("id" in data_html)
         this.target_html = data_html.id;
      else
         data_html.id = (this.target_html = "html_" + label);
   }
};

Dialog.prototype.get_xml_target = function()
{
   if (this.target_xml)
      return schema.getElementById(this.target_xml);
   return null;
};

Dialog.prototype.get_html_target = function()
{
   if (this.target_html)
      return document.getElementById(this.target_html);
   return null;
};

Dialog.prototype.untag_targets = function()
{
   var tx = this.get_xml_target();
   if (tx && tx.id.substring(0,4)=="xml_")
      tx.id = null;

   var th = this.get_html_target();
   if (th && th.id.substring(0,5)=="html_")
      th.id = null;
};
