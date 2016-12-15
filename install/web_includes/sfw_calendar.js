
// sfw_calendar.js

(function _init()
{
   if (SFW.delay_init("sfw_calendar",_init))
      return;

   SFW.types["calendar"] = _calendar;

   function _calendar(base, doc, caller)
   {
      SFW.base.call(this,base,doc,caller);
   }

   SFW.derive(_calendar, SFW.base);

   // No need for replot function, which is only used to re-sort table rows

   _calendar.prototype.child_finished = function(child, cmd)
   {
   };

   _calendar.prototype.process_day_click = function(t, did)
   {
      SFW.alert("Ya got a click on " + did);
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
            if (tag=="cal_day" && (did=t.getAttribute("data-date")))
               return this.process_day_click(t, did);
         }

         t = t.parentNode;
      }
      return true;
   };
   
})();
