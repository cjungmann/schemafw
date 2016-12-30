
// sfw_calendar.js

(function _init()
{
   if (SFW.delay_init("sfw_calendar",_init, "table"))
      return;

   SFW.types["calendar"] = _calendar;

   function _calendar(base, doc, caller, data)
   {
      SFW.types["table"].call(this,base,doc,caller,data);
   }

   SFW.derive(_calendar, SFW.types["table"]);

   // No need for replot function, which is only used to re-sort table rows

   _calendar.prototype.get_on_day_click_url = function()
   {
      return this._xmldoc.documentElement.getAttribute("on_day_click");
   };

   _calendar.prototype.child_finished = function(child, cmd)
   {
      this.replot();
      SFW.base.prototype.child_finished.call(this,child,cmd);
   };

   _calendar.prototype.process_day_click = function(t, did)
   {
      var xslo = SFW.xslobj;

      // set mode.
      // set date.
      // open dialog.

      var odc = this.get_on_day_click_url();
      if (odc)
      {
         var url = odc + "=" + did;
         var ths = this;
         
         empty_el(this._host);
         SFW.open_interaction(SFW.stage, url, this, this._xmldoc);
      }
   };

   _calendar.prototype.process = function (e,t)
   {
      var did, tag, top = this.top();
      
      if (e.type!="click")
         return true;

      while (t && t!=top)
      {
         if (t.nodeType==1 && (tag=t.tagName.toLowerCase()))
         {
            if (tag=="td" && (did=t.getAttribute("data-date")))
               return this.process_day_click(t, did);
         }

         t = t.parentNode;
      }
      return true;
   };
   
})();
