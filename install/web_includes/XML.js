var nsXHTML = "http://www.w3.org/1999/xhtml";
var nsXSL = "http://www.w3.org/1999/XSL/Transform";

// Global empty function to release XHR callback closures:
function empty(){};

// Declare framework functions:
// Test if browser can do XSL transforms;
function can_do_xsl() {}

// basic functions:
function getXMLDocs(callback)             {}
function getXHRObject()                   {}
function getNewDoc(url,callback,namespace){}
function getXMLDocument()                 {}
function getXSLDocument()                 {}

// XMLHttpRequest quick functions;
function xhr_get(url,callback,failed_callback,headers)      {}
function xhr_post(url,data,callback,failed_callback,headers){}
function xhr_default_req_header(name, value)                {}

// helper/convenience functions:
function parseXML(str,ns)             {}
function serialize(node)              {}
function get_namespace_map(doc)       {}
function prep_namespaces(doc)         {} // only useful in IE for SelectionNamespaces property
function escape_xml(str)              {}
function replace_node(oldn,newn)      {}
function add_namespace_el(tag,namespace,parent,before,doc) {};

// Especially if using transformed document, use the following to add to HTML document:
function addEl(tag,parent,before)   { return add_namespace_el(tag,nsXHTML,parent,before,document); }
function addText(text,el)           { el.appendChild(el.ownerDocument.createTextNode(text)); }


// Call to install implementations of above functions:
implement_functions();


// **** XSL class **** //
function XSL(doc, callback)
{
   if (!doc)
   {
      alert("Null XSL document.");
      return;
   }
   this.init(doc,callback);
}
XSL.prototype.init                = null;  // (doc, callback)
XSL.prototype.fix_stylesheet      = null;  // (callback)
XSL.prototype.find_node           = null;  // (xpath)
XSL.prototype.get_global_val_node = null;  // (name)
XSL.prototype.get_global_val      = null;  // (name)
XSL.prototype.set_global_val      = null;  // (name, value)
XSL.prototype.set_global_string   = null;  // (name, value)
XSL.prototype.transformFill       = null;  // (target, node)
XSL.prototype.transformInsert     = null;  // (container, node, before) node is appended to parent if *before* is omitted
XSL.prototype.transformReplace    = null;  // (target, node)
implement_XSL_methods();



// Flags to ensure prerequisites
var flag_select_node_prototypes = false;
var flag_xhr_functions = false;

function implement_functions()
{
   // functions here are used by following setters:
   prepare_helper_functions();
   
   // Set framework function implementations:
   getXHRObject = get_getXHRObject();
   prepare_xhr_functions();
   prepare_get_doc_functions();

   prepare_select_node_prototypes();
   prepare_getDocument_functions();
}

