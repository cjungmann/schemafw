var xmlDoc = null;
var xslDoc = null;
var xslObj = null;

var arrScriptFiles = ["includes/XML.js",
                      "includes/sfwobjects.js",
                      "includes/sfwmain.js",
                      "includes/classes.js",
                      "includes/Events.js",
                      "includes/Dialog.js",
                      "includes/Moveable.js"
                     ];



window.onload = function mini_startup()
{
   cache_document_parts();
   var qstring = window.location.search;

   function make_sfw_tag(name) { return "sfw_" + name + "_match"; }

   function modify_xsldoc(xsldoc)
   {
      var ss = xsldoc.documentElement;
      var rootmatch = ss.selectSingleNode("xsl:template[@match='/']");
      var nhtml = rootmatch.selectSingleNode("html:html");

      function make_alias(name)
      {
         var node = nhtml.selectSingleNode("html:" + name);
         if (node)
         {
            var nl = node.selectNodes("*");

            var newnode = add_namespace_el("template",nsXSL,ss,rootmatch,xsldoc);
            newnode.setAttribute("match", make_sfw_tag(name));

            for (var i=0, stop=nl.length; i<stop; ++i)
               newnode.appendChild(nl[i]);

            alert(serialize(newnode.parentNode));
            
         }
      }

      make_alias("head");
      make_alias("body");

      alert(serialize(rootmatch));
   }

   function replace_html_part(name)
   {
      var docel = xmlDoc.documentElement;
      var target = document[name];
      var node = xmlDoc.createElement(make_sfw_tag(name));
      docel.appendChild(node);
      xslObj.transformReplace(target, node);
   }

   function translate_page()
   {
      console.log("Ready to translate the page.");

      replace_html_part("head");
      replace_html_part("body");

      cache_document_parts();

      prepare_page();
   }

   function got_xsldoc(xsldoc)
   {
      xslDoc = xsldoc;
      console.log("Got the XSL document");

      modify_xsldoc(xsldoc);

      xslObj = new XSL(xsldoc, translate_page);
   }

   function got_xmldoc(xmldoc)
   {
      xmlDoc = xmldoc;
      var xsl = xmldoc.selectSingleNode("/processing-instruction('xml-stylesheet')");
      if (xsl)
      {
         var m = /href=\"([^\"]+)\"/.exec(xsl.data);
         if (m)
            xhr_get(m[1], got_xsldoc);
      }
   }

   function scripts_loaded()
   {
      xhr_default_req_header("SFW_DATA_REQUEST", "true");
      xhr_get(qstring, got_xmldoc);
   }

   load_scripts(arrScriptFiles, scripts_loaded);
};

function cache_document_parts()
{
   var nl;
   if (!"html" in document || !document.html)
      document.html = document.documentElement;
   if (!"head" in document || !document.head)
      document.head = ((nl=document.getElementsByTagName("head")) && nl.length) ? nl[0] : null;
   if (!"body" in document || !document.body)
   {
      var n = ((nl=document.getElementsByTagName("body")) && nl.length) ? nl[0] : null;
      document.body = n;
   }
}

function load_scripts(arrFiles, callback)
{
   var host = document.head;
//   var arrScripts = ["includes/XML.js"];
   var ndx = 0;

   function get_next()
   {
      if (ndx < arrScripts.length)
      {
         var s = document.createElement("script");
         s.addEventListener("load", get_next, false);
         s.src = arrScripts[ndx++];
         host.appendChild(s);
      }
      else
      {
         console.log("Finished with javascript adds.");
         if (callback)
            callback();
      }
   }

   get_next();
}
