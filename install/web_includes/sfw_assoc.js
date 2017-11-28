
// sfw_assoc.js

/* assoc = Associations object */

(function _init()
{
   if ((!("SFW" in window) && setTimeout(_init,100))
       || SFW.delay_init("sfw_assoc", _init, "tbase"))
      return;

   if (!SFW.derive(_assoc, "assoc", "tbase"))
      return;

   function _assoc(actors)
   {
      SFW.types["tbase"].call(this,actors);
      var ths = this;
      function f(n)
      {
         if (n.nodeType==1 && n.tagName.toLowerCase()=="input")
            return (ths._input=n);
         return false;
      }
      SFW.find_child_matches(actors.anchor,f,true,true);

      this._tbody = actors.anchor.getElementsByTagName("tbody")[0];
   }

   _assoc.prototype.shadow_transform = function(val)
   {
      var fname = this.get_field_name();
      var shadow = this.add_schema_shadow(fname,val.trim());
      if (shadow)
      {
         SFW.show_string_in_pre(serialize(SFW.xmldoc));
         var el = this._tbody;
         if (el)
            SFW.xslobj.transformFill(el, shadow.attributes[0]);
         shadow.parentNode.removeChild(shadow);
      }
   };

   _assoc.prototype.replot = function(result)
   {
      this.do_proxy_override("replot", arguments);
   };

   _assoc.prototype.child_finished = function(child, cancelled)
   {
      var schema = this.schema();
      if (schema)
      {
         var name = this.get_field_name();
         var field = this.get_schema_field();
         if (!field)
            field = schema.selectSingleNode("field[@name='" + name + "']");

         if (field)
         {
            var cont = this.widget();
            if (cont)
               SFW.xslobj.transformReplace(cont, field);
         }
      }

      // When done, call the base method to clean up the merged stuff
      // and close the child window.
      SFW.base.prototype.child_finished.apply(this, arguments);
   };

   _assoc.prototype.get_line_click_action = function()
   {
      var field = this.get_schema_field();
      return field ? field.getAttribute("on_line_click") : null;
   }

   _assoc.prototype.get_on_add_info = function()
   {
      var rval = null;
      var field = this.get_schema_field();
      if (field)
      {
         var result, url;
         if ((url=field.getAttribute("on_add")) && (result=this.get_ref_result()))
            rval = { task:url, result:result};
      }
      return rval;
   };

   _assoc.prototype.process = function(e,t)
   {
      // For now, only handle clicks (keyboard handled earlier):
      if (e.type!="click")
         return true;

      var table_el = this.widget();

      // Allow base class to process generic buttons
      if (!SFW.base.prototype.process.call(this,e,t))
         return false;

      var click_info;
      while (t && t!=table_el)
      {
         var tn = t.tagName.toLowerCase();
         if (tn=="button")
         {
            if (t.name=="add")
            {
               click_info = this.get_on_add_info();
               break;
            }
         }
         else if (t.getAttribute("data-id"))
         {
            if ((click_info=this.get_el_click_info(t)))
               return this.process_click_info(click_info);
         }
         t = t.parentNode;
      }

      return true;
   }

})();
