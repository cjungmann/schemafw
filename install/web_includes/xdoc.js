// Must be non-closure function to allow original window.onload() to return:
function initialize_new_page()
{
   var func = initialize_new_page.func;
   if (func && func in window)
      window[func]();
   else if ("onload" in window)
      window.onload();
}

function dump_text(str)
{ 
   var ppre = document.createElement("pre");
   document.body.appendChild(ppre);
   ppre.appendChild(document.createTextNode(str));
}

function empty_function() { }
function null_return_function()   { return null; }

function xhr_error(xhr) { dump_text(xhr.responseText); alert("failed to get the XML document"); }

function get_get_ie_domdoc()
{
   if (!("ActiveXObject" in window))
      return function() { alert("IE not running."); };
   
	var arr = ["Msxml2.DomDocument.6.0",
              "Msxml2.DomDocument.4.0",
		        "Msxml2.DOMDocument"];

   var protodoc = null;

   function retval(xmlstr)
   {
      var newdoc = protodoc.cloneNode(false);
      if (xmlstr)
         newdoc.loadXML(xmlstr);
      return newdoc;
   };

   for (var i=0,stop=arr.length; protodoc==null && i<stop; ++i)
   {
      try
      {
         protodoc = new ActiveXObject(arr[i]);
      }
      catch(e) {;}
   }

   if (protodoc)
      return retval;
   else
      return function(x) { return null; };
}

var get_ie_domdoc = get_get_ie_domdoc();

var browser_is_ie = ("ActiveXObject" in window)
   ? function() {return true;} : function() {return false; };

function get_xhr()
{
   // Replace global get_xhr() symbol with system-appropriate XHR getter.
   
   // Default in case no getter is found:
   get_xhr = function() { return null; };
   
   if ("XMLHttpRequest" in window)
      get_xhr = function() { return new XMLHttpRequest(); };
   else if ("ActiveXObject" in window)
   {
	   var arr = ["Msxml2.XMLHTTP.6.0",
		           "Msxml2.XMLHTTP.3.0",
		           "MSXML2.XMLHTTP",
		           "Microsoft.XMLHTTP"];

      // Use function to check so unused XHR falls out of scope.
      function try_class(class_str)
      {
         var r = null;

         try { r=new ActiveXObject(class_str); }
         catch(e) {;}

         return r != null;
      }
      
	   for (var i=0,stop=arr.length; i<stop; i++)
	   {
         if (try_class(arr[i]))
         {
            var s = arr[i];
            get_xhr = function() { return new ActiveXObject(s); };
            break;
         }
	   }
   }

   return get_xhr();
}

// For use instead of XPath, to avoid brower differences.
function get_named_elements(el, name, firstOnly)
{
   var arr = [];
   var n = el.firstChild;
   while(n)
   {
      if (n.nodeType==1)
      {
         var nname = String(n.baseName
                            || n.localName
                            || n.nodeName
                            || n.tagName).toLowerCase();
         if (nname==name)
         {
            if (firstOnly)
               return n;
            else
               arr.push(n);
         }
      }
      n = n.nextSibling;
   }

   return arr.length ? arr : null;
}

function make_importable_node(target_doc, source, deep)
{
   var add_el = null_return_function;

   function prepare_add_el(doc)
   {
      add_el = null_return_function;
      if ("createElementNS" in doc)
         add_el = function(doc,tag,ns) { return doc.createElementNS(ns,tag); };
      else if ("createNode" in doc)
         add_el = function(doc,tag,ns) { return doc.createNode(1,tag,ns); };
      else
         add_el = function(doc,tag,ns) { return doc.createElement(tag); };
   }
   
   function copy_attributes(target, source)
   {
      var a = source.attributes;
      if (a)
      {
         for (var i=0,stop=a.length;i<stop;i++)
         {
            var at = a[i];
            target.setAttribute(at.name, at.value);
         }
      }
   }

   function copy_element(doc, source, deep)
   {
      // Likely only used by IE so code written to save time in IE;
      var name = String(source.baseName || source.localName).toLowerCase();
      
      var el = add_el(doc,
                      name,
                      source.namespaceURI);

      copy_attributes(el, source);

      if (deep)
      {
         var cn = source.childNodes;
         for (var i=0,stop=cn.length; i<stop; i++)
         {
            var c = cn[i];
            switch(c.nodeType)
            {
            case 1:  // element
               el.appendChild(copy_element(doc, c, true));
               break;
            case 3:  // text
               el.appendChild(doc.createTextNode(c.data));
               break;
            case 2:  // attribute : skip, already copied above
               // Other, below, shouldn't need to copy, but
               // identified in case I change my mind.
            case 4:  // CDATA
            case 5:  // Entity Reference
            case 6:  // Entity
            case 7:  // Processing Instruction
            case 8:  // Comment
            case 9:  // Document
            case 10: // Document Type
            case 11: // Document Fragment
            case 12: // Notation;
               break;
            }
         }
      }
      return el;
   }

   prepare_add_el(target_doc);

   if ("importNode" in target_doc)
      return target_doc.importNode(source,deep);
   else
      return copy_element(target_doc, source, deep);
}

