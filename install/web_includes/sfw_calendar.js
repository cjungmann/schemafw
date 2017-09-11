
// sfw_calendar.js

(function _init()
{
   if ((!("SFW" in window) && setTimeout(_init,100))
       || SFW.delay_init("sfw_calendar",_init, "table"))
      return;

   if (!SFW.derive(_calendar, "calendar", "table"))
      return;


   function _calendar(base, doc, caller, data)
   {
      SFW.types["table"].call(this,base,doc,caller,data);
   }

   _calendar.prototype.process_cell_click = function(td)
   {
      var rval = null;
      var task = this.get_data_value("on_cell_click");
      if (task)
      {
         rval = {
            task:task,
            id_name:this.get_cell_click_id_name(),
            did:td.getAttribute("data-date")
         };

         function f(n) { return n.nodeType==1 && class_includes(n,"day_content"); }
         var container = SFW.find_child_matches(td,f,true);
         if (container && container.getAttribute("data-id"))
            rval.target = container;
      }

      return rval;
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

      if (!SFW.types["table"].prototype.process.call(this,e,t))
         return false;

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
