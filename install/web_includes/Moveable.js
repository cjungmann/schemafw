function Moveable() {};

Moveable.process_event = function(e,t)
{
   var html = document.documentElement;
   var html_body = document.body || document.getElementsByTagName("body")[0];
   
   var px = function(n) { return String(n)+"px"; };
   var eleft = function(e)
   { return e.pageX || (e.clientX+(html.scrollLeft||html_body.scrollLeft||0)); };
   var etop = function(e)
   { return e.pageY || (e.clientY+(html.scrollTop||html_body.scrollTop||0)); };

   var get_os_mouse = function(e)
   { return {left:eleft(e),top:etop(e)}; };
   var get_os_diff = function(os1,os2)
   { return {left:os2.left-os1.left,top:os2.top-os1.top}; };
   
   var get_window_offset = function(el)
   {
      var os = el.offsetParent ? get_window_offset(el.offsetParent) : {left:0,top:0};
      return {left:os.left+el.offsetLeft, top:os.top+el.offsetTop };
   };

   var check_prereqs = function()
   {
      var arr = [];
      var reqs = ["Events"];
      for (var i=0; i<reqs.length; ++i)
         if (!(reqs[i] in window))
            arr.push(reqs[i]);
      if (arr.length>0)
         console.log("Missing required functions: " + arr.join(','));

      Events.are_events_registered(["mousedown","mouseup"]);
   };

   var id_object = null;
   var last_resort_id = "Moveable_default_id";

   var object_tag = function(el)
   {
      if (el.id && el.id.length>0)
         id_object = el.id;
      else
      {
         id_object = last_resort_id;
         el.id = id_object;
      }
      return el;
   };

   var object_get = function() {return document.getElementById(id_object);};
   var object_untag = function()
   {
      var el = object_get();
      if (el && el.id==last_resort_id)
         el.id = null;
      return el;
   };

   var get_object_from_handle = function(t)
   {
      var count, dmu = t.getAttribute("data-moveable_up");
      if (dmu)
         count = parseInt(dmu);
      else if (t.tagName.toLowerCase()=="legend")
         count = 2;
      else
         count = 0;
      
      for (var i=0; i<count; ++i)
         t = t.parentNode;
      return t;
   };

   var os_orig_form = null;
   var os_orig_mouse = null;
   
   var internal_process_event = function(e,t)
   {
      var o;
      var f = function(ev)
      {
         var e=ev||window.event;
         var t=e.target||e.srcElement;
         return internal_process_event(e,t);
      };

      switch(e.type)
      {
         case "mousedown":
            if (class_includes(t,"Moveable"))
            {
               if ((o=get_object_from_handle(t)))
               {
                  t.ondragstart = o.ondragstart = null;
                  
                  object_tag(o);
                  os_orig_form = get_window_offset(o);
                  os_orig_mouse = get_os_mouse(e);
                  
                  Events.add_event("mousemove",internal_process_event);
                  return Events.full_cancel(e);
               }
            }
            break;

         case "mousemove":
            if ((o=object_get()))
            {
               var os_diff = get_os_diff(os_orig_mouse, get_os_mouse(e));
               var os_new = { left: (os_diff.left + os_orig_form.left),
                              top : (os_diff.top + os_orig_form.top)};
               
               o.style.left = px(os_new.left);
               o.style.top = px(os_new.top);

               if (!(os_diff.left==0 && os_diff.right==0))
               {
                  os_cur_form = get_window_offset(o);
                  
                  if (os_cur_form.left==os_orig_form.left
                      && os_cur_form.top==os_orig_form.top)
                  {
                     Events.remove_event("mousemove", internal_process_event);
                     console.error("Moveable Target is not position:fixed");
                  }
               }

               return Events.full_cancel(e);
            }
            break;
         
         case "mouseup":
            Events.remove_event("mousemove",internal_process_event);
            object_untag();
            os_orig_form = os_orig_mouse = null;
            break;
      };

      return true;
   };

   check_prereqs();
   
   Moveable.process_event = internal_process_event;
   return internal_process_event(e,t);
};
