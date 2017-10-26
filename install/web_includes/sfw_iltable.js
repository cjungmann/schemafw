
// sfw_iltable.js

/* iltable = InLine TABLE */

(function _init()
{
   if ((!("SFW" in window) && setTimeout(_init,100))
       || SFW.delay_init("sfw_iltable", _init, "tbase"))
      return;

   if (!SFW.derive(_iltable, "iltable", "tbase"))
      return;

   function _iltable(actors)
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
      this._tbody = actors.anchor.getElementsByTagName("tbody")[1];
   }

   _iltable.prototype.replot = function(result)
   {
      if (!this.do_proxy_override("replot",arguments))
      {
         var val = this._input.value.trim();
         var fname = this.get_field_name();
         var shadow = this.add_schema_shadow(fname,val);
         if (shadow)
         {
            var el = this._tbody;
            if (el)
               SFW.xslobj.transformFill(el, shadow.attributes[0]);
            shadow.parentNode.removeChild(shadow);
         }
      }
   };

   _iltable.prototype.get_line_click_action = function()
   {
      var field = this.get_schema_field();
      return field ? field.getAttribute("on_line_click") : null;
   }

   _iltable.prototype.get_on_add_info = function()
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

   _iltable.prototype.process = function(e,t)
   {
      // For now, only handle clicks (keyboard handled earlier):
      if (e.type!="click")
         return true;

      var table_el = this.top();

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
         if (tn=="tr")
         {
            if ((click_info=this.get_line_click_info(t)))
               break;
         }
         t = t.parentNode;
      }

      if (click_info)
         return this.process_click_info(click_info);

      return true;
   }

})();
