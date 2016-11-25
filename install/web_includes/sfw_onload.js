window.onload = function()
{
   function begin_app()
   {
      // if ("get_properties_info" in SFW)
      //    alert(SFW.get_properties_info(SFW).join("\n"));
      
      if ("sfwanchor" in document)
      {
         var director = SFW.get_director(SFW.xmldoc);
         if (director)
            SFW.pageobj = director;
         
         // var el = document.sfwanchor;
         // var className = el.getAttribute("data-sfw-class");
         // if (className in SFW)
         //    SFW.pageobj = new SFW[className](el, SFW.xmldoc);
      }

      SFW.setup_event_handling();
   }
   
   init_SFW(begin_app);

};

