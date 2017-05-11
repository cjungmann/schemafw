
// sfw_onload.js

window.onload = function()
{
   SFW.debugging = true;

   function begin_app()
   {
      if (SFW.continuing_autoloads())
      {
         setTimeout(begin_app, 125);
         return;
      }

      var sfwhost = SFW.seek_top_sfw_host();
      if (sfwhost)
      {
         var obj = SFW.get_object_from_host(sfwhost);
         if (obj)
         {
            SFW.arrange_in_host(sfwhost, obj.top());

            obj.setup(SFW.xmldoc);
            obj.initialize();
         }
      }
      SFW.setup_event_handling();
   }

   init_SFW(begin_app);
};

