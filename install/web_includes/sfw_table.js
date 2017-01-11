
// sfw_table.js

(function _init()
{
   if (SFW.delay_init("sfw_table",_init,"iclass"))
      return;

   if (!SFW.derive(_table, "table", "iclass") ||
       !SFW.derive(_table, "import-review", "iclass"))
      return;
   
   function _table(base, doc, caller, data)
   {
      SFW.base.call(this,base,doc,caller,data);
      SFW.fix_table_heads(base);
   }

   // Adding useful local functions to global object
   SFW.fix_table_heads = _fix_table_heads;

   function _match_rndx(n) { return n.nodeType==1 && n.getAttribute("rndx"); }

   _table.prototype.result = function(match)
   {
      var mfunc = _match_rndx;
      if (match)
      {
         var str = ("tagName" in match)?match.tagName:match;
         mfunc = function(n) {
            return n.nodeType==1
               && n.getAttribute("rndx")
               && n.getAttribute("row-name")==str;
         };
      }
      
      return SFW.find_child_matches(this._xmldoc, mfunc, true, true);
   };
   
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

   _table.prototype.replot = function(match)
   {
      var tbody = SFW.find_child_matches(this.top(), "tbody", true);
      if (tbody)
      {
         this.result(match).setAttribute("make_table_body", "true");
         SFW.xslobj.transformFill(tbody, this.result(match));
         this.result(match).removeAttribute("make_table_body");
         _fix_table_heads(this.top());
      }
      else
      {
         SFW.xslobj.transformFill(this._host, this.result(match));
         _fix_table_heads(this.top());
      }
   };

   _table.prototype.get_line_id = function _get_line_id(row)
   {
      var line_id = null;
      if ((line_id=row.getAttribute("data-id")))
         return line_id;
      
      var doc, f;
      if ((doc=this.doc()) && this.schema())
      {
         if (!(f=this.schema().selectSingleNode("field[@line_id]")))
            f = this.schema().selectSingleNode("field[@primary-key]");
         if (f)
            return row.getAttribute(f.getAttribute("name"));
         else // return value of first attribute child
         {
            var a = row.firstChild;
            while (a)
            {
               if (a.nodeType==2)
               {
                  line_id = a.value;
                  break;
               }
               else
                  a = a.nextSibling;
            }
         }
      }
      return line_id;
   };

   _table.prototype.update_row = function _update_row(row, doc)
   {
      var xpath = "*[@type='update' or @type='open']";
      var node, newnode;
      if ((node=doc.documentElement.selectSingleNode(xpath))
          && (newnode=node.selectSingleNode("*[1]")))
      {
         var parent = row.parentNode;
         SFW.xslobj.transformfgInsert(parent, doc, row);
         parent.removeChild(row);
      }
   };

   _table.prototype.get_sfw_attribute = function(aname)
   {
      var name = null;
      if (this.schema() && !(name=this.schema().getAttribute(aname)))
         name = this._xmldoc.documentElement.getAttribute(aname);
      return name || "id";
   };

   _table.prototype.get_line_click_id_name = function()
   {
      return this.get_sfw_attribute("line_click_id");
   };

   function _empty_node(node)
   {
      // Remove attribtes separately, since they are not included in the child list:
      // Iterate in reverse or removing each attribute will change the position of
      // the remainder of the attributes.
      for (var s=node.attributes,i=s.length-1; i>=0; --i)
         node.removeAttribute(s[i].nodeName);
      
      var n, a = node.firstChild;
      while ((n=a))
      {
         a = a.nextSibling;
         node.removeChild(n);
      }
   }

   function _copy_node(target, source)
   {
      // Copy attribtes separately, since they are not included in the child list:
      for (var s,i=0,sa=source.attributes,stop=sa.length; i<stop && (s=sa[i]); ++i)
         target.setAttribute(s.nodeName,s.nodeValue);

      var n, a = source.firstChild;
      var doc = "documentElement" in target ? target : target.documentObject;
      while((n=a))
      {
         a = a.nextSibling;
         switch(n.nodeType)
         {
            case 1: // element
               var newel = SFW.addXMLEl(n.tagName,target);
               _copy_node(newel, n);
               break;

            // Handled in for-loop at top of function
            // case 2: // attribute
            //    target.setAttribute(n.nodeName,n.nodeValue);
            //    break;

            case 3: // text
               addText(n.nodeValue, target);
               break;

            // case 4: // CDataSection
            //    break;

            case 5: // EntityReference
               target.appendChild(doc.createEntityReference(n.nodeName));
               break;

            default:
               break;
         }
      }
   }

   _table.prototype.add_result_node = function(result,xrow)
   {
      var n, p, res;
      if ((n=SFW.first_child_element(result)))
      {
         if (xrow)
         {
            if ((p=xrow.parentNode))
            {
               p.insertBefore(n,xrow);
               p.removeChild(xrow);
            }
         }
         else if ((res=this.result(n)))
            res.appendChild(n);
      }
   };

   _table.prototype.copy_result_node = function(result,row_to_replace)
   {
      var res, n;
      if ((n=SFW.first_child_element(result)))
      {
         if ((res=this.result(n)))
         {
            if (row_to_replace)
               _empty_node(row_to_replace);
            else
               row_to_replace = SFW.addXMLEl(n.tagName,res);

            _copy_node(row_to_replace, n);
         }
      }
   };

   _table.prototype.delete_row = function(xrow)
   {
      if (this.result(xrow) && xrow)
         xrow.parentNode.removeChild(xrow);
   };

   _table.prototype.find_matching_data_row = function(cfobj)
   {
      var id, res, el = ("rowone" in cfobj) ? cfobj.rowone : null;
      if (el)
      {
         var xpath = "*/*[@rndx and @row-name='" + el.tagName + "']";
         res = this._xmldoc.selectSingleNode(xpath);
         id = el.getAttribute("id");
         if ((res=this.result(el)) && (id=el.getAttribute("id")))
         {
            function f(n) { return n.nodeType==1 && n.getAttribute("id")==id; }
            return SFW.find_child_matches(res, f, true, false);
         }
      }

      return null;
   };

   _table.prototype.update_row = function(cfobj, preserve_result)
   {
      var xrow = this.find_matching_data_row(cfobj);
      if ("docel" in cfobj)
      {
         if (cfobj.mtype=="delete" && cfobj.confirm_delete() && xrow)
         {
            debugger;
            // find the selected row:
            this.delete_row(xrow);
         }
         else if (cfobj.rtype=="update")
         {
            // If xrow is NULL, the new result will be appended:
            if (preserve_result)
               this.copy_result_node(cfobj.result, xrow);
            else
               this.add_result_node(cfobj.result, xrow);
         }
      }
      else if ("cmd" in cfobj)
      {
         switch(cfobj.cmd)
         {
            case "delete": this.delete_row(cfobj.cdata.xrow); break;
            case "fail":
               SFW.alert(cfobj.rtype + " operation failed.");
               return;   // skip replot() at end of function
         }
      }
   };

   _table.prototype.child_finished = function(cfobj)
   {
      this.update_row(cfobj);
      this.replot();

      var dobj = cfobj.cdata;
      if (dobj && "os" in dobj)
         SFW.set_page_offset(dobj.os);
      SFW.base.prototype.child_finished.call(this,cfobj);
   };
   
   _table.prototype.process_button_add = function(button)
   {
      var os = SFW.get_page_offset();  // Get offset before discarding contents
      var host = this._host;
      var url = button.getAttribute("data-task") || button.getAttribute("data-url");

      if (url)
      {
         empty_el(host);

         SFW.open_interaction(SFW.stage,
                              url,
                              this,
                              { os:os, host:host }
                             );
         return false;
      }
      
      return true;
   };

   _table.prototype.process_line_click = function(tr)
   {
      var id, url;
      if ((id=tr.getAttribute("data-id"))
          && (url=this.top().getAttribute("data-on_line_click")))
      {
         url += "=" + id;

         var idname = this.get_line_click_id_name();
         function f(n) { return n.nodeType==1 && n.getAttribute(idname)==id; }


         // Somehow get row-name, perhaps saved in the attributes of the table,
         // and pass it as an argument to result():
         var xrow = SFW.find_child_matches(this.result(), f, true);

         var os = SFW.get_page_offset();  // Get offset before discarding contents
         var host = this._host;

         empty_el(host);

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

      while (t && t!=table_el)
      {
         var tag = t.tagName.toLowerCase();
         switch(tag)
         {
            case "button":
               return this.process_clicked_button(t);
            case "tr":
               return this.process_line_click(t);

            case "th":
            {
               if ((class_includes(t,'sortable')))
               {
                  // if (result_is_free_to_replot(bndl.result))
                  // {
                     this.set_sort_column(t);
                     this.replot();
                     return false;
                  // }
               }
               break;
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
