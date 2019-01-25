
// sfw_selectx.js

(function _init()
{
   var bclass="iclass";
   if ((!("SFW" in window) && setTimeout(_init,100))
       || SFW.delay_init("sfw_selectx", _init, bclass))
      return;

   if (!SFW.derive(_selectx, "selectx", bclass))
      return;

   var _bs=8, _enter=13, _left=37, _up=38, _right=39, _down=40, _esc=27;

   function _selectx(actors) { SFW.base.call(this,actors); }

   _selectx.prototype.process = function(e,t)
   {
      switch(e.type)
      {
         // case "focus":
         //    return this.process_focus(e,t);
         case "blur":
            return this.process_blur(e,t);
         case "mouseup":
            return this.process_click(e,t);
         case "keydown":
            // Confine ESC keypress to closing selectx, not closing the form.
            // (Getting keycode cheaper than is_activated(), which must traverse the DOM tree.)
            if (SFW.keycode_from_event(e)==_esc && this.is_activated())
            {
               this.deactivate();
               return false;
            }
            else
               return this.process_key(e,t);
      };

      return true;
   };

   function is_li(n) { return n.nodeType==1 && n.tagName.toLowerCase()=="li"; }

   _selectx.prototype.update_contents = function(newdoc,type,child)
   {
      var crow = child.get_context_row();
      var newrow = SFW.get_update_row(newdoc);
      if (newrow && !crow)
      {
         var result = this.get_contents_result();
         var idname = SFW.get_result_idname(result);
         var rid = newrow.getAttribute(idname);
         if (rid)
         {
            var tagname = result.getAttribute("row-name");
            var copyrow = SFW.addXMLEl(tagname,result);
            SFW.copy_attributes(copyrow, newrow);

            // var input = this.get_post_input();
            // input.value += (input.value.length?",":"") + rid;
            this.refill_li_options_from_result();

            var masked = this.get_masked_input();
            this.filter_options(masked.value);
            masked.focus();
         }
      }
   };

   // Interrupt cascade, which is not appropriate here because
   // the update is from adding a list element.
   _selectx.prototype.cascade_updates = function(newdoc,type,child)
   {
      this.update_contents(newdoc,type,child);
   };

   function get_window_origin(el)
   {
      var obj={ top  : el.offsetTop,
                left : el.offsetLeft };

      if (el.offsetParent)
      {
         var pobj = get_window_origin(el.offsetParent);
         obj.top  += pobj.top;
         obj.left += pobj.left;
      }

      return obj;
   }


   // Event-processing methods:
   _selectx.prototype.process_focus = function(e,t)
   {
      var t_display = this.get_display_div();
      var t_input = this.get_masked_input();
      if (!this.is_activated() && t==t_display)
         this.activate();

      return false;
   };

   _selectx.prototype.process_blur = function(e,t)
   {
      var is_active = this.is_activated();
      var t_display = this.get_display_div();
      var t_input = this.get_masked_input();

      if (is_active && t==t_input && !class_includes(t,"entry"))
         this.deactivate();

      return false;
   };
   
   _selectx.prototype.process_click = function(e,t)
   {
      var node, id;
      if ((node=SFW.self_or_ancestor_by_tag(t,"span"))
          && (id=node.getAttribute("data-id")))
      {
         var li = this.get_li_by_id(id);
         class_remove(li,"on");
         this.remove_id(id);
      }


      if (!this.is_activated() && class_includes(t,"display"))
         this.activate();
      else if ((node = SFW.self_or_ancestor_by_tag(t,"li")))
         this.fire_target(node);
         
      return false;
   };

   _selectx.prototype.process_key = function(e,t)
   {
      var keycode = SFW.keycode_from_event(e);
      var is_active = this.is_activated();
      var ths = this;
      switch(keycode)
      {
         case _down:
            if (is_active)
               this.move_target(1);
            else
               this.activate();
            return false;
         case _up:
            if (is_active)
               this.move_target(-1);
            return false;
         case _enter:
            if (is_active)
               this.process_enter_press(e);
            else
               this.activate();
            return false;
         case _esc:
            if (is_active)
               this.deactivate();
            return true;
         default:
            if (keycode >= 32 || keycode==_bs)
            {
               if (!is_active)
                  this.activate();

               var masked_input = this.get_masked_input();
               function f() { ths.filter_options(masked_input.value); }
               window.setTimeout(f, 50);
               return false;
            }
            break;
      }
      
      return true;
   };

   // Component access methods
   _selectx.prototype.get_mask_form = function()
   {
      return SFW.find_child_matches(this.widget(),"form",true,false);
   };

   _selectx.prototype.get_ul = function()
   {
      var mform = this.get_mask_form();
      if (mform)
         return SFW.find_child_matches(mform,"ul",true,false);
      return null;
   };

   _selectx.prototype.get_masked_input = function()
   {
      var mform = this.get_mask_form();
      if (mform)
         return SFW.find_child_matches(mform,"input",true,false);
      return null;
   };

   _selectx.prototype.get_post_input = function()
   {
      return SFW.find_child_matches(this.widget(),"input",true,false);
   };

   _selectx.prototype.get_display_div = function()
   {
      function f(n) { return n.nodeType==1 && class_includes(n,"display"); }
      return SFW.find_child_matches(this.widget(),f,true,true);
   };

   _selectx.prototype.get_displayed_array = function()
   {
      var arr = [];
      var div = this.get_display_div();
      var spans = div.getElementsByTagName("span");
      if (spans.length)
      {
         var nodes = spans[0].childNodes;
         for (var i=0, stop=nodes.length; i<stop; ++i)
         {
            var n = nodes[i];
            if (n.nodeType==3)
               arr.push(n.data.toLowerCase());
         }
      }
      return arr;
   };

   _selectx.prototype.get_li_by_id = function(id)
   {
      function f(n) { return is_li(n) && n.getAttribute("data-value")==id; }
      return SFW.find_child_matches(this.get_ul(), f, true, true);
   };

   // XML document access method(s)

   _selectx.prototype.get_schema = function()
   {
      var form, fhost, obj;
      if ((form = this.get_host_form())
          && (fhost = form.parentNode)
          && (obj = SFW.get_object_from_host(fhost)))
      {
         return obj.schema();
      }
      return null;
   };

   _selectx.prototype.get_schema_field = function()
   {
      var schema = this.get_schema();
      if (schema)
      {
         var input = this.get_post_input();
         var xpath = "field[@name='" +  input.getAttribute("name") + "']";
         return schema.selectSingleNode(xpath);
      }
      return null;
   };

   _selectx.prototype.get_contents_result = function()
   {
      var field, xpath;
      if ((field=this.get_schema_field()))
      {
         xpath = "/*/" + field.getAttribute("result") + "[@rndx]";
         return field.ownerDocument.selectSingleNode(xpath);
      }
      return null;
   };

   // Status access methods

   _selectx.prototype.get_style = function()
   {
      return this.widget().getAttribute("data-style");
   };

   _selectx.prototype.get_target_li = function()
   {
      var sel = null;
      function f(n) { return is_li(n) && class_includes(n,"target"); }
      var ul = this.get_ul();
      if (ul)
         sel = SFW.find_child_matches(ul,f,true,false);

      return sel;
   };

   _selectx.prototype.is_activated = function()
   {
      return class_includes(this.widget(),"active") ? true : false;
   };

   _selectx.prototype.get_post_array = function()
   {
      var str = this.get_post_input().value;
      if (str.length)
         return str.split(",");
      else
         return [];
   };

   _selectx.prototype.remove_id = function(id)
   {
      var ids = this.get_post_array();
      var pos = ids.indexOf(id);
      if (pos!=-1)
      {
         ids.splice(pos,1);
         this.get_post_input().value = ids.join(',');
      }
   };

   _selectx.prototype.append_id = function(id)
   {
      var ids = this.get_post_array();
      var pos = ids.indexOf(id);
      if (pos==-1)
      {
         ids.push(id);
         this.get_post_input().value = ids.join(',');
      }
   };

   _selectx.prototype.set_id = function(id)
   {
      this.get_post_input().value = id;
   };

   // Methods that change the user's view:

   _selectx.prototype.activate = function(e,t)
   {
      class_add(this.widget(),"active");

      var minput = this.get_masked_input();
      minput.value = "";
      minput.focus();
      
      this.get_masked_input().focus();
      this.filter_options();

      return false;
   };

   _selectx.prototype.deactivate = function(e,t)
   {
      this.unfilter_options();

      var disp = this.get_display_div();
      class_remove(this.widget(),"active");
      if (disp)
         disp.focus();

      var ael = document.activeElement;
      if (disp != ael)
      {
         if (ael)
            alert("Unexpectedly, the "
                  + ael.tagName.toLowerCase()
                  + " element is active.");
         else
            alert("Unexpectedly, there is no active element (item with focus)?");
      }

      return false;
   };

   _selectx.prototype.fire_target = function(target)
   {
      var id = target.getAttribute("data-value");
      var is_style_single = this.get_style()=="single";

      var sels=[];

      function f(n)
      {
         if (is_li(n))
         {
            if (class_includes(n,"on"))
                sels.push(n);
            if (class_includes(n,"target"))
               class_remove(n,"target");
         }
         return false;
      }

      // Scan options for "on" and "target" classes:
      SFW.find_child_matches(this.get_ul(),f,false,false);

      if (class_includes(target,"on"))
      {
         class_remove(target,"on");
         this.remove_id(id);
      }
      else
      {
         if (is_style_single)
         {
            var dval;
            if (sels && sels[0]!=target)
            {
               var dval = target.getAttribute("data-value");
               class_remove(sels[0],"on");
            }
            this.set_id(dval);
         }
         else
            this.append_id(id);

         class_add(target,"on");
      }

      var masked = this.get_masked_input();
      masked.value = "";
      masked.focus();

      this.filter_options("");
      this.update_li_options_from_value();
      this.update_display_from_value();
      this.ensure_target_visible(target);

      class_add(target,"target");

   };

   _selectx.prototype.old_fire_target = function(target)
   {
      var sels = [];

      function toggle(n)
      {
         if (class_includes(n,"on"))
            class_remove(n,"on");
         else
            class_add(n,"on");
      }

      function li_is_on(n) { return is_li(n) && class_includes(n,"on"); }

      var li_target = null;
      function set_target(n)
      {
         if (!li_target)
         {
            li_target = n;
            class_add(n,"target");
         }
      }

      function multi(n)
      {
         class_remove(n,"target");

         if (li_is_on(n))
         {
            set_target(n);
            sels.push(n);
         }

         // Always return false to prevent exit on first_only
         return false;
      }

      function single(n)
      {
         if (li_is_on(n))
         {
            class_remove(n,"on");
            // Trigger early-terminate because of first_only
            return true;
         }
         return false;
      }

      // Using closure-global ul variable
      function proc_li_els(func)
      {
         SFW.find_child_matches(ul, func, true, true);
      }

      // Save initial setting of target element:
      var ison = class_includes(target,"on");
      var ul = this.get_ul();
      if (ul)
      {
         var f = null;
         if (this.get_style() == "multiple")
         {
            // Toggle first, process next
            // so process saves all selections;
            toggle(target);
            proc_li_els(multi);
         }
         else
         {
            // Process first, toggle next
            // so all selections are cleared before
            // selecting the target (if it was off).
            proc_li_els(single);
            if (!ison)
            {
               toggle(target);
               sels.push(target);
            }
         }

         this.set_from_selections(sels);
         this.deactivate();
      }
   };

   

   _selectx.prototype.refill_li_options_from_result = function()
   {
      var idlist = this.get_post_input().value;
      
      var field, ul;
      if ((field = this.get_schema_field())
          && (ul = this.get_ul()))
      {
         field.setAttribute("selectx_ul", idlist);
         SFW.xslobj.transformFill(ul, field);
         field.removeAttribute("selectx_ul");
      }
   };

   _selectx.prototype.update_li_options_from_value = function()
   {
      var idarr = this.get_post_input().value.split(',');
      var val = this.get_masked_input().value;

      function f(n)
      {
         if (is_li(n))
         {
            


            class_remove(n,"on");
            var id = n.getAttribute("data-value");
            if (idarr.indexOf(id) != -1)
               class_add(n,"on");
         }
         return false;
      }
      SFW.find_child_matches(this.get_ul(),f);
   };
   
   _selectx.prototype.update_display_from_value = function()
   {
      var idlist = this.get_post_input().value;
      
      var field, display;
      if ((field = this.get_schema_field())
          && (display = this.get_display_div()))
      {
         field.setAttribute("selectx_display", idlist);
         SFW.xslobj.transformFill(display, field);
         field.removeAttribute("selectx_display");
      }
   };

   _selectx.prototype.set_from_selections = function(arr)
   {
      var input_for_post = this.get_post_input();

      var arrids = [];
      for (var i=0,stop=arr.length; i<stop; ++i)
         arrids.push(arr[i].getAttribute("data-value"));

      var idlist = arrids.join(',');

      input_for_post.value = idlist;

      var field, display;
      if ((field = this.get_schema_field())
          && (display = this.get_display_div()))
      {
         field.setAttribute("selectx_display", idlist);
         SFW.xslobj.transformFill(display, field);
         field.removeAttribute("selectx_display");
      }
   };

   _selectx.prototype.get_on_add_url = function()
   {
      var result = this.get_contents_result();
      if (result)
      {
         var url = result.selectSingleNode("@on_add|schema/@on_add");
         if (url)
            return url.nodeValue;
      }
      return null;
   };

   _selectx.prototype.process_enter_press = function(e)
   {
      var inpval = this.get_masked_input().value;
      var sel = this.get_target_li();

      if (!sel && inpval.length > 0)
      {
         var url = this.get_on_add_url();
         if (url)
         {
            SFW.open_interaction(SFW.stage,
                                 url,
                                 this,
                                 { os:SFW.get_page_offset(),
                                   this:this.host(),
                                   preset_value:inpval }
                                );
         }
      }
      else if (sel)
         this.fire_target(sel);
   };

   _selectx.prototype.move_target = function(dir)
   {
      var visibles = [];
      var pos_max, pos_cur=-1, pos_new=-1;
      function f(n)
      {
         if (is_li(n))
         {
            if (n.style.display!="none")
            {
               visibles.push(n);
               if (class_includes(n,"target"))
                  pos_cur = visibles.length-1;
            }
         }
      };

      var ul = this.get_ul();
      if (ul)
      {
         SFW.find_child_matches(ul,f,false,true);
         pos_max = visibles.length-1;
         if (pos_cur==-1)
            pos_new=0;
         else if (dir<0)
            pos_new = pos_cur-1;
         else if (dir>0)
            pos_new = pos_cur+1;

         if (pos_new<0)
            pos_new = 0;
         else if (pos_new>pos_max)
            pos_new = pos_max;

         if (pos_cur!=pos_new)
         {
            var newel = visibles[pos_new];

            if (pos_cur>=0)
               class_remove(visibles[pos_cur],"target");
            if (pos_new>=0)
            {
               class_add(newel,"target");
               this.ensure_target_visible(newel);
            }
         }
      }
   };

   _selectx.prototype.get_on_array = function()
   {
      function f(n) { return is_li(n) && class_includes(n,"on"); }
      return SFW.find_child_matches(this.get_ul(),f,false,false);
   };

   _selectx.prototype.ensure_target_visible = function(li)
   {
      if (!li)
      {
         var nl = this.get_on_array();
         if (nl.length)
            li = nl[0];
      }

      if (li)
      {
         var top_li = li.offsetTop;

         if (li.parentNode != li.offsetParent)
            top_li -= li.parentNode.offsetTop;

         var bottom_li = top_li + li.offsetHeight;

         var parent = li.parentNode;
         var bottom_parent = parent.offsetHeight;
         var scroll_parent = parent.scrollTop;

         var offset_move;

         if ((offset_move=bottom_li-bottom_parent-scroll_parent)>0)
            parent.scrollTop += offset_move;
         else if ((offset_move=scroll_parent-top_li)>0)
            parent.scrollTop -= offset_move;
      }
   };

   // Methods that update elements' status

   _selectx.prototype.unfilter_options = function()
   {
      function f(n)
      {
         if (is_li(n))
            n.style.display = "block";
      };

      var ul = this.get_ul();
      if (ul)
         SFW.find_child_matches(ul,f,false,true);
   };

   // This method is called on two occasions, when the control
   // is activated, or when the input text has changed.  In both
   // cases, the list is filtered (elements are hidden or displayed
   // according to partial matches of the typed string), and a
   // target li should identified and marked with the "target"
   // class name.
   _selectx.prototype.filter_options = function(filter_str)
   {
      // Conditionally resolve, only used by closure function enroll():
      var is_match;
      var arr_select = null;
      if (!filter_str)
      {
         var arr = this.get_displayed_array();
         // var arr = this.scan_for_targets();
         if (arr)
         {
            var stop = arr.length;
            is_match = function(text)
            {
               for(var i=0; i<stop; ++i)
                  if (arr[i] == text)
                     return true;
               return false;
            };
         }
         else
            is_match = function() { return false; };
      }

      filter_str = filter_str?filter_str.toLowerCase():"";

      var target = null;

      function set_target(n)
      {
         if (!target)
         {
            class_add(n,"target");
            target = n;
         }
      }

      function filter(n)
      {
         var s = n.style;
         var text = n.firstChild.data.toLowerCase();
         if (text.search(filter_str)==-1)
            s.display = "none";
         else
         {
            set_target(n);
            s.display = "block";
         }
      }

      function enroll(n)
      {
         n.style.display = "block";
         if (is_match(n.firstChild.data.toLowerCase()))
            set_target(n);
      }

      var pfunc = filter_str ? filter : enroll;

      function f(n)
      {
         if (is_li(n))
         {
            class_remove(n,"target");
            pfunc(n);
         }
      };

      SFW.find_child_matches(this.get_ul(),f,false,true);
      if (target)
         this.ensure_target_visible(target);
   };


})();
