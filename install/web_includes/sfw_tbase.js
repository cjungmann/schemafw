
// sfw_tbase.js

(function _init()
{
   if ((!("SFW" in window) && setTimeout(_init,100))
       || SFW.delay_init("sfw_tbase",_init,"iclass"))
      return;

   if (!SFW.derive(_tbase, "tbase", "iclass"))
      return;

   function _tbase(actors)
   {
      SFW.base.call(this,actors);
   }

   // Can be overridden in derived classes:
   _tbase.prototype.get_result_name = function() { return null; };

   _tbase.prototype.result = function()
   {
      var xpath = this.get_result_name() || this.get_result_name_from_top();
      if (!xpath)
         xpath = "*[@rndx=1][not(@merged)]";
         
      return this.xmldocel().selectSingleNode(xpath);
   };

   _tbase.prototype.find_matching_data_row = function(cfobj)
   {
      var el =
         ("cdata" in cfobj && "xrow" in cfobj.cdata)
         ? cfobj.cdata.xrow
         : cfobj.update_row;

      if (el)
      {
         var id, res, xpath = "*[@rndx][@row-name='" + el.tagName + "']";
         res = this.xmldocel().selectSingleNode(xpath);
         id = el.getAttribute("id");
         if ((res=this.result(el)) && (id=el.getAttribute("id")))
         {
            function f(n) { return n.nodeType==1 && n.getAttribute("id")==id; }
            return SFW.find_child_matches(res, f, true, false);
         }
      }

      return null;
   };

   _tbase.prototype.replot = function(result)
   {
      console.error("This function should be overridden!");
   };

   _tbase.prototype.empty_node = function(node)
   {
      // Remove attributes separately, since they are not included in the child list:
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
   };

   function _get_copied_node(target, source)
   {
      var doc = target.ownerDocument;
      function ca(target,source)
      {
         // Copy attribtes separately, since they are not included in the child list:
         for (var s,i=0,sa=source.attributes,stop=sa.length; i<stop && (s=sa[i]); ++i)
            target.setAttribute(s.nodeName,s.nodeValue); 
      }

      function cp(target,source)
      {
         ca(target,source);

         var node, next = source.firstChild;
         while((node=next))
         {
            next = node.nextSibling;
            switch(node.nodeType)
            {
               case 1:  // element
                  cp(doc.createElement(node.tagName),node);
                  break;
               // no case 2: ca() copies attributes
               case 3:  // text
                  target.appendChild(doc.createTextNode(node.nodeValue));
                  break;
               case 5: // EntityReference
                  target.appendChild(doc.createEntityReference(n.nodeName));
                  break;
               default: break;
            }
         }
      }

      var newnode = doc.createElement(source.tagName);

      cp(newnode, source);
      return newnode;
   }
   
   _tbase.prototype.delete_row = function(xrow)
   {
      if (this.result(xrow) && xrow)
         xrow.parentNode.removeChild(xrow);
   };

   _tbase.prototype.get_result_to_update = function(cfobj)
   {
      var result = null;
      var tname = cfobj.target_name;
      if (tname)
         result = this.xmldocel().selectSingleNode(tname+"[@rndx]");
      return result;
   };

   _tbase.prototype.update_row = function(cfobj, preserve_result)
   {
      var cdata = "cdata" in cfobj ? cfobj.cdata : null;
      var xrow = (cdata && "xrow" in cdata) ? cdata.xrow : null;
      var urow = ("update_row" in cfobj) ? cfobj.update_row : null;

      if ("docel" in cfobj)
      {
         if (cfobj.mtype=="delete")
         {
            if (cfobj.confirm_delete() && xrow)
               this.delete_row(xrow);
         }
         else if (cfobj.rtype=="update" && urow)
         {
            var target = this.get_result_to_update(cfobj);
            if (!target)
            {
               if (xrow)
                  target = xrow.parentNode;
               else
                  target = this.xmldocel().selectSingleNode("*[@rndx=1][1]");
            }

            if (target)
            {
               if (preserve_result || target.ownerDocument!=urow.ownerDocument)
                  urow = _get_copied_node(target,urow);

               target.insertBefore(urow, xrow);

               if (xrow)
                  target.removeChild(xrow);
            }
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


   _tbase.prototype.get_cell_click_id_name = function()
   {
      return this.get_sfw_attribute("cell_click_id") || "id";
   };

   _tbase.prototype.get_line_click_id_name = function()
   {
      return this.get_sfw_attribute("line_click_id") || "id";
   };

   /**
    * The assumption is that an application that allows table cell-based
    * editing must allow for empty cells.  Thus the standard implementation
    * of get_cell_click_info sets the click_info object property data_id=0
    * if the cell is empty.
    *
    * This differs from get_line_click_info, where it is assumed that any
    * line that is displayed actually exists in the data.
    *
    * Variations from these assumptions should be handled with custom
    * implementations or get_cell_click_info, get_line_click_info, or
    * a custom object or window method that can be called with the
    * click_info object.
    */
   _tbase.prototype.get_cell_click_info = function(td)
   {
      var task, did, dname;
      if ((task=this.get_data_value("on_cell_click")))
      {
         rval = { target:td, task:task, id_name:this.get_cell_click_id_name() };

         // As a cell-click procedure, set data_id=0 if there is no
         // data-id attribute.  That indicates an empty record.  I assume
         // (and may be wrong about this) that any cell-editing table
         // will likely be potentially sparsely-populated. That is, there
         // may be some empty cells without it being an error.
         if ((did=td.getAttribute("data-id")))
            rval.data_id = did;
         else
            rval.data_id = 0;

         // There should be a data-name attribute that allows a sparse
         // table to know what kind of object should be created.  An
         // example is a calendar date with no contents, and the data-name
         // would be the date.
         if ((dname=td.getAttribute("data-name")))
            rval.data_name = dname;

         return rval;
      }

      return null;
   };

   _tbase.prototype.get_line_click_info = function(tr)
   {
      var task, did;
      if ((task=this.get_data_value("on_line_click"))
          && tr.parentNode.tagName.toLowerCase()=="tbody")
      {
         rval = { target:tr, task:task, id_name:this.get_line_click_id_name() };

         if ((did=tr.getAttribute("data-id")))
            rval.data_id = did;

         return rval;
      }

      return null;
   };

   _tbase.prototype.process_click_info = function(info)
   {
      var id, xrow = null;

      if ("target" in info)
      {
         // This sequence makes a few assumptions about the data:
         // 1. A missing data_id is a failure and should be diagnosed
         // 2. Table cells may be sparsely populated, ie, some cells may
         //    be empty.  In that case, data_id will be set to 0.
         //    This is something to keep in mind for custom implementations
         //    of the get_cell_click_info().
         // 3. Another situation that might have a 0 data_id is if the
         //    intent is to create a new record.
         if (!("data_id" in info))
         {
            this.diagnose_missing_data_id(info.target);
            return true;
         }
         else if ((id=info.data_id))
         {
            function f(n) { return n.nodeType==1 && n.getAttribute(info.id_name)==id; }
            xrow = SFW.find_child_matches(this.result(), f, true);
         }
      }

      // Look for custom handlers, first at the object level as a object procedure,
      // then at the global level, as window[(function_name=info.task)]().
      if (info.task in this)
         return this[info.task](info.target, xrow, info);
      if (info.task in window)
         return window[info.task](info.target, xrow, info);

      // The default behavior is that an item click, either line or cell,
      // should open an edit dialog.  A sparse table should create a custom
      // handler to respond to *new* or *edit* requests.
      // Because of the preceding, this function only continues if an xrow is
      // found.
      if (xrow)
      {
         var url = info.task;

         if (url.indexOf('&')==-1)
            url += "=" + id;
         else
            url = SFW.apply_row_context(url, xrow);

         var os = SFW.get_page_offset();  // Get offset before discarding contents
         var host = this.host();

         SFW.open_interaction(SFW.stage,
                              url,
                              this,
                              { os:os, host:host, xrow:xrow }
                             );

         return false;
      }

      return true;
   };

   
})();
