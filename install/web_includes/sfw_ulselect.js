
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
      this.shadow = this.seek_input_element(actors);
   }

   // Primary public method for integration with framework:
   _ulselect.prototype.process = function(e,t)
   {
      if (e.type!="click")
         return true;

      var field = this.get_input_field();

      var val;
      if ((val=t.getAttribute("data-id")))
      {
         this.set_li_class_by_id(t,"in");
         return this.remove_selection(val);
      }
      else if ((val=t.getAttribute("data-value")) && t.className=="out")
      {
         t.className = "in";
         return this.add_selection(val);
      }

      return true;
   };

   _ulselect.prototype.seek_input_element = function(actors)
   {
      function f(n) { return n.nodeType==1 && n.tagName.toLowerCase()=="input"; }
      if ("input" in actors)
         return SFW.find_child_matches(actors.input, f, true, true);

      console.error("Unable to find \"input\" in actors.");
      return null;
   };

   _ulselect.prototype.set_li_class_by_id = function(t,val)
   {
      var li, id = t.getAttribute("data-id");

      function f(n) { return n.nodeType==1 && n.getAttribute("data-value")==id; }
      var el = SFW.find_child_matches(t.parentNode,f,true,false);
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
      var schema, row;
      if ((schema=this.schema()) && (row=this.get_host_form_data_row()))
         return row.selectSingleNode("@kws");
      else
         return null;
   };

   _ulselect.prototype.get_value_attribute = function()
   {
      return this.dattr || (this.dattr=this.seek_value_attribute());
   };

   _ulselect.prototype.update_selections = function()
   {
      var attr = this.get_value_attribute();
      var li = this.get_selected_li();

      SFW.xslobj.transformFill(li, attr);
      this.shadow.value = attr.value;
   };

   function _is_selected_li(n)
   {
      return n.nodeType==1 && n.tagName.toLowerCase()=="li" && n.className=="selected";
   }

   _ulselect.prototype.get_selected_li = function()
   {
      var input = this.input();
      if (input)
         return SFW.find_child_matches(input, _is_selected_li, true, false);
      else
         return null;
   };

   _ulselect.prototype.add_selection = function(val)
   {
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
