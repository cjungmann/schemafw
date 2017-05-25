
// sfw_mixed_view.js

(function _init()
{
   if (SFW.delay_init("sfw_mixed_view",_init,"table"))
      return;

   if (!SFW.derive(_mixed_view, "mixed-view", "iclass"))
      return;

   function _vtable(base,doc,caller,data)
   {
      SFW.types["mixed-view"].call(this,base,doc,caller,data);
   }

   function _mixed_view(base,doc,caller,data)
   {
      SFW.types["iclass"].call(this,base,doc,caller,data);
   }

   _mixed_view.prototype.process_button_manage_subview = function(t,cb)
   {
      var url = t.getAttribute("data-task") || t.getAttribute("data-url");
      if (!url)
         return true;
      
      var nt, iclass, anchor, host;
      while ((nt=t.nodeType)<9)
      {
         if (nt==1)
         {
            
            if (!iclass && (iclass=t.getAttribute("data-sfw-class")))
               anchor = t;
            else if (!host
                     && t.className=="SFW_Host"
                     && !t.getAttribute("data-mview"))
            {
               host = t;
               break;
            }
         }

         t = t.parentNode;
      }

      if (anchor && host)
      {
         var obj = {anchor:anchor, host:host };
         SFW.open_interaction(host, url, this);
      }
      return true;
   };

   _mixed_view.prototype.process_button_delete = function(t,cb)
   {
      return true;
   };

   _mixed_view.prototype.process_button_edit = function(b,cb)
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
         field = SFW.find_child_matches(this.schema(),f,true);
      }

      if (field && "content" in parts && (content=parts.content))
      {
         var url = b.getAttribute("data-url");
         if (url)
            SFW.open_interaction(SFW.stage, url, this);
      }
   };

   // We may no longer need this part....let's see
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

   // Derive after the prototypes are installed!
   if (!SFW.derive(SFW.types["table"], "mixed-table", "mixed-view"))
      return;

   if (!SFW.derive(_vtable, "vtable", "mixed-table"))
      return;



})();
   
