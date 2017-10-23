
// sfw_ulselect.js

(function _init()
{
   if ((!("SFW" in window) && setTimeout(_init,100))
       || SFW.delay_init("sfw_ulselect", _init, "tbase"))
      return;

   if (!SFW.derive(_ulselect, "ulselect", "tbase"))
      return;

   function _ulselect(actors)
   {
      SFW.base.call(this,actors);
      var widget = this.widget();
      var ths = this;
      function fi(n)
      {
         if (n.nodeType==1)
         {
            var tn = n.tagName.toLowerCase();
            if (tn=="input")
            {
               if (n.className=="transfer")
                  ths._input = n;
               else
                  ths.input_el = n;
            }
         }
         return false;
      }
      SFW.find_child_matches(widget,fi,true,true);
      
      function fu(n) { return n.nodeType==1 && n.tagName.toLowerCase()=="ul"; }
      this.ul_options = SFW.find_child_matches(widget,fu,true,true);

      this.is_multiple = this.get_schema_field_attribute("style")=="multiple";
   }

   // Primary public method for integration with framework:
   _ulselect.prototype.process = function(e,t)
   {
      if (e.type=="blur")
         return this.process_blur(e,t);
      if (e.type=="focus")
         return this.process_focus(e,t);
      if (e.type.substring(0,3)=="key")
         return this.process_key(e,t);
      else if (e.type=="mousedown")
      {
         var val;
         // Elements with @data-id are contained in span elements that
         // are displayed in the selections element to show the selected
         // options of a multiple-select input field.  A click on this
         // element means the user wants to remove the option.
         if ((val=t.getAttribute("data-id")))
         {
            this.set_li_class_by_id(val,"out");
            this.remove_selection(val);
            this.update_defacto_display();
            this.close_and_clear();
            this.stage_input_focus();
            return false;
         }
         // Elements with @data-value are li elements in the list of
         // available options.  Clicking on one means the option should
         // replace a previously selected option (single mode) or be added
         // added to the selections list (multiple mode).
         else if ((val=t.getAttribute("data-value")) && t.className=="out")
         {
            this.select_option(t);
            return false;
         }
      }

      return true;
   };

   _ulselect.prototype.process_focus = function(e,t)
   {
      var tn = t.tagName.toLowerCase();
      if ((tn=="ul" && t.className=="ulselect")
          || (tn=="li" && t.className=="cluster"))
      {
         this.shift_typeable(true);
         return false;
      }
      return true;
   };

   _ulselect.prototype.process_blur = function(e,t)
   {
      // if (t.tagName.toLowerCase()=="input")
      //    this.close_and_clear();
      if (t==this.input_el)
      {
         this.close_and_clear();
         return false;
      }
      return true;
   };

   _ulselect.prototype.process_key = function(e,t)
   {
      var keycode = SFW.keycode_from_event(e);
      if (e.type=="keyup")
      {
         if (keycode>48 && !this.options_are_visible())
            this.reveal_options();

         // Process keypresses that affect the display after the
         // browser has updated the input (on keyup).
         if (keycode==8 || keycode==46 || keycode>48)
         {
            this.update_options_filter();
            return true;
         }
      }
      else if (e.type=="keydown")
      {
         switch(keycode)
         {
         case 8:
         case 46:
            // We have to check before letter is erased (at keydown)
            // or erasing the last letter will also erase the previous
            // selected option.
            if (this.is_multiple && this.input_is_empty())
            {
               this.remove_last_selection();
               this.conceal_options();
               return false;
            }
            break;

         case 27: // escape key
            this.close_and_clear();
            return true;
            
         case 13: // enter key
            return this.process_enter_press(e,t);
         case 37: // left
         case 39: // right
            break;

         case 38: // up
            return this.process_arrow_press(1);
         case 40: // down
            return this.process_arrow_press(0);
         default:
            this.size_input();
            break;
         }
      }

      return false;
   };

   _ulselect.prototype.size_input = function()
   {
      var cnt = this.input_el.value.length;
      var width = String((cnt<6?6:cnt) / 2) + "em";
      this.input_el.style.width = width;
   };

   _ulselect.prototype.process_enter_press = function(e,t)
   {
      var el = this.seek_preselect();
      if (el)
      {
         this.select_option(el);
         // Don't let ENTER submit the form when we're updating the options
         e.preventDefault();
         this.stage_input_focus();
         return true;
      }
      else
      {
         var val = this.input_el.value.trim();
         if (val.length>0)
         {
            var url = this.get_schema_field_attribute("on_add");
            if (url)
            {
               SFW.open_interaction(SFW.stage,
                                    url,
                                    this,
                                    { os:SFW.get_page_offset(),
                                      host:this.host(),
                                      prefill:val }
                                   );
               e.preventDefault();
               return true;
            }
         }
      }
      return false;
   };

   function _get_prefill_value(obj)
   {
      return SFW.get_property(obj.host(), "data", "prefill");
      // var h, d, p=null;
      // if ((h=obj.host()) && (d=("data" in h)?h.data:null))
      //    p = "prefill" in d ? d.prefill : null;
      // return p;
   }

   function _set_prefill_field(obj, val)
   {
      var sfield = obj.get_schema_field("field[@prefill]");
      if (sfield && "set_field" in obj)
         obj.set_field(sfield.getAttribute("name"), val);
      else if ("get_first_editable_form_field" in obj && (sfield=obj.get_first_editable_form_field()))
         sfield.value = val;
   }

   // Prevent replot since it doesn't make sense for this control.
   _ulselect.prototype.replot = function(result) {};

   _ulselect.prototype.child_ready = function(obj)
   {
      var val;
      if ((val=_get_prefill_value(obj)))
         _set_prefill_field(obj,val);
   };

   _ulselect.prototype.preview_result = function(returned_doc, child)
   {
      SFW.types["tbase"].prototype.preview_result.call(this,returned_doc,child);

      var crow = child.get_context_row();
      var xpath = "/*/*[@rndx][@type='update']/*[local-name()=../@row-name]";
      var newrow = returned_doc.selectSingleNode(xpath);
      if (newrow && !crow)
      {
         var rid = newrow.getAttribute("id");
         if (rid)
            this.add_selection(rid);
      }
   };

   /** At present, the only reason I can think that a child
    *   would have been opened is for adding a missing option,
    *   in which case the lookup table would need to be updated
    *   AND the new option should be used as the current seletion.
    */
   _ulselect.prototype.child_finished = function(child, cancelled)
   {
      // Must call base::child_finished() to clean out
      // any merged elements before calling replot().
      SFW.base.prototype.child_finished.call(this, child, cancelled);

      if (!cancelled)
      {
         this.refresh_options();
         this.update_defacto_display();
      }

      // Restore previous page position when form closes.
      var os = child.get_saved_os();
      if (os)
         SFW.set_page_offset(os);
   };

   _ulselect.prototype.process_arrow_press = function(up)
   {
      var ths = this; // save for lambda function

      var top, prev, cur, before;
      var list_hidden = !this.options_are_visible();
      var select_top = !up && list_hidden;
      
      function swap(nw, old)
      {
         nw.className = "selected";
         old.className = "out";
         cur = nw;
         return true;
      }
      function f(n)
      {
         if (n.nodeType==1)
         {
            var cn = n.className;
            var option_hidden = n.style.display=="none";

            if (cn=="in" || option_hidden)
               return false;

            if (!top && cn!="in" && !option_hidden)
               top = n;

            if (top && list_hidden)
            {
               if (cn=="selected")
                  n.className = "out";
               return false;
            }

            if (up)
            {
               if (cn=="selected")
                  return n==top?true:swap(prev?prev:top,n);
               else
                  prev = n;
            }
            else // not-up (down)
            {
               if (prev)
                  return swap(n,prev);
               else if (cn=="selected")
                  cur = prev = n;
            }
         }
         return false;
      }

      // Set first_only flag (3rd param) to stop scan early:
      var el = SFW.find_child_matches(this.ul_options, f, true, false);
      
      if (list_hidden)
         this.reveal_options();

      if (!el && !cur && !up && top)
         top.className = "selected";

      this.ensure_option_visible(cur||top);
   };

   // This function scans the list of options, doing for each:
   // 1. Hide options without a substring matching _value_,
   // 2. Mark options "in" if included in the list of selections,
   //    "out" otherwise.
   //
   // If the filtered list includes only a single option, that option
   // will be set as the preselect, waiting for a ENTER press.
   _ulselect.prototype.update_options_filter = function()
   {
      var val = this.get_value();
      var list = val.length ? ','+val+',' : null;

      // var attr = this.get_value_attribute();
      // var list = (attr && attr.value.length) ? ','+attr.value+',' : null;

      var value = this.input_el.value.toLowerCase();
      var matches = true;
      var id, isin;
      function f(n)
      {
         if (n.nodeType==1)
         {
            isin = (list
               && (id=n.getAttribute("data-value"))
               && list.search(','+id+',')>=0);
            
            n.className = isin ? "in" : "out";

            if (value.length)
               matches = n.firstChild && n.firstChild.data.toLowerCase().indexOf(value)>=0;

            n.style.display = matches ? "" : "none";
            return matches && !isin;
         }
         return false;
      }
      var nl = SFW.find_child_matches(this.ul_options,f,false,false);
      if (nl && nl.length==1)
         this.preselect_option(nl[0]);
   };

   _ulselect.prototype.preselect_option = function(el) {
      el.className = "selected"; };

   _ulselect.prototype.reveal_options = function() {
      this.ul_options.parentNode.style.display = "block"; };

   _ulselect.prototype.conceal_options = function() {
      this.ul_options.parentNode.style.display = "none"; };

   _ulselect.prototype.options_are_visible = function() {
      return this.ul_options.parentNode.style.display=="block"; };

   function _get_id_from_selection(li)
   {
      var id = null;
      if ((li=li.getElementsByTagName("span"))  && (li=li[0]))
         id = li.getAttribute("data-id");
      return id;
   }

   _ulselect.prototype.get_li_by_id = function(id)
   {
      function f(n) { return n.nodeType==1 && n.getAttribute("data-value")==id; }
      return SFW.find_child_matches(this.ul_options,f,true,false);
   };

   // In multiple-mode, this function finds the last span element
   // in the selection element.  When found, the ID is used to
   // call remove_selection(), which performs the actual removal.
   // options out of the selection element.
   _ulselect.prototype.remove_last_selection = function()
   {
      var span = this.get_defacto_span();
      var id, n = span.lastChild;
      while (n)
      {
         if (n.nodeType==1 && n.className=="item" && n.tagName.toLowerCase()=="span")
            break;
          n=n.previousSibling;
      }

      // The id value is actually an attribute of a nested span
      // element. Find that to get the id value:
      if (n && (id=_get_id_from_selection(n)))
      {
         this.set_li_class_by_id(id,"out");
         this.remove_selection(id);
         this.update_defacto_display();
      }
   };

   _ulselect.prototype.seek_preselect = function()
   {
      function f(n)
      {
         return n.nodeType==1
            && n.className=="selected"
            && n.tagName.toLowerCase()=="li";
      }
      return SFW.find_child_matches(this.ul_options,f,true,false);
   };

   _ulselect.prototype.shift_typeable= function(to_use)
   {
      var s = this.input_el.style;
      s.position = to_use?"static":"";
      s.display = to_use?"block":"";
      this.input_el.focus();
   };

   _ulselect.prototype.stage_input_focus = function()
   {
      var ths = this;
      function f() { ths.shift_typeable(true); }
      window.setTimeout(f,100);
   };

   _ulselect.prototype.close_and_clear = function(focus)
   {
      this.conceal_options();
      this.input_el.value = "";
      this.update_options_filter();
      if (focus)
         this.stage_input_focus();
      else
         this.shift_typeable(false);
   };

   _ulselect.prototype.clear_options = function()
   {
      function f(n)
      {
         if (n.nodeType==1 && n.className=="out")
            n.className="in";
         return false;
      }
      SFW.find_child_matches(this.ul_options,f,false,false);
   };

   _ulselect.prototype.clear_selections = function()
   {
      var p = this.input_el.parentNode;
      this._input.value = "";
      function f(n)
      {
         if (n.nodeType==1 && n.className=="item")
            p.removeChild(n);
         return false;
      }
      SFW.find_child_matches(p,f,false,false);
   };

   _ulselect.prototype.select_option = function(el)
   {
      if (this.is_multiple)
      {
         this.clear_options();
         this.clear_selections();
      }

      el.className = "in";
      this.add_selection(el.getAttribute("data-value"));
      this.update_defacto_display();
      this.close_and_clear();
   };

   _ulselect.prototype.input_is_empty = function()
   {
      return this.input_el.value=="";
   };

   _ulselect.prototype.set_li_class_by_id = function(id,val)
   {
      var el = this.get_li_by_id(id);
      if (el)
         el.className = val;
      else
         console.log("Unable to find select line.");
   };

   // The base _tbase::find_matching_data_row() is responsible for
   // returning the element in a dataset that is being edited so that
   // the old row can be replaced by the updated row returned from
   // the server.
   //
   // In a ulselect control, one only adds rows to a ulselect, rows are
   // never edited.  Thus, it is never appropriate to return anything
   // other than null;
   _ulselect.prototype.find_matching_data_row = function(cfobj)
   {
      return null;
   };

   _ulselect.prototype.seek_value_attribute = function()
   {
      var name, schema, row;
      if ((name=this.get_field_name())
          && (schema=this.schema())
          && (row=this.get_host_form_data_row()))
         return row.selectSingleNode("@"+name);
      else
         return null;
   };

   _ulselect.prototype.get_value_attribute = function() {
      return this.dattr || (this.dattr=this.seek_value_attribute());
   };

   _ulselect.prototype.get_defacto_span = function()
   {
      var nl = this.widget().getElementsByTagName("span");
      if (nl)
      {
         for (var i=0,stop=nl.length; i<stop; ++i)
            if (nl[i].className=="defacto")
               return nl[i];
      }
      return null;
   };

   _ulselect.prototype.update_defacto_display = function()
   {
      var span = this.get_defacto_span();
      if (span)
      {
         var fname = this.get_field_name();
         var value = this.get_value();

         var shadow = this.add_schema_shadow(fname,value);
         if (shadow)
         {
            var span, attr;
            if ((span=this.get_defacto_span()) && (attr=shadow.selectSingleNode("@"+fname)))
               SFW.xslobj.transformFill(span, attr);

            shadow.parentNode.removeChild(shadow);
         }
      }
   };

   _ulselect.prototype.get_value = function()
   {
      var i = this._input;
      var val = i.value || i.getAttribute("value");
      return val ? val.trim() : "";
   };

   /**
    * Abstract setting value because, at least for Chrome, both the attribute
    * "value" AND the property "value" must be set because the browser is not
    * updating the value property as defined in the HTML spec, and since the
    * value property is not set from the attribute, get_value() must get the
    * attribute value.
    */
   _ulselect.prototype.set_value = function(val)
   {
      var i=this._input;
      i.setAttribute("value",(i.value=val));
   }

   _ulselect.prototype.add_selection = function(val)
   {
      if (val==null || val==0)
         return false;

      var list = this.get_value();
      if (list.length==0)
         list = val;
      else if ((','+list+',').search(','+val+',')<0)
         list += ',' + val;

      this.set_value(list);
      return false;
   };

   _ulselect.prototype.remove_selection = function(val)
   {
      var list = this.get_value();
      if (list.length>0)
      {
         var arr = list.split(',');
         var pos = arr.indexOf(val);
         if (pos>=0)
         {
            arr.splice(pos,1);
            list = arr.join(',');
         }

         this.set_value(list);
      }
   };

   _ulselect.prototype.ensure_option_visible = function(li)
   {
      if (!li) return;

      var parent = li.parentNode;

      var top_li = li.offsetTop;
      var bottom_li = top_li + li.offsetHeight;

      var bottom_parent = parent.offsetHeight;
      var scroll_parent = parent.scrollTop;

      var offset_move;

      if ((offset_move=bottom_li-bottom_parent-scroll_parent)>0)
         parent.scrollTop += offset_move;
      else if ((offset_move=scroll_parent-top_li)>0)
         parent.scrollTop -= offset_move;
   };

   // Possible problem with this function is that that by replacing
   // everything, it makes orphans of any elements being held elsewhere.
   _ulselect.prototype.refresh_options = function()
   {
      var name, schema, field;
      if ((name=this.get_field_name())
          && (schema=this.schema())
          && (field=schema.selectSingleNode("field[@name='" + name + "']"))
          && field.getAttribute("options")!="off")
      {
            SFW.xslobj.transformFill(this.ul_options,field);
      }
   };


})();
