window.onload = function()
{
   function begin_app()
   {
      if ("get_properties_info" in SFW)
         alert(SFW.get_properties_info().join("\n"));
      
      if ("sfwanchor" in document)
      {
         var el = document.sfwanchor;
         var className = el.getAttribute("data-sfw-class");
         if (className in SFW)
            SFW.pageobj = new SFW[className](el, window.XMLDocument);
      }

      SFW.setup_event_handling();
   }
   
   init_SFW(begin_app);

};

