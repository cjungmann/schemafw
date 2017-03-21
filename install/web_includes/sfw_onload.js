
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
         var obj = new SFW.base(sfwhost);
         obj.setup(SFW.xmldoc);
         var type = obj.get_hosted_class();
         if (type)
         {
            obj = new type(sfwhost);
            obj.initialize();
         }
      }
      SFW.setup_event_handling();
   }

   init_SFW(begin_app);
};

