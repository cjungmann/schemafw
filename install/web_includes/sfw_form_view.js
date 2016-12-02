function init_SFW_Views()
{
   SFW.types["form-view"] = _form_view;

   function _form_view(base,doc,caller)
   {
      SFW.types["form-edit"].call(this,base,doc,caller);
   }

   SFW.derive(_form_view, SFW.types["form-edit"]);

   function _get_field_parts(b)
   {
      var r = {};
      var p = b.parentNode;
      var c = SFW.first_child_element(p);
      while (c)
      {
         var tn = c.tagName.toLowerCase();
         switch(tn)
         {
            case "label":
               r.label = c;
               break;
            case "div":
               if (class_includes(c,"field_content"))
                  r.content = c;
               break;
            case "button":
               r.button = c;
               break;
         }

         c = SFW.next_sibling_element(c);
      }

      // Return object if not empty:
      for (c in r)
         return r;
      
      return null;
   }

   _form_view.prototype.process_button_view = function(b,cb)
   {
      var field = null;
      var parts, label, afor, content;
      if ((parts = _get_field_parts(b))
          && (label=parts.label)
          && (afor=label.getAttribute("for"))
         )
      {
         function f(n) { return n.nodeType==1
                         && n.tagName=="field"
                         && n.getAttribute("name")==afor; }
         field = SFW.find_child_matches(this._schema,f,true);
      }

      if (field && "content" in parts && (content=parts.content))
      {
         var url = b.getAttribute("data-url");
         if (url)
            SFW.open_interaction(this._top.parentNode, url, this);
      }
   };
}
