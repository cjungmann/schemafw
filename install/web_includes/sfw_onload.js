
// sfw_onload.js

window.onload = function()
{
   SFW.debugging = true;

   function begin_app()
   {
      var sfwhost = SFW.seek_top_sfw_host();
      if (sfwhost)
      {
         var obj = SFW.get_object_from_host(sfwhost);
         if (obj)
         {
            SFW.base.call(obj, {host:sfwhost});
            SFW.arrange_in_host(sfwhost, obj.top());

            SFW.setup_sfw_host(sfwhost, SFW.xmldoc);
            obj.initialize();
         }
      }
      SFW.setup_event_handling();
   }

   function wait_to_start()
   {
      if (SFW.continuing_autoloads())
         setTimeout(wait_to_start, 125);
      else
         begin_app();
   }

   init_SFW(begin_app);
};