function fix_output_element(doc)
{
   var node = get_named_elements(doc.documentElement, "output", true);
   if (node)
   {
      if (node.getAttribute("output")=="xml")
         node.setAttribute("output","html");
      if (node.getAttribute("doctype-public"))
         node.removeAttribute("doctype-public");
      if (node.getAttribute("doctype-system"))
         node.removeAttribute("doctype-system");
   }
}

function inject_imports(ibefore, docel, impdoc, callback)
{
   var doceldoc = docel.ownerDocument;
   
   function diff_by_attr(left, right, aname)
   {
      var lval = left.getAttribute(aname);
      var rval = right.getAttribute(aname);
      return (lval==null && rval==null) ? false : lval!=rval;
   }

   var attarr = [ "match", "name", "mode" ];
   var astop = attarr.length;

   function insert_if_not_found(el)
   {
      var ns = el.namespaceURI;
      var lname = el.localName;

      if (lname=="output")
         return;
      
      var found = false;
      var kid = docel.firstChild;
      while (kid)
      {
         if (kid.nodeType==1)
         {
            if (kid.namespaceURI==ns && kid.localName==lname)
            {
               var matched = true;
               for (var i=0; matched && i<astop; ++i)
                  matched = !diff_by_attr(el, kid, attarr[i]);

               if (matched)
               {
                  found = true;
                  break;
               }
            }
         }

         kid = kid.nextSibling;
      }

      if (!found)
         docel.insertBefore(make_importable_node(doceldoc, el, true), ibefore);
   }
   
   var elImp = impdoc.documentElement.firstChild;
   while (elImp)
   {
      if (elImp.nodeType==1)
         insert_if_not_found(elImp);

      elImp = elImp.nextSibling;
   }

   ibefore.parentNode.removeChild(ibefore);

   if (callback)
      callback();
}

function resolve_imports(xsl, callback)
{
   var docel = xsl.documentElement;
   var import_list = get_named_elements(docel, "import");
   if (import_list)
   {
      var stop = import_list.length;
      var ndx = 0;

      // Recursion via inject() callback for xhr_get()
      function do_next()
      {
         if (ndx<stop)
         {
            var import_el = import_list[ndx++];
            var href = import_el.getAttribute("href");
            function inject(doc) { inject_imports(import_el, docel, doc, do_next); }
            xhr_get_xsl(href, inject);
         }
         else
            callback(xsl);
      }
      do_next();
   }
   else
      callback(xsl);
}

function xhr_get(url, cb_ok, cb_error)
{
   if (!cb_error)
      cb_error = xhr_error;
   function check_xhr_result(xhr)
   {
      if (xhr.readyState==4)
      {
         xhr.onreadystatechange = empty_function;
         if (xhr.responseXML)
         {
            var doc;
            if (browser_is_ie())
               doc = get_ie_domdoc(xhr.responseText);
            else
               doc = xhr.responseXML;
            cb_ok(doc);
         }
         else
            cb_error(xhr);
      }
   }

   var xhr = get_xhr();

   if (xhr)
   {
      xhr.open("GET", url, true);
      if (browser_is_ie())
         xhr.responseType = "msxml-document";
      xhr.onreadystatechange = function() { check_xhr_result(xhr); };
      xhr.send(null);
   }
}

function xhr_get_xsl(url, cb_ok, db_error)
{
   function cb(xsl)
   {
      // Perform WebKit repairs to XSL document
      fix_output_element(xsl);
      resolve_imports(xsl,cb_ok);
   }

   xhr_get(url, cb, db_error);
}

