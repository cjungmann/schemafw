var SchemaFW = { process_event        : function(e,t) { return true; },
                 process_button       : function(e,t) { return true; },
                 default_delete       : function(url, bundle) { },
                 fix_table_heads      : function(table) { },
                 new_context          : function(url, bundle, callback) { },
                 
                 get_form_data        : function(form) { },
                 focus_on_first_field : function(dlg) { },
                 
                 px                   : function(el) { },

                 set_xmldoc           : function(doc) { },
                 set_xslobj           : function(xslo) { },

                 get_xslobj           : function() { return null; },

                 report_message       : function(msg_el) { },
                 get_cached_docel     : function(url, callback) { callback(); }
               };


function InitializeSchemaFW(xmld, xslo)
{
   var xmlDoc = xmld;
   var xslObj = xslo;
   var content_host = get_content_host();

   var cached_docels = {};
   
   var current_event_handler = bogus_event_handler;

   var default_delete_xpath = "/*result[row[@delete]]/row";
   
   // Set SchemaFW functions:
   SchemaFW.process_event = _process_event;
   SchemaFW.process_button = function(e,t)
   {
      if (e.type=="click")
         return _process_button(e,t);
      else
         return true;
    };
   SchemaFW.default_delete = _default_delete;
   SchemaFW.fix_table_heads = _fix_table_heads;
   SchemaFW.new_context = _new_context;

   SchemaFW.get_form_data = get_form_data;
   SchemaFW.focus_on_first_field = focus_on_first_field;
   SchemaFW.px = px;

   SchemaFW.set_xmldoc = function(doc)
   {
      xmlDoc = doc;
      initialize_current_event_handler();
   };
   
   SchemaFW.set_xslobj = function(ob)  { xslObj = ob; };

   SchemaFW.get_xslobj = function()    { return xslObj; };

   SchemaFW.report_message = report_message;
   SchemaFW.get_cached_docel = _get_cached_docel;

   function px(n) { return String(n)+"px"; }
   
   function focus_on_first_field(dlg)
   {
      var nl = dlg.getElementsByTagName("label");
      if (nl && nl.length>0)
      {
         var label = nl[0];
         var name = label.getAttribute("for");
         function f(n) { return n.getAttribute("name")==name; }
         var el = getFirstMatchingEl(label.parentNode, f);
         if (el)
         {
            el.focus();
            if (el.getAttribute("type")=="text")
            {
               el.selectionStart = 0;
               el.selectionEnd = el.value.length;
            }
         }
      }
   }

   function get_next_el(el)
   {
      var f = null;
      if ('nextElementSibling' in el)
         f = function(e) { return e.nextElementSibling; };
      else
      {
         f = function(e)
         {
         };
      }

      get_next_el = f;
      return f(el);
   }

   function get_previous_el(el)
   {
      var f = null;
      if ('previousElementSibling' in el)
         f = function(e) { return e.previousElementSibling; };
      else
      {
         f = function(e)
         {
            while ((e=e.previousSibling))
               if (e.nodeType==1)
                  return e;
            return null;
         };
      }
      get_previous_el = f;
      return f(el);
   }

   function get_first_el(host)
   {
      var f = null;
      if ('firstElementChild' in host)
         f = function(h) { return h.firstElementChild; };
      else
      {
         f = function(h)
         {
            var n = h.firstChild;
            if (!n) return null;
            if (n.nodeType==1) return n;
            return get_next_el(n);
         };
      }
      get_first_el = f;
      return f(host);
   }

   function get_last_el(host)
   {
      var f = null;
      if ('lastElementChild' in host)
         f = function(h) { return h.lastElementChild; };
      else
      {
         f = function(h)
         {
            var n = h.lastChild;
            if (!n) return null;
            if (n.nodeType==1) return n;
            return get_previous_el(n);
         };
      }
      get_last_el = f;
      return f(host);
   }

   function _get_row_id(new_row,schema)
   {
      if (schema)
      {
         var f;
         if (!(f=schema.selectSingleNode("field[@line_id]")))
            f = schema.selectSingleNode("field[@primary-key]");
         if (f)
            return new_row.getAttribute(f.getAttribute("name"));
      }

      var a = new_node.firstChild;
      while (a)
      {
         if (a.nodeType==2)
            return a.value;
         a = a.nextSibling;
      }
      return null;
   }

   function _get_cached_docel(url, callback)
   {
      if (url in cached_docels)
         callback(cached_docels[url]);
      else
      {
         function got_doc(doc)
         {
            var docel = doc.documentElement;
            if (docel.tagName=="message")
            {
               report_message(docel);
               callback(null);
            }
            else
            {
               cached_docels[url] = docel;
               callback(docel);
            }
         }

         function failed(xhr)
         {
            process_xhr_get_error(xhr,url);
            callback(null);
         }
         xhr_get(url, got_doc, failed);
      }
   }

   function report_message(msg_el)
   {
      var type = msg_el.getAttribute("type");
      var msg = msg_el.getAttribute("message");
      var detail = msg_el.getAttribute("detail");

      if (type=="error")
         console.error(msg + (detail?(" "+detail):""));
      else
         console.log(type + " message: " + msg + (detail?(" "+detail):""));
   }
   
   if (xmlDoc)
      initialize_current_event_handler();

   // Only function definitions follow here, this (should)
   // effectively end the execution of InitializeSchemaFW().

   function initialize_current_event_handler()
   {
      var docel = xmlDoc.documentElement;
      var schema = docel.selectSingleNode("schema");
      if (!schema)
         schema = docel.selectSingleNode("result/schema");

      var asform = false;

      if (docel.hasAttribute("mode-type"))
         asform = docel.getAttribute("mode-type").substring(0,4)=="form";
      else if (schema && schema.getAttribute("form-action"))
         asform = true;

      if (asform)
         current_event_handler = form_event_handler;
      else
         current_event_handler = table_event_handler;
   }
   
   function el_return_or_sibling(n)
   {
      while (n)
      {
         if (n.nodeType==1)
            break;
         n = n.nextSibling;
      }
      return n;
   }

   function nextSiblingEl(node) { return el_return_or_sibling(node.nextSibling); }
   function firstChildEl(node)  { return el_return_or_sibling(node.firstChild); }

   function get_content_host()
   {
      var el = document.getElementById("SFW_Content");
      if (!el)
         el = document.body;
      return el;
   }

   function resize_sfw_content()
   {
      if (content_host)
      {
         var ohigh, owide, high=0, wide=0;
         var c = get_first_el(content_host);
         while (c)
         {
            ohigh = c.offsetHeight;
            owide = c.offsetWidth;
            if (ohigh > high)
               high = ohigh;
            if (owide > wide)
               wide = owide;
            c = get_next_el(c);
         }

         content_host.style.height = String(wide) + "px";
      }
   }

   function resize_to_fill_sfw_content(el)
   {
      if (content_host)
      {
         el.style.height = String(content_host.offsetHeight)+"px";
         el.style.width = String(content_host.offset_width)+"px";
      }
   }


   function get_sfw_host(t)
   {
      while(t.nodeType!=9)
      {
         if (t.nodeType==1 && class_includes(t,"SFW_Host"))
            return t;
         t = t.parentNode;
      }
      return null;
   }

   function get_base_element(t)
   {
      while (t)
      {
         switch(t.nodeType)
         {
            case 9:  return null;
            
            case 1:
               var tname = t.tagName.toLowerCase();
               if (tname=="table" || tname=="form")
                  if (t.getAttribute("data-result-type"))
                     return t;
               break;
            
            default: break;
         }

         t = t.parentNode;
      }
      return null;
   }

   function remove_from_parent(el)
   {
      if (el)
      {
         var p = el.parentNode;
         if (p)
            p.removeChild(el);
      }
   }

   // search siblings before children
   function getFirstMatchingEl(start, match_func)
   {
      function ssibs(n)
      {
         if (n)
         {
            if (match_func(n))
               return n;
            else
               return ssibs(nextSiblingEl(n));
         }
         return n;
      }

      function search(base)
      {
         var fchild = firstChildEl(base);
         var n = ssibs(fchild);
         if (!n)
         {
            var i = fchild;
            while (i && !(n=search(i)))
               i = nextSiblingEl(i);
         }
         return n;
      }

      return search(start);
   }

   
   // Build and alert if doc is a notice response.
   // Return true if it is a notice, false if not.
   function alert_notice(doc)
   {
      var type, msg, where;
      
      var el = doc.documentElement;
      var ln = el.localName;
      if (ln=="notice")
      {
         type  = el.getAttribute("type");
         msg   = el.getAttribute("msg");
         where = el.getAttribute("where");
      }
      else if (ln=="message")
      {
         type = el.getAttribute("type");
         msg = el.getAttribute("message");
      }
      else if ((el=el.selectSingleNode("message")))
      {
         type = el.getAttribute("type");
         msg = el.getAttribute("message");
         where = el.getAttribute("detail");
      }
      
      var str = "";
      
      if (type)
      {
         if (type=="error")
            str += "Error";
         else if (type=="message")
            str += "Message";
         else
            str += type;
      }

      if (msg)
      {
         str += "\n\n";
         str += msg;
      }
      
      if (where)
      {
         str += "\n\n";
         str += where;
      }

      if (str.length)
      {
         alert(str);
         return true;
      }
      else
         return false;
   }

   function translate_url(url, xmldoc)
   {
      var docel = xmldoc.documentElement;

      var refs = docel.selectNodes("*[@type='ref']");
      if (!refs || refs.length==0)
         return url;

      function cb(match)
      {
         var rnode, xpath = "*[@type='ref']/*[" + match + "]";
         if ((rnode=docel.selectSingleNode(xpath)))
         {
            var val = rnode.getAttribute(match.substring(1));
            return encodeURIComponent(val);
         }
         else
            return match;
      }

      url = url.replace(/@[a-z0-1_-]+/, cb);
      return url;
   }

   // Beginning of event handler section, supporting _process_event() which
   // concludes the section.

   function get_keycode_from_event(e) { return e.charCode || e.keyCode || e.which; }
   
   function bogus_event_handler(e,t,top) { return true; }

   function form_event_handler(e,t,top)
   {
      if (!top)
         return true;

      // For now, only handle clicks (keyboard handled earlier):
      if (e.type!="click")
         return true;

      // Allow form to be a normal form, so only handle
      // click events on a button element:
      if (t.type=="button")
      {
         if (!process_button_click(e,t,top,null))
            return false;
      }
      
      return true;
   }

   function make_bundle(t,top)
   {
      var resulttype = top.getAttribute("data-result-type");
      var resultpath = top.getAttribute("data-result-path");

      var doc;
      if ("xmldoc" in top)
         doc = top.xmldoc;
      else
         doc = xmlDoc;

      var schema, result = doc.selectSingleNode(resultpath);
      if (!result || !(schema=result.selectSingleNode("schema")))
         return null;
      else
         return { source : top,
                  table  : top,
                  result : result,
                  schema : schema,
                  url      : null,
                  xml_node : null,
                  html_row : null,
                  row_host : null
                };
   }

   function resolve_line_references(node, url)
   {
      var re = /\{![^\}]+\}/;
      function f(match)
      {
         var ref = match.substring(2, match.length-1);
         var val = node.getAttribute(ref);
         if (val)
            return encodeURIComponent(val);
         else
         {
            console.error("Unable to find attribute '" + ref + "' in the selected node.");
            return "";
         }
      }
      
      return url.replace(re, f);
   }

   function process_tr_click(bndl, tr)
   {
      if (!bndl.table)
         console.error("Table missing in bundle...aborting.");
      else
      {
         var id = tr.getAttribute("data-id");
         var url = bndl.table.getAttribute("data-on_line_click");
         if (url)
         {
            if (url.endsWith('='))
               url += id;
            else if (!url.includes('&'))
               url += ('='+id);
         }
         else
         {
            console.warn("Failed to find @on_line_click, searching elsewhere.");
            url = get_line_click_url(bndl, tr);
         }
            
         if (url)
         {
            var node = get_row_with_id(bndl.result,id);
            
            bndl.html_row = tr;
            bndl.row_host = tr.parentNode;
            bndl.xml_node = node;
            bndl.url = resolve_line_references(node,url);

            start_dialog(bndl);
            return false;
         }
      }
      return true;
   }

   function delay_open(rowid, rowhost, table)
   {
      function f()
      {
         // find the referenced html row:
         var row = null;
         var r = get_first_el(rowhost);
         while (r)
         {
            if (r.getAttribute("data-id")==rowid)
            {
               row = r;
               break;
            }
            r = get_next_el(r);
         }

         if (row)
         {
            var bndl = make_bundle(row, table);
            if (bndl)
               process_tr_click(bndl, row);
         }
      }

      var t = setTimeout(f, 250);
   }

   function table_event_handler(e,t,top)
   {
      // Only handle for schema-hosted items.
      if (!top)
         return true;

      // For now, only handle clicks (keyboard handled earlier):
      if (e.type!="click")
         return true;

      // If fails to make a bundle, return TRUE to continue default processing:
      var bndl = make_bundle(t,top);
      if (!bndl)
         return true;

      var doc = ("xmldoc" in top) ? top.xmldoc : xmlDoc;

      while (t && t!=top)
      {
         var tag = t.tagName.toLowerCase();
         switch(tag)
         {
            case "button":
            {
               if (t.getAttribute("data-type")=="add")
               {
                  bndl.url = translate_url(t.getAttribute("data-task"), doc);
                  bndl.row_host = top.getElementsByTagName("tbody")[0];
                  start_dialog(bndl);
                  return false;
               }
               break;
            }
            case "tr":
               return process_tr_click(bndl, t);

            case "th":
            {
               if ((class_includes(t,'sortable')))
               {
                  if (result_is_free_to_replot(bndl.result))
                  {
                     set_sort_column(bndl.schema, t);
                     replot_table_contents(top, bndl.result);
                     return false;
                  }
               }
               break;
            }
         }

         
         t = t.parentNode;
      }

      return true;
   }

   function _process_event(e,t)
   {
      return current_event_handler(e,t,get_base_element(t));
   }

   function _process_button(e,t)
   {
      if (t.tagName.toLowerCase()=="button")
         return process_button_click(e,t,null,null);

      return true;
   }
   
   function get_row_with_id(result, id)
   {
      var idname, schema, xpath = "*[@id=" + id + "]";
      if ((schema=result.selectSingleNode("schema")))
      {
         if (!(idname=schema.getAttribute("line_click_id")))
            idname = "id";
         xpath = schema.getAttribute("name") + "[@" + idname + "=" + id + "]";

         return result.selectSingleNode(xpath);
      }
      
      return null;
   }


   function getPageXOffset()
   {
      if ('pageXOffset' in window)
         getPageXOffset = function() { return window.pageXOffset; };
      else if ('body' in document && 'scrollLeft' in document.body)
         getPageXOffset = function() { return document.body.scrollLeft; };
      else
         getPageXOffset = void(0);
      
      return getPageXOffset();
   }

   function getPageYOffset()
   {
      if ('pageYOffset' in window)
         getPageYOffset = function() { return window.pageYOffset; };
      else if ('body' in document && 'scrollTop' in document.body)
         getPageYOffset = function() { return document.body.scrollTop; };
      else
         getPageYOffset = void(0);
      
      return getPageYOffset();
   }

   function get_doc_offset(el)
   {
      if (el===null)
         return {left:0, top:0};
      
      var os = el.offsetParent ? get_doc_offset(el.offsetParent) : {left:0,top:0};
      return {left:os.left+el.offsetLeft, top:os.top+el.offsetTop };
   }

   function get_relative_offset(target, ref)
   {
      var ost = get_doc_offset(target);
      var osr = get_doc_offset(ref);
      ost.left -= osr.left;
      ost.top -= ost.top;
      return ost;
   }

   function _default_delete(url,bundle)
   {
      function cb(xmldoc)
      {
         if (!alert_notice(xmldoc))
         {
            var dnode = xmldoc.selectSingleNode("/*/result[row[@deleted]]/row");
            if (dnode)
            {
               var dcount = Number(dnode.getAttribute("deleted"));
               if (dcount>0)
               {
                  remove_from_parent(bundle.html_row);
                  remove_from_parent(bundle.xml_node);
                  if ("table" in bundle)
                  {
                     _fix_table_heads(bundle.table);
                     _set_changed_attribute(bundle.table);
                  }
               }
            }
         }

         if ("form_close" in bundle)
            bundle.form_close();
      }
      function cb_failed(xhr)
      {
         process_xhr_get_error(xhr,url);
         alert("Record not deleted.");
      }
      xhr_get(url, cb, cb_failed);
   };

   function _set_changed_attribute(table)
   {
      console.log("setting table's data-changed attribute.");
      table.setAttribute("data-changed", "true");
   }
   
   // For paired ref and float header rows, size and position the floater
   // to cover the ref row, but make the floater fixed to the page so the
   // table scrolls under the floater.
   //
   // This function contains many closure-scope functions, and will be
   // replaced at the global scope with a closure function that will take
   // its place.
   function _fix_table_heads(table)
   {
      var sizer_tr = {left:null, top:null, wide:null, high:null};
      var sizer_td = {left:null, top:null, wide:null, high:null};
      var sizer_th = {left:null, top:null, wide:null, high:null};
      
      function is_tr_el(node)
      {
         return node.nodeType==1 && node.tagName.toLowerCase()=="tr";
      }

      function is_th_el(node)
      {
         return node.nodeType==1 && node.tagName.toLowerCase()=="th";
      }

      // search nextSiblings for match
      function find_matched_headfix_row(tr, hf_name)
      {
         while ((tr=tr.nextSibling))
            if (is_tr_el(tr))
               if (class_includes(tr,hf_name))
                  return tr;

         return null;
      }

      function reposition_element(ref, flo, sizer)
      {
         var ref_par = ref.parentNode;
         var os = get_relative_offset(ref, ref_par);
         flo.style.left = px(os.left + sizer.left);
         flo.style.top = px(os.top + sizer.top);
      }

      function resize_element(ref, flo, sizer)
      {
         flo.style.height = px(ref.offsetHeight + sizer.high);
         flo.style.width = px(ref.offsetWidth + sizer.wide);
      }

      function calculate_sizer(ref, flo, sizer)
      {
         sizer.left = ref.clientLeft - flo.clientLeft;
         sizer.top = ref.clientTop - flo.clientTop;

         sizer.wide = ref.clientWidth - flo.clientWidth;
         sizer.high = ref.clientHeight - flo.clientHeight;
      }

      function align_cells(ref, flo)
      {
         var r,f, i;

         r = firstChildEl(ref);
         f = firstChildEl(flo);

         if (!r || !f)
            return;

         var sizer = null;
         switch(r.tagName.toLowerCase())
         {
            case "th": sizer = sizer_th; break;
            case "td": sizer = sizer_td; break;
         }

         if (sizer.wide==null)
         {
            reposition_element(r,f,sizer);
            resize_element(r,f,sizer);
            calculate_sizer(r,f,sizer);
         }

         while (r && f)
         {
            reposition_element(r,f,sizer);
            resize_element(r,f,sizer);

            r = nextSiblingEl(r);
            f = nextSiblingEl(f);
         }
      }

      function align_pair(ref, flo)
      {
         var sizer = null;
         switch(ref.tagName.toLowerCase())
         {
            case "tr": sizer = sizer_tr; break;
            case "th": sizer = sizer_th; break;
            case "td": sizer = sizer_td; break;
         }
         
         var osref = get_doc_offset(ref);
         flo.style.left = px(osref.left);
         flo.style.top = px(osref.top);
         resize_element(ref,flo,sizer);

         if (sizer.high===null)
         {
            calculate_sizer(ref,flo,sizer);

            if (sizer.left || sizer.top)
               reposition_element(ref,flo,sizer);
            if (sizer.wide || sizer.high)
               resize_element(ref,flo,sizer);
         }

         var first = firstChildEl(ref);
         if (first)
         {
            var ftag = first.tagName.toLowerCase();
            if (ftag=="th" || ftag=="td")
               align_cells(ref, flo);
         }
      }

      var re_headfix = /headfix_\S+/;
      function align_paired_rows(thead)
      {
         var rematch, trmatch;
         var currow = firstChildEl(thead);
         while (currow)
         {
            if (is_tr_el(currow))
               if ((match=currow.className.match(re_headfix)))
                  if ((trmatch=find_matched_headfix_row(currow, match[0])))
                  {
                     if (class_includes(currow,"floater"))
                        align_pair(trmatch,currow,sizer_tr);
                     else
                        align_pair(currow,trmatch,sizer_tr);
                  }
            currow = nextSiblingEl(currow);
         }
      }

      // replace function with function in closure:
      _fix_table_heads = function(tab)
      {
         var thead = tab.getElementsByTagName("thead");
         if (thead && thead.length)
            align_paired_rows(thead[0]);
      };

      // must explicitely call the function when setting up the closure:
      _fix_table_heads(table);
      
   }  // end of _fix_table_heads


   function position_dialog(dlg, ref)
   {
      var html = document.documentElement;
      var h_dlg = dlg.offsetHeight;
      var w_dlg = dlg.offsetWidth;
      var h_window = html.clientHeight;
      var w_window = html.clientWidth;

      var left = w_window/2 - w_dlg/2;
      var top = h_window/2 - h_dlg/2;

      if (ref)
      {
         var os_ref = get_doc_offset(ref);
         var h_ref = ref.offsetHeight;
         
         if (h_dlg + os_ref.top + h_ref < h_window)
            top = os_ref.top + h_ref;
      }

      dlg.style.left = px(left);
      dlg.style.top = px(top);
   }

   function get_sort_column(schema)
   {
      var xpath = "field[@sorting]";
      return schema.selectSingleNode(xpath);
   }

   function set_sort_column(schema, th)
   {
      var fieldname = th.getAttribute("data-name");
      var xpath_new_sort = "field[@name='" + fieldname + "']";
      var node;

      if ((node=get_sort_column(schema)))
         node.removeAttribute("sorting");

      if ((node=schema.selectSingleNode(xpath_new_sort)))
      {
         node.setAttribute("sorting", "true");
         return true;
      }
      else
         return false;
   }

   function result_is_free_to_replot(result)
   {
      var a = result.getAttribute("make_table_body");
      return a==null;
   }

   function replot_table_contents(table, result)
   {
      var xsl = xslObj;
      if ("xslobj" in table)
         xsl = table.xslobj;
      
      var tbody = table.getElementsByTagName("tbody")[0];
      if (tbody)
      {
         result.setAttribute("make_table_body", "true");
         xsl.transformFill(tbody, result);
         result.removeAttribute("make_table_body");
         _fix_table_heads(table);
      }
   }

   function process_button_call(t,bundle)
   {
      var task = t.getAttribute("data-task");
      if (!task)
         console.error("'call' button without task setting.");
      else if (!(task in window))
         console.error("'call' button task not defined.");
      else
         window[task](bundle);
   }

   function process_button_delete(t,bundle)
   {
      var delete_it = false;
      
      var msg = t.getAttribute("data-confirm");
      if (msg)
         delete_it = window.confirm(msg);
      else
         delete_it = true;

      if (delete_it)
      {
         var url = t.getAttribute("data-task");
         if (url)
            _default_delete(url, bundle);
      }
   }

   function process_button_jump(t)
   {
      var url = t.getAttribute("data-url");
      if (!url)
         console.error("'jump' button without URL.");
      else
         window.location = url;
   }

   function process_button_open(t,bundle)
   {
      var url = t.getAttribute('data-url');
      if (url)
      {
         _new_context(url, null, null, null);
      }
   }

   function object_has_props(o)
   {
      var p;
      for (p in o)
         return true;
      return false;
   }

   function find_field_parts(t)
   {
      var r = {};
      var p = t.parentNode;
      var n = p.firstChild;
      while(n)
      {
         if (n.nodeType==1)
         {
            var tn = n.tagName.toLowerCase();
            switch(tn)
            {
               case "label":
                  r.label = n;
                  break;
               case "div":
                  if (n.className=="field_content")
                     r.content = n;
                  break;
               case "button":
                  r.button = n;
                  break;
            }
         }
         n = n.nextSibling;
      }

      if (object_has_props(r))
         return r;
      else
         return null;
   }

   function replace_field_related_result(field,div)
   {
      var url, rname;
      
      function cbfailed(text)
      {
         console.error("Invalid document from http://" + url);
      }
      function cb(newdoc)
      {
         var premsg = "Unable to find result from the ";
         var oldresult, newresult;
         var xpath = "/*/" + rname + "[@rndx]";
         if (!(oldresult=field.ownerDocument.selectSingleNode(xpath)))
            console.error(premsg + "original document");
         else if (!(newresult=newdoc.selectSingleNode(xpath)))
            console.error(premsg + "new document");
         else
         {
            replace_node(oldresult, newresult);
            xslo.transformReplace(div,field);
         }
      }

      if ((url=div.getAttribute("data-update"))
          && (rname=div.getAttribute("data-result")))
      {
         xhr_get(url, cb, cbfailed);
      }
      else
      {
         console.error("component field refresh requires data-update and data-result attributes.");
      }
   }

   function process_button_view(t,bundle)
   {
      var emsg = "Missing in process_button_view: ";
      var schema_xpath = "(/*/schema)|(/*/*[@rndx=1]/schema)";
      var form, parts, schema, field=null;
      
      if (!(form=get_base_element(t)))
         console.error(emsg + "form");
      else if (!(parts=find_field_parts(t)))
         console.error(emsg + " field parts");
      else if (!(schema=form.xmldoc.selectSingleNode(schema_xpath)))
         console.error(emsg + " form schema");
      else
      {
         if ("label" in parts)
         {
            var fxpath = "field[@name='" + parts.label.getAttribute("for") + "']";
            field = schema.selectSingleNode(fxpath);
         }
      }
         
      var cb_update = null;

      var content;
      if (field && (content=parts.content))
      {
         cb_update = function(guest)
         {
            var tn = guest.tagName.toLowerCase();
            if (tn=="table" && guest.getAttribute("data-changed"))
            {
               replace_field_related_result(field, content);
            }
         };
      }

      // Add a callback parameter to _new_context
      // that will be called when the context ends.
      // We also should track if an update is necessary,
      // and if so, provide a mode with which the
      // field-content element can be refilled.
      
      var url = t.getAttribute("data-url");
      if (url)
         _new_context(url, bundle, null, cb_update);
   }

   function process_button_click(e,t,top,bundle)
   {
      var type = t.getAttribute("data-type");
      switch(type)
      {
         case "delete":
            return process_button_delete(t,bundle);
         case "call":
            process_button_call(t,bundle);
            return false;
         case "jump":
            process_button_jump(t);
            return false;
         case "open":
            process_button_open(t,bundle);
            return false;
         case "view":
            process_button_view(t,bundle);
            return false;
         default:
            console.error("No provision for button type \"" + type + "\"");
      }
      return true;
   }

   function get_value_from_vars(name, result)
   {
      var rval = null;
      var docel = result.ownerDocument.documentElement;
      var nl = docel.selectNodes("result[@type='variables']/*[1]");
      for (var i=0, stop=nl.length; !rval && i<stop; ++i)
         rval = nl[i].getAttribute(name);

      return rval;
   }

   function get_value_from_ref(name, result)
   {
      var xpath = "../*[@type='ref']/*[" + name + "]";
      var nl = result.selectNodes(xpath);
      
      for (var i=0, stop=nl.length; i<stop; ++i)
         return nl[i].getAttribute(name.substring(1));

      return null;
   }

   function get_value_from_path(path, result)
   {
      node = doc.selectSingleNode(path);
      if (node)
         return node.value;
      else
         return null;
   }

   // @ get attribute of tr, * and % searches for td with matching
   // className (*) or index(%) and returns its text contents.
   function get_value_from_line(name, tr)
   {
      var td = null;
      var cfunc = null;
      var index=-1, sub = null;
      switch(name[0])
      {
         case "@":
            return tr.getAttribute(name.substring(1));
         case "*":
            name = name.substring(1);
            cfunc = function(t) { return t.className==name; };
            break;
         case "%":
            sub = Number(name.substring(1));
            cfunc = function(t) { return sub==index; };
            break;
      }

      if (cfunc)
      {
         var nl = tr.getElementsByTagName("td");
         var stop = nl.length;
         for (index=0, td=nl[index]; index<stop; ++index)
            if (cfunc(td))
               return td.firstChild.data;
      }

      return null;
   }

   function get_value_from_node(name, result, tr)
   {
      var node = get_row_with_id(result, tr.getAttribute("data-id"));
      if (name[0]=="@")
         return node.getAttribute(name.substring(1));
      else
         return node.getAttribute(name);
   }

   function get_param_from_element(el, result, tr)
   {
      if (!el.getAttribute("name"))
         return null;

      var rval = "&" + el.getAttribute("name");
      var attrs = el.attributes;
      var id = tr.getAttribute("data-id");
      var val = null;
      for (var i=0,stop=attrs.length; !val && i<stop; ++i)
      {
         var attr = attrs[i];
         var filter = attr.value;
         switch(attr.name)
         {
            case "name":
               break;
            case "ref-value":
               val = get_value_from_vars(filter, result);
               break;
            case "value":
               val = filter;
               break; 
            case "ref":
               val = get_value_from_ref(filter, result);
               break;
            case "line":
               val = get_value_from_line(filter, tr);
               break;
            case "path":
               val = get_value_from_path(filter, result);
               break;
            case "node":
               val = get_value_from_node(filter, result, tr);
               break;
         }
      }

      if (val)
         return "&" + el.getAttribute("name") + "=" + encodeURIComponent(val);
      else
         return "";
   }

   function get_line_click_element(schema, docel)
   {
      var node_olc = null;
      if (!(schema && (node_olc=schema.selectSingleNode("on_line_click"))))
         if (!(node_olc=schema.parentNode.selectSingleNode("on_line_click")))
            node_olc=docel.selectSingleNode("on_line_click");
      return node_olc;
   }

   function get_attribute_value(aname, node, schema, docel)
   {
      var rval = node ? node.getAttribute("on_line_click") : null;
      if (!rval)
         rval = schema ? schema.getAttribute(aname) : null;
      if (!rval)
         rval = docel ? docel.getAttribute(aname) : null;
      return rval;
   }

   function get_line_click_attribute(schema, docel)
   {
      var url = null;
      if (!(schema && (url=schema.getAttribute("on_line_click"))))
            url = docel.getAttribute("on_line_click");
      return url;
   }

   function get_line_click_id_name(schema, docel)
   {
      var name = null;
      if (!(schema && (name=schema.getAttribute("line_click_id"))))
         name = docel.getAttribute("line_click_id");
      return name;
   }


   // Looks for more flexible on_line_click element first, then for
   // simplified on_line_click attribute if element not found.
   function get_line_click_url(bndl, tr)
   {
      var schema = bndl.schema;
      
      var url = null;
      var row_id = tr.getAttribute("data-id");
      var docel = schema.ownerDocument.documentElement;
      
      var node;
      if ((node=get_line_click_element(schema,docel)))
      {
         url = node.getAttribute("url");
         if (row_id)
            url += "=" + row_id;
         
         var pcount = 0;
         var child = node.firstChild;
         while (child)
         {
            if (child.nodeType==1 && child.tagName=="param")
            {
               ++pcount;
               url += get_param_from_element(child, bndl.result, tr);
            }
            child = child.nextSibling;
         }
      }
      else if ((url = get_line_click_attribute(schema,docel)))
      {
         // fall-through if enclosed in <>:
         if (!(url[0]=="<" && url[url.length-1]==">"))
         {
            var id_name = get_line_click_id_name(schema,docel);
            if (id_name)
               url += "&" + id_name;
            
            url += "=" + row_id;
         }
      }

      return url;
   };

   // functions supporting start_dialog closure:
   function OLD_build_dialog(xmldoc, sfw_host, ref)
   {
      var docel = xmldoc.documentElement;
      
      var schema;
      if (!(schema=xmldoc.selectSingleNode("*/schema")))
         if (!(schema=xmldoc.selectSingleNode("*/*[@type='form' | @type='record']/schema")))
            schema = xmldoc.selectSingleNode("*/*/schema");
      
      if (!schema)
      {
         console.error("Can't find schema necessary for building a form.");
         return null;
      }

      var host = addEl("div", sfw_host);

      var result = schema.parentNode;
      var datanode = null;
      if (result==xmldoc.documentElement)
         datanode = result.selectSingleNode("*[@rndx=1]/*[1]");
      else
      {
         var nodename = schema.getAttribute("name");
         datanode = result.selectSingleNode(nodename);
      }

      if (datanode)
      {
         datanode.setAttribute("make_node_dialog", "true");
         xslo.transformFill(host, datanode);
         datanode.removeAttribute("make_node_dialog");
      }
      else
      {
         result.setAttribute("make_dialog", "true");
         xslo.transformFill(host, result);
         result.removeAttribute("make_dialog");
      }
      
      var form = host.getElementsByTagName("form")[0];
      if (form)
      {
         form.style.visible = "hidden";
         
         function finish()
         {
            form.className = "dialog Schema";
            position_dialog(form,ref);
            form.style.visible = "visible";
            
            focus_on_first_field(form);
         }

         if ("custom_form_setup" in window)
            custom_form_setup(form, finish);
         else
            finish();
      }
      else
      {
         console.error("The build_dialog transform failed to make a form.");
         remove_from_parent(host);
      }

      return form;
   }

   function NEW_build_dialog(xmldoc, sfw_host, ref)
   {
      var docel = xmldoc.documentElement;
      var mtype = docel.getAttribute("mode-type");

      var host = null;

      if (mtype=="context")
      {
         // Build a context?
      }
      else
         host = addEl("div", sfw_host);

      if (host)
      {
         xslo.transformFill(host, docel);
         
         var nl, form;
         if ((nl=host.getElementsByTagName("form")).length)
         {
            form = nl[0];
            form.className = "dialog Schema";
            position_dialog(form,ref);
            form.style.visible = "visible";
            focus_on_first_field(form);

            return form;
         }
      }

      return null;
   }

   var build_dialog = NEW_build_dialog;
   
   function get_form_data(form)
   {
      var el, els = form.elements;
      var arr = [];
      var noninputs = 'submit reset button';
      for (var i=0, stop=els.length; i<stop; i++)
      {
         el = els[i];
         if (noninputs.search(el.type)==-1)
         {
            if (el.type=="checkbox")
            {
               if (el.checked)
                  arr.push(el.name + "=1");
            }
            else if ('value' in el && el.value.length>0)
               arr.push(el.name + "=" + encodeURIComponent(el.value));
         }
      }
      return arr;
   }

   function check_close_form(e,t)
   {
      // check for value in t in case t is not a button:
      if (e.type=='click' && ('value' in t && (t.value=='Cancel' || t.value=='Close')))
         return true;
      if (e.type=='keydown')
      {
         var keyp = get_keycode_from_event(e);
         return keyp==27;
      }
      return false;
   }

   function process_expired_login(xhr)
   {
      var newloc = "/";
      var refresh = xhr.getResponseHeader("Refresh");
      if (refresh)
      {
         var match = /url=([^;]+)/.exec(refresh);
         if (match)
            newloc = match[1];
      }

      if (confirm("Authorization has expired.\n\nPress OK to login again.\nPress Cancel to stay here."))
         window.location = newloc;
   }

   function process_xhr_get_error(xhr, url)
   {
      if (xhr.status==403)
         process_expired_login(xhr);
      else
      {
         var msg = "xhr_get failed";
         if (xhr.responseXML==null && xhr.responseText.substr(0,5)=="<?xml")
            msg += " with invalid XML";
         
         if (!url)
            url = xhr.responseURL;

         if (url)
            msg += " at URL=\"" + url + "\"";
         else
            msg += " with no URL(?)";

         console.error(msg);
      }
   }

   function alert_notice_node(node)
   {
      var type = node.getAttribute("type");
      var msg = node.getAttribute("msg");
      var where = node.getAttribute("where");
      var str = type + ":\n" + msg;
      if (where)
         str += "\nwhere: " + where;
      alert(str);
   }

   function has_expected_tagname(schema,new_node,skip_warning)
   {
      var ntag = new_node.tagName;
      var stag = schema.getAttribute("name");
      if (ntag!=stag && !skip_warning)
         console.error("Tag mismatch: added \"" + ntag + "\" element to group of \"" + stag + "\" elements.");
      return ntag==stag;
   }

   // Search for "dlg_bundle" to see how "bundle" is constructed:
   function process_submit_callback(xmldoc, bundle, xslo)
   {
      var docel = xmldoc.documentElement;
      var node;
      if (docel.tagName=="notice")
      {
         alert_notice_node(docel);
         return;
      }
      else
      {
         var n = docel.selectNodes("*");
         if (n && n.length==1 && (n=n[0]))
         {
            if (n.tagName=="message")
            {
               report_message(n);
               return;
            }
            if (n.tagName=="notice")
            {
               console.error("Not expecting to get \"notice\" elements anymore.");
               return;
            }
         }
      }
         
      if ((node=docel.selectSingleNode("*[@type='update' or @type='open']")))
      {
         var type = node.getAttribute("type");
         var new_node = node.selectSingleNode("*[1]");
         
         if (!new_node)
            return;
         
         var result = bundle.result;
         var schema = result ? result.selectSingleNode("schema") : null;

         if (!has_expected_tagname(schema,new_node))
            return;

         // Update xml
         if (bundle.xml_node)
            replace_node(bundle.xml_node, new_node);
         else if (result)
            result.appendChild(new_node);

         // update html
         var host;
         if ((host=bundle.row_host))
         {
            var row = bundle.html_row;
            
            var sort_col = schema ? get_sort_column(schema) : null;
            if (sort_col)
            {
               // Don't add HTML row if we're gonna resort anyway
               result.setAttribute("make_table_body", "true");
               xslo.transformFill(host, result);
               result.removeAttribute("make_table_body");
            }
            else
            {
               // Not sorting, so insert or append HTML row:
               new_node.setAttribute("make_table_line", "true");
               xslo.transformInsert(host, new_node, row);
               new_node.removeAttribute("make_table_line");

               // If row used for insert, remove obsolete row now
               if (row)
                   host.removeChild(row);
            }
            
            _fix_table_heads(bundle.table);
            _set_changed_attribute(bundle.table);

            if (type=="open" && host)
            {
               var rowid = _get_row_id(new_node, schema);
               delay_open(rowid, host, bundle.table);
            }
         }
      }
   }
      
   // Using closures for dialogs for per-dialog data
   function start_dialog(bundle)
   {
      if (xslo==null)
         xslo = xslObj;

      // save and disable event_handler until the dialog is built
      var saved_event_handler = current_event_handler;
      current_event_handler = bogus_event_handler;

      var sfw_host = get_sfw_host(bundle.source);
      var dialog_host = null;

      // Add dialog-specific items to the bundle:
      bundle.form_close = end_closure;

      function end_closure()
      {
         current_event_handler = saved_event_handler;
         if (dialog_host)
         {
            remove_from_parent(dialog_host);
            dialog_host = null;
         }

         // remove closure items from the bundle:
         if ("form_close" in bundle)
            bundle.form_close = null;
         if ("form_submit" in bundle)
            bundle.form_submit = null;
      }

      function callback_form_submit(xmldoc)
      {
         process_submit_callback(xmldoc, bundle, xslo);
         end_closure();
      }

      function _submit_form(form)
      {
         var arr = get_form_data(form);
         var url = form.getAttribute("action");
         if (!url)
            console.error("attempt to submit form without an action.");
         xhr_post(url, arr.join("&"), callback_form_submit, callback_data_error);
      }

      function dialog_click_events(e,t,top)
      {
         switch(t.type)
         {
            case "submit":
               _submit_form(top);
               e.preventDefault();
               return false;
            case "button":
               return process_button_click(e,t,top,bundle);
         }
         return true;
      }

      function dialog_event_handler(e,t,top)
      {
         if (check_close_form(e,t))
         {
            end_closure();
            return false;
         }

         if (e.type=="click" && !dialog_click_events(e,t,top))
            return false;

         if (top && !saved_event_handler(e,t,top))
            return false;
         return true;
      }

      function doc_check_for_message(xmldoc)
      {
         var node = xmldoc.selectSingleNode("*/message");
         if (node)
         {
            var type = node.getAttribute("type");
            var msg = node.getAttribute("message");
            if (type=="error")
            {
               alert(msg);
               return false;
            }
            else
               alert(msg);
         }

         return true;
      }

      function callback_dialog_data(xmldoc)
      {
         if (doc_check_for_message(xmldoc))
         {
            var row = "html_row" in bundle ? bundle.html_row : null;
            var form = build_dialog(xmldoc, sfw_host, row);
            if (form)
            {
               bundle.form_submit = function() { _submit_form(form); };
               
               current_event_handler = dialog_event_handler;
               dialog_host = form.parentNode;

               // Let form access its xml source from a closure:
               form.xmldoc = xmldoc;
            }
            else
               end_closure();
         }
         else
            end_closure();
      }

      function callback_data_error(xhr)
      {
         process_xhr_get_error(xhr,bundle.url);
         end_closure();
      }

      xhr_get(bundle.url, callback_dialog_data, callback_data_error);
   }

   function create_context_host()
   {
      var host = addEl("div", content_host);
      var closer = addEl("div", host);
      addText("X", closer);
      
      host.className = "SFW_Host";
      closer.className = "closer";

      return host;
   }


   function _new_context(url, bundle, callback_open, callback_close)
   {
      var new_host = null;
      var old_host = null;
      var hosted_element = null;
      var saved_old_host_display = null;
      var saved_event_handler = null;
      var saved_custom_handler = null;

      function start_closure()
      {
         new_host = create_context_host();

         if (bundle && "source" in bundle)
            old_host = get_sfw_host(bundle.source);
         
         if (old_host)
         {
            saved_old_host_display = old_host.style.display;
            old_host.style.display = "none";
         }
         
         if ("custom_handler" in window)
            saved_custom_handler = window.custom_handler;
      }

      function end_closure()
      {
         window.custom_handler = saved_custom_handler;

         if (new_host)
         {
            remove_from_parent(new_host);
            new_host = null;
         }
         
         if (old_host)
            old_host.style.display = saved_old_host_display;

         if (hosted_element)
         {
            if ("xmldoc" in hosted_element)
               hosted_element.xmldoc = null;
            if (callback_close)
               callback_close(hosted_element);
         }
         
         if (hosted_element && "xmldoc" in hosted_element)
            hosted_element.xmldoc = null;

         if (bundle && "update" in bundle)
            bundle.update();

         if (saved_event_handler!=null)
            current_event_handler = saved_event_handler;
      }

      function thandler(e,t)
      {
         if ((e.type=='keydown' && get_keycode_from_event(e)==27)
            || (class_includes(t,"closer")))
         {
            end_closure();
            return false;
         }
         return true;
      }

      function xhr_callback(doc)
      {
         var nl;
         
         if (!alert_notice(doc))
         {
            start_closure();

            // ensure SFW_Content extent backs all contents:
            resize_sfw_content();

            var docel = doc.documentElement;
            var subd = addEl("div", new_host);
//            var subd = new_host;
            if (!docel.hasAttribute("form-type"))
            {
               docel.setAttribute("form-type","form");
               xslObj.transformFill(subd, docel);
               docel.removeAttribute("form-type");
            }
            else
               xslObj.transformFill(subd, docel);

            if ((nl=subd.getElementsByTagName("table")).length)
            {
               hosted_element = nl[0];
               _fix_table_heads(hosted_element);
               saved_event_handler = current_event_handler;
               current_event_handler = table_event_handler;
            }
            else if ((nl=subd.getElementsByTagName("form")).length)
               hosted_element = nl[0];

            if (hosted_element)
               hosted_element.xmldoc = doc;
 
            // Create closure and change event handling only if
            // opening a filled window
            window.custom_handler = thandler;

            // Make host large enough to cover all previous content:
            resize_to_fill_sfw_content(subd);

            if (callback_open)
               callback_open(subd);
         }
      }

      function error_callback(xhr)
      {
         process_xhr_get_error(xhr,url);
      }

      xhr_get(url, xhr_callback, error_callback);
   }
}

 
