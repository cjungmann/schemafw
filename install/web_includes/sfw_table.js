
// sfw_table.js

(function _init()
{
   if ((!("SFW" in window) && setTimeout(_init,100))
       || SFW.delay_init("sfw_table",_init,"tbase"))
      return;

   if (!SFW.derive(_table, "table", "tbase") ||
       !SFW.derive(_table, "import-review", "tbase"))
      return;
   
   function _table(actors)
   {
      SFW.base.call(this, actors);
   }

   // Adding useful local functions to global object
   SFW.fix_table_heads = _fix_table_heads;


   _table.prototype.get_sort_field = function()
   {
      function f(n) { return n.nodeType==1
                      && n.tagName=="field"
                      && n.getAttribute("sorting"); }
      return SFW.find_child_matches(this.schema(),f,true);
   };

   _table.prototype.get_named_field = function(name)
   {
      function f(n) { return n.nodeType==1
                      && n.tagName=="field"
                      && n.getAttribute("name")==name; }
      return SFW.find_child_matches(this.schema(), f, true);
   };

   var sort_tr_arr = ["sorted_up", "sorted_down"];
   function _remove_sort_classes(n)
   {
      for (var i=0; i<2; ++i)
      {
         var c = sort_tr_arr[i];
         if (class_includes(n,c))
            class_remove(n,c);
      }
   }

   function _style_sorted_column_head(tr, descending)
   {
      var thead = tr.parentNode;
      var n = thead.firstChild;
      var sclass = "sorted_" + (descending?"down":"up");
      while (n)
      {
         if (n.nodeType==1 && n.tagName.toLowerCase()=="th")
         {
            _remove_sort_classes(n);
            if (n==tr)
               class_add(n,sclass);
         }
         n = n.nextSibling;
      }
   }

   _table.prototype.set_sort_column = function(th)
   {
      var field = this.get_named_field(th.getAttribute("data-name"));
      if (field)
      {
         var descending = false;
         var sorted = this.get_sort_field();
         if (sorted==field)
         {
            if (field.getAttribute("descending"))
               field.removeAttribute("descending");
            else
            {
               field.setAttribute("descending", "1");
               descending = true;
            }
         }
         else
         {
            field.setAttribute("sorting", "true");
            if (sorted)
            {
               sorted.removeAttribute("sorting");
               sorted.removeAttribute("descending");
            }
         }

         _style_sorted_column_head(th, descending);
         return true;
      }
      else
         return false;
   };

   _table.prototype.get_table_lines_node = function()
   {
      var xpath = "/xsl:stylesheet/xsl:variable[@name='table_lines']";
      return SFW.xslobj.find_node(xpath);
   };

   _table.prototype.set_table_lines = function(xpath)
   {
      var n;
      if ((n=this.get_table_lines_node()))
         n.setAttribute("select", xpath);
   };

   _table.prototype.clear_table_lines = function()
   {
      var n;
      if ((n=this.get_table_lines_node()))
         n.removeAttribute("select");
   };

   _table.prototype.replot = function(result)
   {
      this.pre_transform();
      
      SFW.xslobj.transformFill(this.host(), SFW.xmldoc.documentElement);
      
      var top = this.top();
      if (top)
         _fix_table_heads(top);

      this.post_transform();
   };

   _table.prototype.diagnose_missing_data_id = function()
   {
      var schema = this.schema();
      if (schema)
      {
         var nl = schema.selectNodes("field[@group]");
         if (nl && nl.length)
         {
            debugger;
            SFW.alert("Table with GROUP BY field is missing xrow_id.");
         }
      }
   };

   _table.prototype.initialize = function()
   {
      var anchor = SFW.seek_child_anchor(this.host());
      if (SFW.confirm_not_null(anchor,"Unable to get child anchor"))
         SFW.fix_table_heads(anchor);
   };

   _table.prototype.process_button_add = function(button)
   {
      var os = SFW.get_page_offset();  // Get offset before discarding contents
      var host = this.host();
      var url = button.getAttribute("data-task") || button.getAttribute("data-url");

      if (url)
      {
         // this.empty_node(host);

         SFW.open_interaction(SFW.stage,
                              url,
                              this,
                              { os:os, host:host, button:button }
                             );
         return false;
      }
      
      return true;
   };

   _table.prototype._old_process_line_click = function(tr)
   {
      if (!(url=this.get_data_value("on_line_click")))
         return true;
      
      // By default, process the line if and only if it's under a tbody.
      // That is, skip rows in thead and tfoot elements.
      if (tr.parentNode.tagName.toLowerCase()!="tbody")
         return null;

      var id, url;
      
      if (!(id=tr.getAttribute("data-id")))
      {
         this.diagnose_missing_data_id(tr);
         return true;
      }
      
      var idname = this.get_line_click_id_name();
      function f(n) { return n.nodeType==1 && n.getAttribute(idname)==id; }

      // Somehow get row-name, perhaps saved in the attributes of the table,
      // and pass it as an argument to result():
      var xrow = SFW.find_child_matches(this.result(), f, true);

      if (xrow)
      {
         if (url.indexOf('&')==-1)
            url += "=" + id;
         else
            url = SFW.apply_row_context(url, xrow);

         var os = SFW.get_page_offset();  // Get offset before discarding contents
         var host = this.host();

         // this.empty_node(host);

         SFW.open_interaction(SFW.stage,
                              url,
                              this,
                              { os:os, host:host, xrow:xrow }
                             );

         return false;
      }

      return true;
   };

   _table.prototype.process = function _table_process_message(e,t)
   {
      var table_el = this.top();

      // For now, only handle clicks (keyboard handled earlier):
      if (e.type!="click")
         return true;

      // Allow base class to process generic buttons
      if (!SFW.base.prototype.process.call(this,e,t))
         return false;

      var click_info;
      while (t && t!=table_el)
      {
         if (t.getAttribute("data-id"))
         {
            if ((click_info=this.get_el_click_info(t)))
               return this.process_click_info(click_info);
         }
         else if (t.tagName.toLowerCase()=="th")
         {
            if (class_includes(t,'sortable'))
            {
               this.set_sort_column(t);
               this.replot();
               return false;
            }
         }
         t = t.parentNode;
      }

      return true;
   };

   function is_tr_el(node)
   {
      return node.nodeType==1 && node.tagName.toLowerCase()=="tr";
   }

   function is_th_el(node)
   {
      return node.nodeType==1 && node.tagName.toLowerCase()=="th";
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

      function reposition_element(ref, flo, sizer)
      {
         var ref_par = ref.parentNode;
         var os = SFW.get_relative_offset(ref, ref_par);
         flo.style.left = SFW.px(os.left + sizer.left);
         flo.style.top = SFW.px(os.top + sizer.top);
      }

      function resize_element(ref, flo, sizer)
      {
         flo.style.height = SFW.px(ref.offsetHeight + sizer.high);
         flo.style.width = SFW.px(ref.offsetWidth + sizer.wide);
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
         var r,f,i;

         r = SFW.first_child_element(ref);
         f = SFW.first_child_element(flo);

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

            r = SFW.next_sibling_element(r);
            f = SFW.next_sibling_element(f);
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
         
         var osref = SFW.get_doc_offset(ref);
         flo.style.left = SFW.px(osref.left);
         flo.style.top = SFW.px(osref.top);
         resize_element(ref,flo,sizer);

         if (sizer.high===null)
         {
            calculate_sizer(ref,flo,sizer);

            if (sizer.left || sizer.top)
               reposition_element(ref,flo,sizer);
            if (sizer.wide || sizer.high)
               resize_element(ref,flo,sizer);
         }

         var first = SFW.first_child_element(ref);
         if (first)
         {
            var ftag = first.tagName.toLowerCase();
            if (ftag=="th" || ftag=="td")
               align_cells(ref, flo);
         }

         flo.style.visibility = "visible";
      }

      var re_headfix = /headfix_\S+/;
      function align_paired_rows(thead)
      {
         var match, classname, trmatch;
         var currow = SFW.first_child_element(thead);
         while (currow)
         {
            if (is_tr_el(currow) && class_includes(currow,"floater"))
               if ((match=currow.className.match(re_headfix)) && (classname=match[0]))
                  if ((trmatch=find_matched_headfix_row(currow, classname)))
                     align_pair(trmatch,currow,sizer_tr);
            currow = SFW.next_sibling_element(currow);
         }
      }

      // search nextSiblings for match
      function find_matched_headfix_row(tr, hf_name)
      {
         var cname = tr.className;
         var fpos = cname.lastIndexOf(" floater");
         cname = cname.substring(0,fpos);
         
         var n = tr.parentNode.firstChild;
         while (n)
         {
            if (is_tr_el(n))
               if (n.className == cname)
                  return n;
            n = n.nextSibling;
         }

         return null;
      }

      // replace function with function in closure:
      _fix_table_heads = function(tab)
      {
         var thead = tab.getElementsByTagName("thead");
         if (thead && thead.length)
            align_paired_rows(thead[0]);
      };

      // must explicitely call the function when setting up the closure:
      SFW.fix_table_heads = _fix_table_heads;
      _fix_table_heads(table);
      
   }  // end of _fix_table_heads
})();
