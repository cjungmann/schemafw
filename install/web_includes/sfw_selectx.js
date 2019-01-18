
// sfw_selectx.js

(function _init()
{
   var bclass="iclass";
   if ((!("SFW" in window) && setTimeout(_init,100))
       || SFW.delay_init("sfw_selectx", _init, bclass))
      return;

   if (!SFW.derive(_selectx, "selectx", bclass))
      return;

   function _selectx(actors) { SFW.base.call(this,actors); }

   _selectx.prototype.process = function(e,t)
   {
      switch(e.type)
      {
         case "focus":
            return this.process_focus(e,t);
         case "blur":
            return this.process_blur(e,t);
         case "click":
            return this.process_click(e,t);
         case "keyup":
            return this.process_keyup(e,t);
      };

      return true;
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
      var ul = this.get_ul();
      if (ul)
         ul.style.display="block";
      return false;
   };

   _selectx.prototype.process_blur = function(e,t)
   {
      var ul = this.get_ul();
      if (ul)
      {
         ul.style.display="none";
         this.unfilter_options();
      }
      return false;
   };

   _selectx.prototype.process_click = function(e,t)
   {
      var node = SFW.self_or_ancestor_by_tag(t,"li");
      if (node)
      {
         this.fire_target(node);


         
      }
      return false;
   };

   _selectx.prototype.process_press_enter = function()
   {
      var sel = this.get_target_li();
      if (sel)
         this.fire_target(sel);
   };

   _selectx.prototype.process_keyup = function(e,t)
   {
   // SFW.keycode_from_event   = _keycode_from_event;
   // SFW.keychar_from_event   = _keychar_from_event;

      var _enter=13, _left=37, _up=38, _right=39, _down=40;
      var keycode = SFW.keycode_from_event(e);
      switch(keycode)
      {
         case _down:
            this.move_target(1);
            return false;
         case _up:
            this.move_target(-1);
            return false;
         case _enter:
            this.process_press_enter();
            return false;
         default:
            console.log("keycode is " + keycode);
            this.filter_options(t.value);
      }
      
      return true;
   };

   // Component access methods
   _selectx.prototype.get_mask_form = function()
   {
      var widget = this.widget();
      return SFW.find_child_matches(widget,"form",true,false);
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
      var widget = this.widget();
      return SFW.find_child_matches(widget,"input",true,false);
   };

   _selectx.prototype.get_display_div = function()
   {
      var widget = this.widget();
      function f(n) { return n.nodeType==1 && class_includes(n,"display"); }
      return SFW.find_child_matches(widget,f,true,false);
   };

   // XML document access method(s)

   _selectx.prototype.get_schema_field = function()
   {
      var form, fhost, obj, schema, xpath;
      if ((form = this.get_host_form())
          && (fhost = form.parentNode)
          && (obj = SFW.get_object_from_host(fhost))
          && (schema = obj.schema()))
      {
         var widget = this.widget();
         xpath = "field[@name='" +  widget.getAttribute("name") + "']";
         return schema.selectSingleNode(xpath);
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
      function f(n)
      {
         return (n.nodeType==1
                 && n.tagName.toLowerCase()=="li"
                 && class_includes(n,"target"));
      }

      var ul = this.get_ul();
      if (ul)
         sel = SFW.find_child_matches(ul,f,true,false);

      return sel;
   };

   // Methods that change the user's view:

   _selectx.prototype.replot = function(val)
   {
      var field, widget;
      if ((field = this.get_schema_field())
          && (widget = this.widget()))
      {
         field.setAttribute("selectx_replot", val);
         SFW.xslobj.transformFill(widget, field);
         field.removeAttribute("selectx_replot");
      }
   };

   _selectx.prototype.replot_display = function(val)
   {
      var field, display;
      if ((field = this.get_schema_field())
          && (display = this.get_display_div()))
      {
         field.setAttribute("selectx_display", val);
         SFW.xslobj.transformFill(display, field);
         field.removeAttribute("selectx_display");
      }
   };

   _selectx.prototype.fire_target = function(target)
   {
      var ison = class_includes(target,"on");

      var sels = [];

      function toggle(n)
      {
         if (class_includes(n,"on"))
            class_remove(n,"on");
         else
            class_add(n,"on");
      }

      function does_match(n)
      {
         return n.nodeType==1
            && n.tagName.toLowerCase()=="li"
            && class_includes(n,"on");
      }

      function multi(n)
      {
         if (does_match(n))
            sels.push(n);

         // Always return false to prevent exit on first_only
         return false;
      }

      function single(n)
      {
         if (does_match(n))
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
      }


         var inp = this.set_input();
         if (inp)
         {
            this.replot(inp.value);
         }
      
   };

   _selectx.prototype.set_display = function(arr)
   {
      if (this.get_style()=="multiple")
      {
         var disp = this.get_display_div();
         for (var i=0, stop=arr.length; i<stop; ++i)
         {
            
         }
      }
      else
      {
      }
   };


   _selectx.prototype.set_from_selections = function(arr)
   {
      var masked = this.get_masked_input();
      var hidden = this.get_post_input();

      var arrids = [];
      for (var i=0,stop=arr.length; i<stop; ++i)
         arrids.push(arr[i].getAttribute("data-value"));

      var idlist = arrids.join(',');

      hidden.value = masked.value = idlist;

      if (this.get_style()=="multiple")
      {
         var field = this.get_schema_field();
         if (field)
         {
            field.setAttribute("selectx_display", idlist);
            
         }
      }
      
      this.set_display(arr);
   };

   _selectx.prototype.move_target = function(dir)
   {
      var visibles = [];
      var pos_max, pos_cur=-1, pos_new=-1;
      function f(n)
      {
         if (n.nodeType==1 && n.tagName.toLowerCase()=="li")
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
               class_add(newel,"target");
         }
      }
   };

   // Methods that update elements' status

   _selectx.prototype.unfilter_options = function()
   {
      function f(n)
      {
         if (n.nodeType==1 && n.tagName.toLowerCase()=="li")
            n.style.display = "block";
      };

      var ul = this.get_ul();
      if (ul)
         SFW.find_child_matches(ul,f,false,true);
   };

   _selectx.prototype.filter_options = function(str)
   {
      function f(n)
      {
         if (n.nodeType==1 && n.tagName.toLowerCase()=="li")
         {
            var text = n.firstChild.data;
            n.style.display = text.search(str)==-1 ? "none" : "block";
         }
      };

      var ul = this.get_ul();
      if (ul)
         SFW.find_child_matches(ul,f,false,true);
   };

   _selectx.prototype.set_input = function()
   {
      var widget = this.widget();
      var selected=[];
      var input = null;
      function f(n)
      {
         if (n.nodeType==1)
         {
            switch(n.className)
            {
               case "input":
                  input = n;
                  break;
               case "on":
                  selected.push(n.getAttribute("data-value"));
                  break;
            }
         }
         return false;
      }

      // Only read first-generation children for li elements:
      SFW.find_child_matches(widget,f,true,false);

      if (input && (input=SFW.find_child_matches(input,"input")))
         input.value = selected.join(',');

      return input;
   };


})();
