
// sfw_lookup.js

(function _init()
{
   if ((!("SFW" in window) && setTimeout(_init,100))
       || SFW.delay_init("sfw_lookup",_init,"table"))
      return;

   if (!SFW.derive(_lookup, "lookup", "table"))
      return;

   function _lookup(actors)
   {
      SFW.base.call(this,actors);
   }

   _lookup.prototype.top = function() { return this.widget(); };

   function field_name_from_host(host)
   {
      var label = host.parentNode.selectSingleNode("label");
      return label ? label.getAttribute("for") : "";
   };

   _lookup.prototype.get_form_label = function()
   {
      var tag_name, matched=false, e = this.top();
      while (e && e.nodeType==1)
      {
         if ((matched=(e.tagName.toLowerCase()=="p" && class_includes(e,"form-row"))))
            break;

         e = e.parentNode;
      }

      if (matched)
      {
         var labels = e.getElementsByTagName("label");
         if (labels)
            return labels[0];
      }

      return null;
   };

   _lookup.prototype.get_field_name = function()
   {
      var label = this.get_form_label();
      if (label)
         return label.getAttribute("for");
      return null;
   };

   function _rebuild_lookup(host, xrow, fieldname)
   {
      xrow.setAttribute("lookup-field-match", fieldname);
      SFW.xslobj.transformFill(host, xrow);
      xrow.removeAttribute("lookup-field-match");
   }

   _lookup.prototype.get_form_xrow = function()
   {
      var top, xpath, node = null;
      if ((top=this.top()) && (xpath=top.getAttribute("data-path")))
         node = SFW.xmldoc.selectSingleNode(xpath);
      return node;
   };

   _lookup.prototype.rebuild_content = function()
   {
      var widget, host, xrow, fname;
      if ((widget=this.widget())
          && (host=widget.parentNode)
          && (xrow=this.get_form_xrow())
          && (fname=this.get_field_name()))
         _rebuild_lookup(host,xrow,fname);
   };


   _lookup.prototype.preview_result = function(returned_doc, child)
   {
      this.rebuild_content();
   };

   _lookup.prototype.replot = function(result)
   {
      this.rebuild_content();
      // var top = this.top();
      // var host = top.parentNode;
      // var xpath = top.getAttribute("data-path");
      // var node = SFW.xmldoc.selectSingleNode(xpath);
      // var fname = this.get_field_name();

      // this.pre_transform();
      // _rebuild_lookup(host, node, fname);
      // this.post_transform();
   };


})();
