
// sfw_iltable.js

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
      SFW.find_child_matches(actors.input,f,true,true);
   }

   _iltable.prototype.get_on_line_click = function()
   {
      var field = this.get_schema_field();
      return field ? field.getAttribute("on_line_click") : null;
   }

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
         if(t.tagName.toLowerCase()=="tr")
            if ((click_info=this.get_line_click_info(t)))
               return this.process_click_info(click_info);
         t = t.parentNode;
      }

      return true;
   }

})();
