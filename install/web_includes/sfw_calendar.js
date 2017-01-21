
// sfw_calendar.js

(function _init()
{
   if (SFW.delay_init("sfw_calendar",_init, "table"))
      return;

   if (!SFW.derive(_calendar, "calendar", "table"))
      return;

   function _calendar(base, doc, caller, data)
   {
      SFW.types["table"].call(this,base,doc,caller,data);
   }

   // No need for replot function, which is only used to re-sort table rows

   _calendar.prototype.get_on_day_click_url = function()
   {
      return this._xmldoc.documentElement.getAttribute("on_day_click");
   };

   // _calendar.prototype.child_ready = function(child)
   // {
   // };

   _calendar.prototype.child_finished = function(child, cmd)
   {
      this.replot();
      SFW.base.prototype.child_finished.call(this,child,cmd);
   };

   _calendar.prototype.process_day_click = function(t, did)
   {
      var odc = this.get_on_day_click_url();
      if (odc)
      {
         var url = odc + "=" + did;
         var ths = this;
         
         empty_el(this._host);
         SFW.open_interaction(SFW.stage, url, this, this._xmldoc);
      }
   };

   _calendar.prototype.process_month_jump = function(month)
   {
      var url = SFW.update_location_arg("mdate", month);
      if (url)
         window.location = url;

      return false;
   };

   _calendar.prototype.process = function (e,t)
   {
      var did, tag, mon, top = this.top();
      
      if (e.type!="click")
         return true;

      while (t && t!=top)
      {
         if (t.nodeType==1 && (tag=t.tagName.toLowerCase()))
         {
            if (tag=="td" && (did=t.getAttribute("data-date")))
               return this.process_day_click(t, did);
            if (tag=="button" && (mon=t.getAttribute("data-jump")))
               return this.process_month_jump(mon);
         }

         t = t.parentNode;
      }

      return this.call_super_event("table", "process", arguments);
   };
   
})();
