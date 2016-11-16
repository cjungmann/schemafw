function init_SFW_Tables()
{
   SFW.default_table = _table;
   SFW.fix_table_heads = _fix_table_heads;

   function _init_table(table, doc)
   {
      _fix_table_heads(table);

      SFW.process_message = function(e,t)
      {
         return _table_process_message(e,t);
      };
   }

   function _table(base, doc)
   {
      SFW.base.call(this,base,doc);
      SFW.fix_table_heads(base);
   }

   SFW.derive(_table, SFW.base);

   _table.prototype.open_dialog = function _open_dialog(button, row)
   {
      var stage = this.top().parentNode;
      
   };

   _table.prototype.get_line_id = function _get_line_id(row)
   {
      var f, schema = this.schema();
      if ((doc=this.doc()) && (schema=doc.selectSingleNode("*/*[@rndx=1]/schema")))
      {
         if (!(f=schema.selectSingleNode("field[@line_id]")))
            f = schema.selectSingleNode("field[@primary-key]");
         if (f)
            return row.getAttribute(f.getAttribute("name"));
         else // return value of first attribute child
         {
            var a = row.firstChild;
            while (a)
            {
               if (a.nodeType==2)
                  return a.value;
               else
                  a = a.nextSibling;
            }
         }
      }
      return null;
   };

   _table.prototype.update_row = function _update_row(row, doc)
   {
      var xpath = "*[@type='update' or @type='open']";
      var node, newnode;
      if ((node=doc.documentElement.selectSingleNode(xpath))
          && (newnode=node.selectSingleNode("*[1]")))
      {
         var parent = row.parentNode;
         SFW.xslobj.transformInsert(parent, doc, row);
         parent.removeChild(row);
      }
   };

   _table.prototype.process_line_click = function _process_line_click(row)
   {
      function finish(doc)
      {
         this.update_row(row,doc);
      }
      var doc, on_line_click;
      if ((doc=this.schema())
          && (on_line_click=doc.documentElement.getAttribute("on_line_click")))
      {
         var id = this.get_line_id(row);
         if (id)
         {
            var url = on_line_click + "=" + id;
            
         }
      }
   };
   
   _table.prototype.process = function _table_process_message(e,t)
   {
      // For now, only handle clicks (keyboard handled earlier):
      if (e.type!="click")
         return true;

      var table_el = this.top();

      // Filter non-owned elements:
      if (table_el != SFW.get_ancestor_anchor(t))
         return true;

      while (t && t!=table_el)
      {
         var tag = t.tagName.toLowerCase();
         switch(tag)
         {
            case "button":
            {
               var btype = t.getAttribute("data-type");
               switch(btype)
               {
                  case "add":
                     var url = SFW.translate_url(t.getAttribute("data-task"),doc);
                     var rowhost = table_el.getElementsByTagName("tbody")[0];
                     var contenthost = this.top().parentNode;
                  
                     // bndl.url = translate_url(t.getAttribute("data-task"), doc);
                     // bndl.row_host = top.getElementsByTagName("tbody")[0];
                     // start_dialog(bndl);
                     return false;
                  case "jump":
                     return process_button_jump(t);
                  case "open":
                     return process_button_open(t);
                  default:
                     alert("Unknown button type \"" + btype + "\"");
                     break;
               }
               break;
            }
            case "tr":
               return process_tr_click(t);

            case "th":
            {
               if ((class_includes(t,'sortable')))
               {
                  if (result_is_free_to_replot(bndl.result))
                  {
                     set_sort_column(bndl.schema, t);
                     replot_table_contents(table_el, bndl.result);
                     return false;
                  }
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
      }

      var re_headfix = /headfix_\S+/;
      function align_paired_rows(thead)
      {
         var rematch, trmatch;
         var currow = SFW.first_child_element(thead);
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
            currow = SFW.next_sibling_element(currow);
         }
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

}
