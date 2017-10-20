
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
      var el = SFW.has_value(cfobj,"cdata","xrow") ? cfobj.cdata.xrow : cfobj.update_row;

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

   _tbase.prototype.delete_row = function(xrow)
   {
      if (this.result(xrow) && xrow)
         xrow.parentNode.removeChild(xrow);
   };

   _tbase.prototype.get_result_to_update = function(cfobj, urow, xrow)
   {
      if (arguments.length<3)
      {
         urow = SFW.get_urow_from_cfobj(cfobj);
         xrow = SFW.get_xrow_from_cfobj(cfobj);
      }
      
      var result = null;
      if (SFW.has_value(cfobj,"target_name"))
         result = this.xmldocel().selectSingleNode(cfobj.target_name+"[@rndx]");
      else if (xrow)
         result = xrow.parentNode;
      else
         result = this.xmldocel().selectSingleNode("*[@rndx=1][not(@merged)]");

      if (result)
      {
         var rname = result.getAttribute("row-name");
         if (!rname)
         {
            var attr = result.selectSingleNode("schema/@name")
                || result.selectSingleNode("schema/@row-name");
            if (attr)
               rname = attr.value;
         }
         var uname = urow ? urow.tagName : null;

         if (rname==uname)
            return result;
         else
            SFW.alert("The update row name (" + (uname||"NULL") +
                      ") does not match the result's row-name (" + rname +").");
      }
      else
         SFW.alert("Unable to find a result element.");

      return null;
   };

   _tbase.prototype.update_row = function(cfobj, preserve_result)
   {
      if (SFW.has_value(cfobj,"cmd"))
      {
         switch(cfobj.cmd)
         {
            case "delete": this.delete_row(cfobj.cdata.xrow); break;
            case "fail": SFW.alert(cfobj.rtype + " operation failed."); break;
         }
         return;
      }

      var urow = SFW.get_urow_from_cfobj(cfobj);
      var xrow = SFW.get_xrow_from_cfobj(cfobj);

      var target = this.get_result_to_update(cfobj, urow, xrow);
      if (target)
      {
         if (cfobj.mtype=="delete")
         {
            if (cfobj.confirm_delete() && xrow)
               this.delete_row(xrow);
         }
         else if (cfobj.rtype=="update")
         {
            urow = preserve_result ? SFW.get_copied_node(target,urow) : urow;
            if (xrow)
            {
               var before = SFW.next_sibling_element(xrow);
               target.removeChild(xrow);
               xrow = before;
            }
            target.insertBefore(urow, xrow);
         }
         else
            SFW.alert("Don't know what to do with the update row.");
      }
   };

   _tbase.prototype.preview_result = function(returned_doc, child)
   {
      var docel = returned_doc.documentElement;
      var mtype = docel.getAttribute("mode-type");
      var result, xrow = SFW.get_property(child,"host","data","xrow");
      if (mtype=="delete" && xrow)
      {
         if ((result=docel.selectSingleNode("*[@rndx][@type='delete']"))
             && result.getAttribute("deleted")!=0)
         {
            console.log("We would be deleting a row here.");
            // xrow.parentNode.removeChild(xrow);
         }
      }
      else
      {
         console.log("Mtype is " + mtype);
      }
   };

   _tbase.prototype.child_finished = function(child, cancelled)
   {
      // Must call base::child_finished() to clean out
      // any merged elements before calling replot().
      SFW.base.prototype.child_finished.call(this, child, cancelled);

      if (!cancelled)
      {
         // this.update_row(cfobj);
         this.replot();
      }

      var os = SFW.get_property(child,"host","data","os");
      if (os)
         SFW.set_page_offset(os);
      // if (SFW.has_value(cfobj,"cdata","os"))
      //    SFW.set_page_offset(cfobj.cdata.os);
   };


   _tbase.prototype.get_sfw_attribute = function(aname)
   {
      var schema, name = null;
      if ((schema=this.schema()) && !(name=schema.getAttribute(aname)))
         name = this.xmldocel().getAttribute(aname);
      return name;
   };

   _tbase.prototype.get_cell_click_id_name = function()
   {
      return this.get_sfw_attribute("cell_click_id") || "id";
   };

   _tbase.prototype.get_line_click_id_name = function()
   {
      return this.get_sfw_attribute("line_click_id") || "id";
   };

   _tbase.prototype.get_row_id = 
      _tbase.prototype.get_cell_id =
      function(el) { return el.getAttribute("data-id");};

   _tbase.prototype.get_row_name = 
      _tbase.prototype.get_cell_name =
      function(el) { return el.getAttribute("data-name");};

   _tbase.prototype.get_cell_click_action = function() { return this.get_data_value("on_cell_click"); };
   _tbase.prototype.get_line_click_action = function() { return this.get_data_value("on_line_click"); };


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
      var field, task, did, dname, rval=null;
      if ((task=this.get_cell_click_action()))
      {
         rval = { target:td, task:task, id_name:this.get_cell_click_id_name() };

         // As a cell-click procedure, set data_id=0 if there is no
         // data-id attribute.  That indicates an empty record.  I assume
         // (and may be wrong about this) that any cell-editing table
         // will likely be potentially sparsely-populated. That is, there
         // may be some empty cells without it being an error.
         if ((did=this.get_cell_id(td)))
            rval.data_id = did;
         else
            rval.data_id = 0;

         // There should be a data-name attribute that allows a sparse
         // table to know what kind of object should be created.  An
         // example is a calendar date with no contents, and the data-name
         // would be the date.
         if ((dname=this.get_cell_name(td)))
            rval.data_name = dname;

         if((result=this.get_ref_result()))
            rval.result = result;
      }

      return rval;
   };

   _tbase.prototype.get_line_click_info = function(tr)
   {
      var result, task, did, rval=null;
      if ((task=this.get_line_click_action())
          && tr.parentNode.tagName.toLowerCase()=="tbody")
      {
         rval = { target:tr, task:task, id_name:this.get_line_click_id_name() };

         if ((did=this.get_row_id(tr)))
            rval.data_id = did;

         if((result=this.get_ref_result()))
            rval.result = result;
      }

      return rval;
   };

   _tbase.prototype.process_click_info = function(info)
   {
      var id, xrow = null;

      if (SFW.has_value(info,"target"))
      {
         // This sequence makes a few assumptions about the data:
         // 1. A missing data_id is a failure and should be diagnosed
         // 2. Table cells may be sparsely populated, ie, some cells may
         //    be empty.  In that case, data_id will be set to 0.
         //    This is something to keep in mind for custom implementations
         //    of the get_cell_click_info().
         // 3. Another situation that might have a 0 data_id is if the
         //    intent is to create a new record.
         if (!(SFW.has_value(info,"data_id")))
         {
            this.diagnose_missing_data_id(info.target);
            return true;
         }
         else if ((id=info.data_id))
         {
            var result = ("result" in info) ? info.result : this.result();
            function f(n) { return n.nodeType==1 && n.getAttribute(info.id_name)==id; }
            xrow = SFW.find_child_matches(result, f, true);
         }
      }

      // Look for custom handlers, first at the object level as a object procedure,
      // then at the global level, as window[(function_name=info.task)]().
      if (SFW.has_value(this,info.task))
         return this[info.task](info.target, xrow, info);
      if (SFW.has_value(window,info.task))
         return window[info.task](info.target, xrow, info);

      // If still here, we'll open a dialog starting with the task as a url,
      // modified as appropriate according to what else is available:
      if ("task" in info)
      {
         var url = info.task

         if (xrow && id)
         {
            if (url.indexOf('&')==-1)
               url += "=" + id;
            else
               url = SFW.apply_row_context(url, xrow);
         }

         var os = SFW.get_page_offset();  // Get offset before discarding contents
         var host = this.host();

         var open_obj = { os:os, host:host };
         if (xrow)
            open_obj.xrow = xrow;

         SFW.open_interaction(SFW.stage,
                              url,
                              this,
                              open_obj
                             );

         return false;
      }

      return true;
   };

   
})();
