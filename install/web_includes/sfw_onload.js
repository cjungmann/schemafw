window.onload = function()
{
   function begin_app()
   {
      // if ("get_properties_info" in SFW)
      //    alert(SFW.get_properties_info(SFW).join("\n"));

      var anchor = SFW.seek_page_anchor(3, document.body);
      if (anchor)
      {
         var mtype = SFW.xmldoc.documentElement.getAttribute("mode-type");
         var p = anchor.parentNode;
         if (mtype in SFW.types && (p.className=="SFW_Host"))
            p.sfwobj = new SFW.types[mtype](p,document.XMLDocument,null);
      }
         
      SFW.setup_event_handling();
   }
   
   init_SFW(begin_app);

};

