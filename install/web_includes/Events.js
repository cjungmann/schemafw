function Events() {}
Events.remove_event = void(0);
Events.is_event_registered = function(name) { return false; };
Events.are_events_registered = function(arr) { return true; };
Events.add_event = function(name,func)
{
   var arr_events = [];

   var adder = null;
   var deleter = null;
//   var target = document.body;
   if ('addEventListener' in document)
   {
      adder = function(n,f) { document.addEventListener(n,f,true); };
      deleter = function(n,f) { document.removeEventListener(n,f,true); };
   }
   else if ('attachEvent' in document.body)
   {
      adder = function(n,f) { document.body.attachEvent("on"+n,f); };
      deleter = function(n,f) { document.body.detachEvent("on"+n,f); };
   }

   // Replace external functions:
   Events.is_event_registered = function(name) { return arr_events.indexOf(name)!=-1; };
   Events.are_events_registered = function(arr)
   {
      var arr_missing = [];
      for (var i=0,stop=arr.length; i<stop; ++i)
         if (!Events.is_event_registered(arr[i]))
            arr_missing.push(arr[i]);

      if (arr_missing.length>0)
      {
         console.log("Missing event handlers: " + arr_missing.join(', '));
         return false;
      }
      
      return true;
   };

   
   Events.add_event = function(n,f)
   {
      if (!Events.is_event_registered(n))
      {
         adder(n,f);
         arr_events.push(n);
      }
   };

   Events.remove_event = function(n,f)
   {
      var pos = arr_events.indexOf(n);
      if (pos!=-1)
      {
         deleter(n,f);
         arr_events.splice(pos,1);
      }
   };
   
   Events.add_event(name,func);
};

Events.full_cancel = function(e)
{
   if ("stopPropagation" in e)
      e.stopPropagation();
   if ("preventDefault" in e)
      e.preventDefault();
   e.cancelBubble = true;
   e.returnValue=false;
   return false;
};

