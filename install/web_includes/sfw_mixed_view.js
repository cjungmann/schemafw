
// sfw_mixed_view.js

(function _init()
{
   if (SFW.delay_init("sfw_mixed_view",_init,"table"))
      return;

   if (!SFW.derive(_mixed_view, "mixed-view", "iclass"))
      return;

   function _mixed_view(base,doc,caller,data)
   {
      SFW.types["iclass"].call(this,base,doc,caller,data);
   }

   function _sub_mixed_view(base,doc,caller,data)
   {
      SFW.types["iclass"].call(this,base,doc,caller,data);
   }

   _sub_mixed_view.prototype.child_finished = function(cfobj)
   {
      this.process_get_update(cfobj);
      cfobj.cdata.host.style.display = "";
   };

   _sub_mixed_view.prototype.child_finished = function(cfobj)
   {
      cfobj.cdata.host.style.display = "";
   };
   
   _sub_mixed_view.prototype.process_button_manage_subview = function(t,cb)
   {
      var url = t.getAttribute("data-task") || t.getAttribute("data-url");
      if (!url)
         return true;

      // A vtable object: do we need this?
      var sobj = SFW.seek_event_object(t);
      
      var actors = SFW.seek_event_actors(t);
      if (actors)
      {
         function geta(name) { return (name in actors) ? actors[name] : null; }
         var host, iclass;
         if ((host=geta("host")) && (iclass=geta("iclass")) && iclass in SFW.types)
         {
            var obj = SFW.types[iclass](host);
            var fobj = {anchor:geta("anchor"),host:host};
            host.style.display = "none";
            SFW.open_interaction(SFW.stage, url, this, fobj);
            return false;
         }
      }
      return true;
   };

   _sub_mixed_view.prototype.process_get_update = function(cfobj)
   {

   };

   _mixed_view.prototype.process_button_delete = function(t,cb)
   {
      return true;
   };

   _sub_mixed_view.prototype.process_button_edit = function(b,cb)
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


   function _vtable(base,doc,caller,data)
   {
      SFW.types["iclass"].call(this,base,doc,caller,data);
   }


   _vtable.prototype.child_finished = function(cfobj)
   {
      cfobj.cdata.host.style.display = "";
   };

   // Derive after the prototypes are installed!
   // if (!SFW.derive(SFW.types["table"], "mixed-table", "mixed-view"))
   //    return;

   if (!SFW.derive(_sub_mixed_view, "sub-mixed-view", "iclass"))
      return;

   if (!SFW.derive(_vtable, "vtable", "sub-mixed-view"))
      return;



})();
   