function prepare_helper_functions()
{
   var re_namespace_prefix = /^xmlns(:(.*))?$/;
   get_namespace_map = function(doc)
   {
      var de, map = {};
      if ((de=doc.documentElement) && de.attributes.length>0)
      {
         var arra = de.attributes;
         var match;
         for (var i=0,stop=arra.length; i<stop; ++i)
         {
            var a = arra[i];
            if ((match=re_namespace_prefix.exec(a.name)))
               map[match[2]] = a.value;
         }
      }
      return map;
   };

   function escape_xml(str)
   {

      function cb(str)
      {
         switch(str)
         {
         case "<": return "&lt;";
         case ">": return "&gt;";
         case "&": return "&amp;";
         case "'": return "&apos;";
         case '"': return "&quot;";
         default: return str;
         }
      }

      var re_xml = /[<>&\'\"]/g;
      
      escape_xml = function(str) { return str.replace(re_xml, cb); };
      return escape_xml(str);
   }

   replace_node = function(oldn, newn)
   {
      var parent = oldn.parentNode;
      parent.replaceChild(newn, oldn);
   };
   
   prep_namespaces = function(doc)
   {
      if ("setProperty" in doc)
      {
         var arr = [];
         var map = get_namespace_map(doc);
         var name;
         for (name in map)
         {
            var ns = map[name];
            var prefix = "xmlns" + (name.length?(":"+name):"");
            arr.push(prefix  + "='" + ns + "'");
         }
         if (arr.length)
            doc.setProperty("SelectionNamespaces",arr.join(' '));
      }
   };

   add_namespace_el = function (tag,namespace,parent,before,doc)
   {
      var n, d= doc || (parent?parent.ownerDocument:document);

      if ('createElementNS' in d)
	      n=d.createElementNS(namespace,tag);
      else if ('createNode' in d)
         n=d.createNode(1,tag,namespace);
      else
	      n=d.createElement(tag);

      if (parent)
         parent.insertBefore(n,before||null);
      
      return n;
   };
}

function prepare_xhr_functions()
{
   var ie_mode = ("ActiveXObject" in window) ? true : false;
   
   var def_headers = [
      {name:"Accept", value:"text/xml"},
      {name:"Content-Type", value:"application/x-www-form-urlencoded"}
   ];
   

   function find_header(arr, name)
   {
      for (var i=0, stop=arr.length; i<stop; ++i)
      {
         if (arr[i].name==name)
            return i;
      }

      return -1;
   }

   function load_req_headers(xhr, arr)
   {
      for (var i=0, stop=arr.length; i<stop; ++i)
      {
         var o = arr[i];
         xhr.setRequestHeader(o.name, o.value);
      }
   }

   function load_request_headers(xhr, headers)
   {
      load_req_headers(xhr, def_headers);
      if (headers)
         load_req_headers(xhr, headers);
   }

   xhr_default_req_header = function(name, value)
   {
      var ndx = find_header(def_headers, name);
      if (ndx<0)
      {
         if (value)
            def_headers.push({name:name, value:value});
      }
      else
      {
         if (value)
            def_headers[ndx].value = value;
         else
            def_headers.slice(ndx,1);
      }
   };
   
   function check_xhr(xhr, callback, cb_failed)
   {
      if (xhr.readyState==4)
      {
         // release closure:
         xhr.onreadystatechange=empty;
         var doc = xhr.responseXML;
         if (doc)
         {
            if (callback)
               callback(doc);
            else
               return doc;
         }
         else if (cb_failed)
            cb_failed(xhr);
         else
            alert(xhr.responseText);
      }
      return null;
   }

   xhr_get = function(url,cb,cb_failed,headers)
   {
      if (!cb_failed)
         cb_failed = function(xhr) { console.error("Failed to get document"); };
      
      var async = cb?true:false;
      var xhr = getXHRObject();
      if (xhr)
      {
         xhr.open("GET",url,async);
         load_request_headers(xhr,headers);
         if (ie_mode)
            xhr.responseType = "msxml-document";
         if (async)
            xhr.onreadystatechange = function() { return check_xhr(xhr,cb,cb_failed); };
         xhr.send(null);

         return async ? null : check_xhr(xhr,cb,cb_failed);
      }
      return null;
   };

   xhr_post = function(url,data,cb,cb_failed,headers)
   {
      var async = cb?true:false;
      var xhr = getXHRObject();
      if (xhr)
      {
         xhr.open("POST",url,async);
         load_request_headers(xhr,headers);
         if (ie_mode)
            xhr.responseType = "msxml-document";
         xhr.onreadystatechange = function() { return check_xhr(xhr,cb,cb_failed); };
         xhr.send(data);
         if (async)
            return null;
         else
            return check_xhr(xhr,cb,cb_failed);
      }
      return null;
   };


   flag_xhr_functions = true;
}

function prepare_get_doc_functions()
{
   if (!flag_xhr_functions) { throw "xhr_functions before get_getNewDoc"; }

   var doc_class = null;
   var dom_parser = ("DOMParser" in window) ? new DOMParser() : null;
   var dom_serializer = ("XMLSerializer" in window) ? new XMLSerializer() : null;

   function internal_get_doc()
   {
      var doc = null;
      var arr = ["MSXML2.DOMDocument.6.0",
                 "MSXML2.DOMDocument.4.0",
                 "MSXML.DOMDocument"];
      for (var i=0,stop=arr.length;i<stop;i++)
      {
         try
         {
            doc = new ActiveXObject(arr[i]);
            doc_class = arr[i];
            break;
         }
         catch(e){;}
      }
      return doc;
   }

   function internal_prep_xpath(doc)
   {
      if ("setProperty" in doc)
      {
         doc.setProperty("SelectionLanguage","XPath");
         prep_namespaces(doc);
      }   
   }
   
   if ("performance" in window)  // check for IE9+ which have dropped certain XML features
   {
      getNewDoc = function(url,cb,ns)
      {
         function int_cb(doc)
         {
            internal_prep_xpath(doc);
            cb(doc);
         }
         var ret = xhr_get(url, (cb?int_cb:null));
         if (ret && !cb)
            internal_prep_xpath(ret);
         return ret;
      };
   }
   else if ("ActiveXObject" in window)
   {
      getNewDoc = function(url,cb,namespace)
      {
         var doc = null;
         if (doc_class)
            doc = new ActiveXObject(doc_class);
         else
            doc = internal_get_doc();

         if (doc)
         {
            doc.async = false;
            doc.load(url);
            internal_prep_xpath(doc);
         }

         if (cb)
         {
            cb(doc);
            return null;
         }
         else
            return doc;
      };
   }

   if (dom_serializer)
   {
      serialize = function(node) { return dom_serializer.serializeToString(node); };
   }
   else
   {
      serialize = function(node) { return node.xml; };
   }

   
   if (dom_parser)
   {
      parseXML = function(str,ns) { return dom_parser.parseFromString(str); };
   }
   else if ("ActiveXObject" in window)
   {
      parseXML = function(str,ns)
      {
         var doc = internal_get_doc();
         doc.validateOnParse = false;
         if (str)
         {
            doc.loadXML(str);
            prep_namespaces(doc);
         }
         doc.setProperty("SelectionLanguage","XPath");
      };
   }

   can_do_xsl = function()
   {
      var doc;
      if ('XMLDocument' in window && 'XSLTProcessor' in window)
         return true;
      else if ((doc=internal_get_doc()) && 'transformNode' in doc)
         return true;
      return false;
   };

}


function get_getXHRObject()
{
   if ("XMLHttpRequest" in window)
   {
      return function() { return new XMLHttpRequest(); };
   }
   else if ("ActiveXObject" in window)
   {
      var xhr_class = null;

      return function()
      {
         if (xhr_class)
            return new ActiveXObject(xhr_class);
         else
            return find_xhr();
      };
      
      function find_xhr()
      { 
	      var arr = ["Msxml2.XMLHTTP.6.0",
		              "Msxml2.XMLHTTP.3.0",
		              "MSXML2.XMLHTTP",
		              "Microsoft.XMLHTTP"];
         
	      var r;
	      for (var i=0,stop=arr.length; i++; i<stop)
	      {
		      try
		      {
		         if ((r=new ActiveXObject(arr[i])))
		         {
                  xhr_class = arr[i];
			         return r;
		         }
		      }
		      catch(e) {;}
	      }
         
	      return null;
      }
   }

   return null;
}

function prepare_select_node_prototypes()
{
   if ("Element" in window
       && "XMLDocument" in window
       && !("selectSingleNode" in Element))
   {
      function resolve_namespace_from_doc(doc,prefix)
      {
         var map = get_namespace_map(doc);
         if (prefix in map)
            return map[prefix];
         else
            return "";
      }
      function get_eval_iter(node,xpath)
      {
         var doc = ("documentElement" in node) ? node : node.ownerDocument;
         function nsresolver(prefix) { return resolve_namespace_from_doc(doc,prefix); }
         return doc.evaluate(xpath,node,nsresolver,XPathResult.ANY_TYPE,null);
      }
      function int_selectSingleNode(xpath)
      {
         var iter = get_eval_iter(this,xpath);
         return iter.iterateNext();
      }
      function int_selectNodes(xpath)
      {
         var n, iter = get_eval_iter(this,xpath);
         var nodelist = [];
         while((n=iter.iterateNext()))
            nodelist.push(n);
         return nodelist;
      }

      Document.prototype.selectSingleNode =
      XMLDocument.prototype.selectSingleNode =
         Element.prototype.selectSingleNode = int_selectSingleNode;

      Document.prototype.selectNodes =
      XMLDocument.prototype.selectNodes =
         Element.prototype.selectNodes = int_selectNodes;
   }

   flag_select_node_prototypes = true;
}

function prepare_getDocument_functions()
{
   getXMLDocument = function()
   {
      if ("XMLDocument" in document)
         return document.XMLDocument;
      else
         return null;
   };
   getXSLDocument = function()
   {
      if ("XSLDocument" in document)
         return document.XSLDocument;
      else
         return null;
   };

   getXMLDocs = function _getXMLDocs(callback)
   {
      function call_callback()
      {
         if (callback)
            callback();
      }
      
      function have_xsl(doc)
      {
         document.XSLDocument = doc;
         call_callback();
      }
      
      function have_xml(doc)
      {
         document.XMLDocument = doc;
         var pixsl = doc.selectSingleNode("/processing-instruction('xml-stylesheet')");
         if (pixsl)
         {
            var re = /href\s*=\s*[\"\']([^\'\"]+)[\"\']/;
            var match = re.exec(pixsl.data);
            if (match && match.length>1)
               xhr_get(match[1], have_xsl, call_callback);
         }
         else
            call_callback();
      }

      
      function prep_for_ie(doc)
      {
         if ("setProperty" in doc)
         {
            doc.setProperty("SelectionLanguage","XPath");
            prep_namespaces(doc);
         }
      }
      
      if ("XMLDocument" in document)
      {
         prep_for_ie(document.XMLDocument);
         if ("XSLDocument" in document)
            prep_for_ie(document.XSLDocument);
         
         call_callback();
      }
      else
         xhr_get(window.location.href, have_xml, call_callback);
   };
}

function prepare_XSL_init(doc)
{
   function fix_output_element(doc)
   {
      var node;
      if ((node=doc.selectSingleNode("/xsl:stylesheet/xsl:output")))
      {
         if (node.getAttribute("output")=="xml")
            node.setAttribute("output","html");
         if (node.getAttribute("doctype-public"))
            node.removeAttribute("doctype-public");
         if (node.getAttribute("doctype-system"))
            node.removeAttribute("doctype-system");
      }
   }

   // webkit needs this, but other benefit from one-time import:
   function inject_imports(doc,callback)
   {
      var imports = null;
      var stylesheet = doc.selectSingleNode("/xsl:stylesheet");
      if (stylesheet)
         imports = stylesheet.selectNodes("xsl:import");

      if (!imports || imports.length==0)
      {
         if (callback)
            callback();
         return;
      }

      var import_index = 0;

      function get_next()
      {
         if (import_index < imports.length)
         {
            var ireq = imports[import_index++];
            var iurl = ireq.getAttribute("href");

            function cb(idoc) { process_import(idoc, ireq); }

            // Use getNewDoc to impose namespace:
            getNewDoc(iurl, cb, nsXSL);
         }
         else
         {
            // Finished with imports, remove original import nodes:
            while (import_index>0)
               stylesheet.removeChild(imports[--import_index]);

            // Recusively call inject_imports() in case
            // any imports have imports.  Callback will
            // be called when there are no more imports:
            inject_imports(doc, callback);
         }
      }

      var attribs = [ "name", "match", "mode" ];
      var alen = attribs.length;
      
      function process_import(idoc, importnode)
      {
         var nl = idoc.selectNodes("/xsl:stylesheet/*");

         // Insert every child that doesn't already exist in doc:
         for (var i=0, stop=nl.length; i<stop; ++i)
         {
            var node = nl[i];
            var name = node.tagName;
            
            // Make xpath to same-named nodes (homonyms) in doc:
            var xpath = "/xsl:stylesheet/" + name;
            for (var j=0; j<alen; ++j)
            {
               var testname = attribs[j];
               var aname = node.getAttribute(testname);

               if (aname)
               {
                  aname = aname.replace(/\'/g, '&apos;');
                  xpath += "[@" + testname + "='" + aname + "']";
               }
               else
                  xpath += "[not(@" + testname + ")]";
            }

            // Find any homonyms, only insert node if not already there:
            var homonyms = doc.selectNodes(xpath);
            if (homonyms.length==0)
               stylesheet.insertBefore(make_importable_node(doc,node,true),
                                       importnode);
         }

         get_next();
      }

      // processing starts here:
      get_next();
   }

   XSL.prototype.fix_stylesheet = function(callback)
   {
      fix_output_element(this.doc);
      inject_imports(doc, callback);
   };
}

// Implement XSL class methods:
function implement_XSL_methods()
{
   // Easy ones first:
   XSL.prototype.init = function(doc,callback)
   {
      if (XSL.prototype.fix_stylesheet==null)
         prepare_XSL_init(doc);
         
      prep_namespaces(doc);
      this.doc = doc;
      this.fix_stylesheet(callback);
   };

   XSL.prototype.fix_stylesheet
      = XSL.prototype.undo_root_one_fix
      = XSL.prototype.make_root_one_fix = null;

   XSL.prototype.find_node = function(xpath)
   {
      return this.doc.selectSingleNode(xpath);
   };
   XSL.prototype.get_global_val_node = function(name)
   {
      var xpath = "(/xsl:stylesheet/xsl:variable|/xsl:stylesheet/xsl:param)[@name='" + name + "']";
      return this.find_node(xpath);
   };
   XSL.prototype.get_global_val = function(name)
   {
      var node = this.get_global_val_node(name);
      if (node)
         return node.getAttribute("select");
      return null;
   };
   XSL.prototype.set_global_val = function(name,val)
   {
      // This function assumes 'val' is appropriately escaped for xml,
      // and can be used 'as is.'
      var node = this.get_global_val_node(name);
      if (node)
      {
         if (val || val==0)
            node.setAttribute("select",val);
         else
            node.removeAttribute("select");
      }
   };
   XSL.prototype.set_global_string = function(name,val)
   {
      // This function assumes 'val' is appropriately escaped for xml.
      var node = this.get_global_val_node(name);
      if (node)
      {
         if (val || val==0)
            node.setAttribute("select", "'" + val + "'");
         else
            node.removeAttribute("select");
      }
   };


   // Prepare transformFill methods:

   XSL.prototype.transformReplace = function(target,node)
   {
      var parent = target.parentNode;
      this.transformInsert(parent,node,target);
      parent.removeChild(target);
   };

   // Branch setup between standard and (old) IE versions:
   if ("XSLTProcessor" in window)
   {
      XSL.prototype.get_processor = function()
      {
         try
         {
            if (!this.processor)
            {
               var p = new XSLTProcessor();
               p.importStylesheet(this.doc);
               this.processor = p;
            }
            return this.processor;
         }
         catch(e) { alert("failed to import stylesheet: " + e.message); }

         return  null;
      };

      // google: webkit bug 28744 "root one"
      // "root_ one" fix is easy and safe even for firefox, which doesn't need it.
      // Checking for "root one" error complicated and likely fragile.
      var root_one_value = 'root_one_bridge';
      var root_one_xpath = "//*[@root_one='" + root_one_value + "']";
      var xpath_doc_node_template = "/xsl:stylesheet/xsl:template[@match='/' and not(@mode)]";

      // entry template should never be needed after page load:
      function gut_node(node)
      {
         var child;
         while ((child=node.lastChild))
            node.removeChild(child);
      }
      XSL.prototype.make_root_one_fix = function(node)
      {
         var doc = node.ownerDocument;
         var xpath_to_node = root_one_xpath;

         // Adjust if passed an attibute node:
         if (node.nodeType==2)
         {
            xpath_to_node += "/@" + node.name;
            node = node.ownerElement;
         }

         // mark <node> for easy retrieval:
         node.setAttribute("root_one", root_one_value);
         
         // make entry template jump to <node>
         var doc_node_template = this.doc.selectSingleNode(xpath_doc_node_template);
         gut_node(doc_node_template);
         var match = add_namespace_el("apply-templates",nsXSL,doc_node_template);
         match.setAttribute("select", xpath_to_node);
      };
      XSL.prototype.undo_root_one_fix = function(node)
      {
         if (node.nodeType==2)
            node = node.ownerElement;
         node.removeAttribute("root_one");
      };
      XSL.prototype.transformInsert = function(host,node,before)
      {
         var host_doc = (host.nodeType==9 ? host : host.ownerDocument);
         var source_doc, source_node;
         if (node.nodeType==9)
         {
            source_doc = node;
            source_node = node.documentElement;
         }
         else
         {
            source_doc = node.ownerDocument;
            source_node = node;
            this.make_root_one_fix(node);
         }
         // protect node in case of error:
         try
         {
            var ddf = this.get_processor().transformToFragment(source_doc, host_doc);
            if (before)
               host.insertBefore(ddf, before);
            else
               host.appendChild(ddf);
         }
         catch(e) { alert("transformToFragment failed: " + e.message); }

         if (node.nodeType!=9)
            this.undo_root_one_fix(node);
      };
      XSL.prototype.transformFill = function(host,node)
      {
         host.innerHTML = "";
         this.transformInsert(host,node);
      };
   }
   else
   {
      var doc = getNewDoc();
      if (doc && 'transformNode' in doc)
      {
         function last_child_el(node)
         {
            var n = node.lastChild;
            while (n)
            {
               if (n.nodeType==1)
                  return n;
               n = n.previousSibling;
            }
            return null;
         }

         function easy_copy(target, str)
         {
            var p = target.parentNode;
            p.innerHTML += str;
            var lc = last_child_el(p);
            if (lc)
            {
               var old = p.replaceChild(lc, target);
               p.removeChild(old);
            }
         }

         // initilize once, when loaded
         var body = document.getElementsByTagName("body")[0];
         var reFirstTag = /^[^<]*<([a-z0-9]+)(\s|\/|>)/i;

         function find_first_tag(str)
         {
            var matches = reFirstTag.exec(str);
            if (matches)
               return matches[1];
            else
               return null;
         }

         function ie_empty_node(node)
         {
            try
            {
               node.innerHTML = "";
            }
            catch(x)
            {
               var child = node.firstChild;
               while (child)
               {
                  var doomed = child;
                  child = child.nextSibling;
                  node.removeChild(doomed);
               }
            }
         }
         
         function ie_insert_node(parent, str, before)
         {
            if (!str)
               return;

            var p_tag, tag=(tag=find_first_tag(str))?tag.toLowerCase():null;
            switch(tag)
            {
            case "tbody":
               str = "<table>" + str + "</table>";
               p_tag = "table";
               break;
            case "tr":
               str = "<table><tbody>" + str + "</tbody></table>";
               p_tag = "tbody";
               break;
            case "td":
               str = "<table><tr>" + str + "</tr></table>";
               p_tag = "tr";
               break;
            }

            var node;
            var body = document.body;
            var div = addEl("div",body);
            div.innerHTML = str;

            if ((p_tag && (node=div.getElementsByTagName(p_tag)[0])) || (node=div))
            {
               // pre-condition add function:
               var add = void(0);
               if (before)
                  add = function(n) {parent.insertBefore(n,before);};
               else
                  add = function(n) {parent.appendChild(n);};

               var saved, child = node.firstChild;
               while(child)
               {
                  // adding to DOM will invalidate saved
                  saved = child;
                  child = child.nextSibling;
                  add(saved);
               }
            }

            body.removeChild(div);
         }
         XSL.prototype.transformFill = function(target,node)
         {
            try
            {
               var str = node.transformNode(this.doc);
               ie_empty_node(target);
               ie_insert_node(target, str);
            }
            catch (x) { alert(x.message); }
         };
         XSL.prototype.transformInsert = function(container,node,before)
         {
            var str = node.transformNode(this.doc);
            ie_insert_node(container,str,before);
         };
      }
   }
}

function make_importable_node(target_doc, source, deep)
{
   function copy_attributes(target, source)
   {
      var a = source.attributes;
      for (var i=0,stop=a.length;i<stop;i++)
         {
            var at = a[i];
            target.setAttribute(at.name, at.value);
         }
   }

   function copy_element(doc, source, deep)
   {
      // Likely only used by IE so code written to save time in IE;
      var name = source.baseName || source.localName;
      
      var el = add_namespace_el(name,
                                source.namespaceURI,
                                null,null,doc);
      
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

   if ("importNode" in target_doc)
   {
      make_importable_node = function(target_doc, source, deep)
      {
         return target_doc.importNode(source, deep);
      };
   }
   else
   {
      make_importable_node = function(target_doc, source, deep)
      {
         return copy_element(target_doc, source, deep);
      };
   }

   return make_importable_node(target_doc, source, deep);
}


