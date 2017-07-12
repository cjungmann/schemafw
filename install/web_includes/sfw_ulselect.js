
// sfw_select.js

(function _init()
{
   if (SFW.delay_init("sfw_ulselect", _init, "iclass"))
      return;

   if (!SFW.derive(_ulselect, "ulselect", "iclass"))
      return;

   function _ulselect(actors)
   {
      SFW.base.call(this,actors);
      var ths = this;
      function fi(n)
      {
         if (n.nodeType==1 && n.tagName.toLowerCase()=="input")
         {
            if (n.className=="transfer")
               ths.input_transfer = n;
            else
               ths.input_el = n;
         }
         return false;
      }
      
      SFW.find_child_matches(actors.input,fi,true,true);
      
      function fu(n) { return n.nodeType==1 && n.tagName.toLowerCase()=="ul"; }
      this.ul_options = SFW.find_child_matches(actors.input,fu,true,true);
   }

   // Primary public method for integration with framework:
   _ulselect.prototype.process = function(e,t)
   {
      var field = this.get_input_field();
      if (e.type=="focus")
         return this.process_focus(e,t);
      else if (e.type=="blur")
         return this.process_blur(e,t);
      if (e.type.substring(0,3)=="key")
         return this.process_key(e,t);
      else if (e.type=="click")
      {
         var val;
         if ((val=t.getAttribute("data-id")))
         {
            this.set_li_class_by_id(val,"out");
            return this.remove_selection(val);
         }
         else if ((val=t.getAttribute("data-value")) && t.className=="out")
         {
            t.className="in";
            return this.add_selection(val);
         }
      }

      return true;
   };

   _ulselect.prototype.set_option_selected = function(el)
   {
      el.className = "selected";
   };

   _ulselect.prototype.filter_options = function(str, unselect)
   {
      str = str.toLowerCase();
      var strempty = str.length==0;
      var matches = true;
      function f(n)
      {
         if (n.nodeType==1)
         {
            if (unselect && n.className=="selected")
               n.className = "out";

            if (!strempty)
               matches = n.firstChild.data.toLowerCase().search(str)>=0;
            
            n.style.display = matches ? "" : "none";
            return matches;
         }
         return false;
      }
      var nl = SFW.find_child_matches(this.ul_options,f,false,false);
      if (nl && nl.length==1)
         this.set_option_selected(nl[0]);
   };

   _ulselect.prototype.update_input_progress = function(e,t)
   {
      var keycode = SFW.keycode_from_event(e);
      var val = this.input_el.value;
      this.filter_options(val,true);
   };

   _ulselect.prototype.reveal_options = function()
   {
      this.ul_options.style.display = "block";
   };

   _ulselect.prototype.conceal_options = function()
   {
      this.ul_options.style.display = "none";
   };

   _ulselect.prototype.options_are_visible = function()
   {
      return this.ul_options.style.display!="none";
   };

   _ulselect.prototype.refresh_options = function()
   {
      var name, schema, field;
      if ((name=this.get_field_name())
          && (schema=this.schema())
          && (field=schema.selectSingleNode("field[@name='" + name + "']")))
      {
         SFW.xslobj.transformFill(this.ul_options,field);
      }
   };

   _ulselect.prototype.process_focus = function(e,t)
   {
      if (t.tagName.toLowerCase()=="input")
         this.reveal_options();
      return false;
   };

   _ulselect.prototype.process_blur = function(e,t)
   {
      if (t.tagName.toLowerCase()=="input")
         this.conceal_options();
      return false;
   };

   _ulselect.prototype.remove_last_selection = function()
   {
      var span = this.get_selections_span();
      var id, n = span.lastChild;
      while (n)
      {
         if (n.nodeType==1 && n.className=="item" && n.tagName.toLowerCase()=="span")
            break;
          n=n.previousSibling;
      }
      
      if (n
          && (n=n.getElementsByTagName("span"))
          && (n=n[0])
          && (id=n.getAttribute("data-id")))
      {
         this.set_li_class_by_id(id,"out");
         this.remove_selection(id);
      }
      
   };

   _ulselect.prototype.seek_current_selection = function()
   {
      function f(n)
      {
         return n.nodeType==1
            && n.className=="selected"
            && n.tagName.toLowerCase()=="li";
      }
      return SFW.find_child_matches(this.ul_options,f,true,false);
   };

   _ulselect.prototype.close_and_clear = function()
   {
      this.conceal_options();
      this.input_el.value = "";
      this.filter_options("");
      this.input_el.focus();
   };

   _ulselect.prototype.input_is_empty = function()
   {
      return this.input_el.value=="";
   };

   _ulselect.prototype.process_enter_press = function(e,t)
   {
      var el = this.seek_current_selection();
      if (el)
      {
         el.className = "in";
         this.add_selection(el.getAttribute("data-value"));
         this.close_and_clear();
         e.preventDefault();
         return true;
      }
      return false;
   };

   _ulselect.prototype.process_arrow_press = function(up)
   {
      var top, prev, cur, before;
      var ths = this; // for lambda function
      
      function swap(nw, old)
      {
         nw.className = "selected";
         old.className = "out";
         cur = nw;
      }
      function f(n)
      {
         if (n.nodeType==1)
         {
            var cn = n.className;
            if (cn=="in" || n.style.display=="none")
               return false;
            
            if (!top && cn=="out")
               top = n;

            if (up)
            {
               if (cn=="selected")
               {
                  if (prev) swap(prev,n);
                  else if (top) swap(top,n);
                  return true;
               }
               else
                  prev = n;
            }
            else // not-up (down)
            {
               if (!ths.options_are_visible())
               {
                  ths.reveal_options();
                  ths.refresh_options();
               }

               if (prev)
               {
                  swap(n,prev);
                  return true;
               }
               else if (cn=="selected")
                  cur = prev = n;
            }
         }
         return false;
      }

      var el = SFW.find_child_matches(this.ul_options, f, true, false);
      if (!el && !cur && !up && top)
         top.className = "selected";
   };

   _ulselect.prototype.process_key = function(e,t)
   {
      var keycode = SFW.keycode_from_event(e);
      if (e.type=="keyup")
      {
         if (keycode==8 || keycode==46|| keycode>48)
         {
            if (!this.options_are_visible())
               this.reveal_options();
            this.update_input_progress(e,t);
            return true;
         }
      }
      else if (e.type=="keydown")
      {
         switch(keycode)
         {
         case 8:
         case 46:
            // We have to check before letter is erased or
            // erasing the last letter will also erase the
            // previous selected option.
            if (this.input_is_empty())
            {
               this.remove_last_selection();
               this.conceal_options();
               return false;
            }
            break;
            
         case 13: // enter key
            return this.process_enter_press(e,t);
         case 37: // left
         case 39: // right
            break;

         case 38: // up
            return this.process_arrow_press(1);
         case 40: // down
            return this.process_arrow_press(0);
            break;
         default:
            break;
         }
      }

      return false;
   };

   _ulselect.prototype.seek_input_element = function(actors)
   {
      function f(n) { return n.nodeType==1 && n.tagName.toLowerCase()=="input"; }
      if ("input" in actors)
         return SFW.find_child_matches(actors.input, f, true, true);

      console.error("Unable to find \"input\" in actors.");
      return null;
   };

   _ulselect.prototype.set_li_class_by_id = function(id,val)
   {
      function f(n) { return n.nodeType==1 && n.getAttribute("data-value")==id; }
      var el = SFW.find_child_matches(this.ul_options,f,true,false);
      if (el)
         el.className = val;
      else
         console.log("Unable to find select line.");
   };

   _ulselect.prototype.get_data_row = function()
   {
      return this.drow || (this.drow=this.get_host_form_data_row());
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

   _ulselect.prototype.update_selections = function()
   {
      var attr = this.get_value_attribute();
      var span = this.get_selections_span();

      SFW.xslobj.transformFill(span, attr);
      this.input_transfer.value = attr.value;
   };

   _ulselect.prototype.get_selections_span = function()
   {
      var li, span, tag;
      function f(n)
      {
         if (n.nodeType==1)
         {
            tag = n.tagName.toLowerCase();
            if (li)
            {
               if (tag=="span")
                  return true;
            }
            else if (tag=="li" && n.className=="selections")
               li = n;
         }
         return false;
      }
      var input = this.input();
      if (input)
         return SFW.find_child_matches(input, f, true, true);
      else
         return null;
   };

   _ulselect.prototype.add_selection = function(val)
   {
      if (val==0)
         return false;

      var drow = this.get_data_row();
      var fname = this.get_field_name();

      var list = drow.getAttribute(fname);
      if (list && list.length)
         list += "," + val;
      else
         list = val;

      drow.setAttribute(fname, list);

      this.update_selections();
      return false;
   };

   _ulselect.prototype.remove_selection = function(val)
   {
      var drow = this.get_data_row();
      var fname = this.get_field_name();

      var list = drow.getAttribute(fname);
      var arr = list.split(',');
      var found = false;
      for (var i=0,stop=arr.length; !found && i<stop; ++i)
      {
         if (arr[i] == val)
         {
            found = true;
            arr.splice(i,1);
            break;
         }
      }

      if (found)
      {
         drow.setAttribute(fname, arr.join(','));
         this.update_selections();
      }

      return false;
   };

})();
