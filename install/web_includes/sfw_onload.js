
// sfw_onload.js

window.onload = function()
{
   SFW.debugging = true;

   function begin_app()
   {
      SFW.prepare_top_sfw_host();
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

