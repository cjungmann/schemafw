
// sfw_calendar.js

(function _init()
{
   if (not ("SFW" in window))
      setTimeout(_init, 100);

   if (SFW.delay_init("sfw_calendar",_init, "table"))
      return;

   if (!SFW.derive(_calendar, "calendar", "table"))
      return;


   function _calendar(base, doc, caller, data)
   {
      SFW.types["table"].call(this,base,doc,caller,data);
   }

   _calendar.prototype.child_finished = function(child, cmd)
   {
      SFW.base.prototype.child_finished.call(this,child,cmd);
      this.replot();
   };

   _calendar.prototype.process_day_click = function(t, did)
   {
      var url = this.get_data_value("on_day_click");
      if (url)
      {
         url += "=" + did;
         empty_el(this.host());
         SFW.open_interaction(SFW.stage, url, this, {caldoc:this.xmldoc(),did:did});
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

      // Don't disturb the arguments array:
      var tel = t;
      while (tel && tel!=top)
      {
         if (tel.nodeType==1 && (tag=tel.tagName.toLowerCase()))
         {
            if (tag=="td" && (did=tel.getAttribute("data-date")))
               return this.process_day_click(tel, did);
            if (tag=="button" && (mon=tel.getAttribute("data-jump")))
               return this.process_month_jump(mon);
         }

         tel = tel.parentNode;
      }

      return this.call_super_event("table", "process", arguments);
   };

})();
