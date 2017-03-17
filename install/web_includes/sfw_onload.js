
// sfw_onload.js

window.onload = function()
{
   function begin_app()
   {
      // if ("get_properties_info" in SFW)
      //    alert(SFW.get_properties_info(SFW).join("\n"));

      SFW.debugging = true;

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

      // var anchor = SFW.seek_page_anchor(3, document.body);
      // if (anchor)
      // {
      //    var mtype = SFW.xmldoc.documentElement.getAttribute("mode-type");
      //    var p = anchor.parentNode;
      //    if (mtype in SFW.types && (p.className=="SFW_Host"))
      //       p.sfwobj = new SFW.types[mtype](p,document.XMLDocument,null);
      // }
         
      SFW.setup_event_handling();
   }
   
   init_SFW(begin_app);

};