function force_sslinks_to_load(nl, callback)
{
   var i = 0;
   var stop = nl.length;
   if (stop==0)
      return;

   var doc = nl[0].ownerDocument;
   var head = nl[0].parentNode;

   function load_next()
   {
      if (i<stop)
      {
         var href;
         var s = nl[i++];
         if (s.getAttribute("rel")=="stylesheet")
         {
            var el = doc.createElement("link");
            el.setAttribute("rel", "stylesheet");
            el.setAttribute("type", "text/css");

            if ((href=s.getAttribute("href")))
               el.href = href;
            else if (s.firstChild && (href=s.firstChild.data))
            {
               if ("stylesheet" in el && "cssText" in el.stylesheet)
                  el.stylesheet.cssText = href;
               else
               {
                  try      { el.appendChild(doc.createTextNode(href)); }
                  catch(e) { el.text = href; }
               }
            }
            head.replaceChild(el,s);
         }
         
         load_next();
      }
      else
         callback();
   }
   
   load_next();
}

function force_scripts_to_load(nl, callback)
{
   var i = 0;
   var stop = nl.length;
   if (stop==0)
      return;

   var doc = nl[0].ownerDocument;
   var head = nl[0].parentNode;

   function load_next()
   {
      if (i<stop)
      {
         var src;
         var s = nl[i++];
         var el = doc.createElement("script");
         el.onload = load_next;
         
         if ((src=s.getAttribute("src")))
            el.src = src;
         else if (s.firstChild && (src=s.firstChild.data))
         {
            try      { el.appendChild(doc.createTextNode(src)); }
            catch(e) { el.text = src; }
         }

         head.replaceChild(el,s);
      }
      else
         callback();
   }

   load_next();
}

function get_scripts()
{
   var head = get_named_elements(document.documentElement, "head", true);
   if (head)
      return get_named_elements(head, "script");
   else
      return null;
}

function get_links()
{
   var head = get_named_elements(document.documentElement, "head", true);
   if (head)
      return get_named_elements(head,"link");
   else
      return null;
}

function transform_doc(xsl, xml)
{
   // Allow window.onload to return so we can access the new window.onload:
   function transform_complete()
   {
      setTimeout(initialize_new_page, 200);
   }

   // Save scripts for comparison
   var original_scripts = get_scripts();
   function load_scripts()
   {
      var sl = get_scripts();
      if (sl && sl[0]!=original_scripts[0])
         force_scripts_to_load(sl, transform_complete);
      else
         transform_complete();
   }

   function load_styles()
   {
      var sl = get_links();
      if (sl)
         force_sslinks_to_load(sl, load_scripts);
      else
         load_scripts();
   }


   if ("XSLTProcessor" in window)
   {
      var prc = new XSLTProcessor();
      if (prc)
      {
         prc.importStylesheet(xsl);
         var ddf = prc.transformToFragment(xml, document);
         if (ddf)
         {
            document.removeChild(document.documentElement);
            document.appendChild(ddf);
            load_styles();
         }
         else
         {
            var dser = new XMLSerializer();
            var xstr = dser.serializeToString(xsl.documentElement);
            var ppre = document.createElement("pre");
            document.body.appendChild(ppre);
            ppre.appendChild(document.createTextNode(xstr));
         }
      }
   }
   else
   {
      var ddoc = get_ie_domdoc();
      xml.transformNodeToObject(xsl.documentElement, ddoc);
      
      var onode = make_importable_node(document, ddoc, true);
      document.removeChild(document.documentElement);
      document.appendChild(onode);

      load_styles();
   }
}

function extract_stylesheet_url(pi)
{
   var data = pi.data;
   var dtype = pi.dataType;
   
   var re = /href\s*=\s*['"]([^'"]+)['"]/;
   var url = data.match(re);
   if (url)
      return url[1];
   else
      return null;
}

function get_xml_stylesheet(doc)
{
   var n = doc.firstChild;
   while (n)
   {
      if (n.nodeType==7 && n.target=="xml-stylesheet")
         return n;
      n = n.nextSibling;
   }
   return null;
}

function load_xml_doc(doc)
{
   document.XMLDocument = doc;

   // var sspi = doc.selectSingleNode("/processing-instruction('xml-stylesheet')");
   var sspi = get_xml_stylesheet(doc);
   if (sspi)
   {
      function finish(xsl)
      {
         document.XSLDocument = xsl;
         transform_doc(xsl,doc);
      }
      
      var url = extract_stylesheet_url(sspi);
      if (url)
         xhr_get_xsl(url, finish, xhr_error);
   }
}

function run_xdoc_loader(url)
{
   xhr_get(url,
           function(doc) { load_xml_doc(doc); },
           xhr_error);
}

