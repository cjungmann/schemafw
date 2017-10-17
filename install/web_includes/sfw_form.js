
// sfw_form.js

(function _init()
{
   if ((!("SFW" in window) && setTimeout(_init,100))
       || SFW.delay_init("sfw_form",_init,"iclass"))
      return;

   if (!SFW.derive(_form, "form", "iclass"))
      return;

   // derive() is not called for _form_new or _form_edit until the prototypes
   // of their base classes are complete.  Look for those derives at the end
   // of the module.

   function _form(actors)
   {
      SFW.base.call(this, actors);
   }

   // Make copies of _form for prototype property "class_name" (RTTI).
   function _form_new(base, doc, caller, data) { _form.apply(this, arguments); }
   function _form_edit(base, doc, caller, data) { _form.apply(this, arguments); }

   // Adding useful local functions to global object
   SFW.get_form_data        = _get_form_data;
   SFW.focus_on_first_field = _focus_on_first_field;
   
   function _find_first_editable_field(form)
   {
      var els = form.elements;
      for (var i=0,stop=els.length; i<stop; ++i)
      {
         var el = els[i];
         if ("value" in el && el.type!="button" && el.type!="hidden" && !el.readOnly)
            return el;
      }
      return null;
   }

   function _focus_on_first_field(dlg)
   {
      var el = _find_first_editable_field(dlg);
      if (el)
      {
         el.focus();
         if (el.getAttribute("type")=="text")
         {
            el.selectionStart = 0;
            el.selectionEnd = el.value.length;
         }
      }
      
      // var nl = dlg.getElementsByTagName("label");
      // if (nl && nl.length>0)
      // {
      //    var label = nl[0];
      //    var name = label.getAttribute("for");
      //    function f(n) { return n.nodeType==1 && n.getAttribute("name")==name; }
      //    var el = SFW.find_child_matches(label.parentNode, f, true);
      //    if (el)
      //    {
      //       el.focus();
      //       if (el.getAttribute("type")=="text")
      //       {
      //          el.selectionStart = 0;
      //          el.selectionEnd = el.value.length;
      //       }
      //    }
      // }
   }

   function _get_multiple_value(el)
   {
      var str = "";
      function f(n)
      {
         if (n.nodeType==1 && n.selected)
            str += (str.length?",":"") + n.value;
      }

      SFW.find_child_matches(el, f);

      return str;
   }

   function _get_form_data(form)
   {
      var el, els = form.elements;
      var arr = [];
      var noninputs = 'submit reset button';
      for (var i=0, stop=els.length; i<stop; i++)
      {
         el = els[i];
         if (noninputs.search(el.type)==-1 && el.name.length)
         {
            if (el.type=="checkbox")
            {
               if (el.checked)
                  arr.push(el.name + "=1");
            }
            else if ('value' in el && el.value.length>0)
            {
               if ("multiple" in el && el.multiple)
                  arr.push(el.name + "=" + _get_multiple_value(el));
               else
                  arr.push(el.name + "=" + encodeURIComponent(el.value));
            }
         }
      }
      return arr;
   }

   _form.prototype.focus_on_first_field = function()
   {
      var top = this.top();
      _focus_on_first_field(this.top());
   };

   _form.prototype.get_first_editable_form_field = function()
   {
      var top = this.top();
      return top?_find_first_editable_field(top):null;
   };

   _form.prototype.process_submit = function _process_submit()
   {
      var form = this.top();
      var arr = _get_form_data(form);
      var url = form.getAttribute("action");
      var enctype = form.getAttribute("enctype");
      var headers = enctype ? [{name:"enctype", value:enctype}] : null;

      var ths = this;
      function cb_good(doc)
      {
         if (SFW.check_for_preempt(doc))
         {
            SFW.add_update_results(doc);

            if (ths.caller())
               ths.caller().child_finished(ths.cfobj_from_doc(doc));
         }
      }

      function cb_bad(xhr)
      {
         console.error(xhr.ResponseText);
      }

      xhr_post(url,arr.join("&"),
               cb_good, cb_bad, headers);
   };

   _form.prototype.process_button_delete = function(t,cb)
   {
      var url = t.getAttribute("data-task") || t.getAttribute("data-url");
      xhr_get(url,cb);
   };

   var _xpath_delete_check = "/*[@mode-type='delete']/*[@rndx=1]/*[1]/@deleted";
   function _is_failed_delete_request(cmd)
   {
      var a;
      return ("documentElement" in cmd
              && (a=cmd.selectSingleNode(_xpath_delete_check))
                 && a.nodeValue=="0");
   }

   _form.prototype.set_field = function _set_field(name,value)
   {
      var form = this.top();
      var el, els = form.elements;
      for (var i=0, stop=els.length; i<stop; i++)
      {
         el = els[i];
         if (el.name == name)
         {
            el.value = value;
            break;
         }
      }
   };

   _form.prototype.process_button = function _process_button(e,t)
   {
      var ths = this;
      function fdone(cmd)
      {
         if (_is_failed_delete_request(cmd))
            SFW.alert("Delete operation failed.");
         else if (ths.caller())
            ths.caller().child_finished(ths.cfobj_from_cmd(cmd));
      }
      
      return this.process_clicked_button(t, fdone);
   };

   _form.prototype.initialize = 
   _form.prototype.post_transform = function()
   {
      this.focus_on_first_field();
   };

   _form.prototype.process = function _form_process_message(e,t)
   {
      if (e.type!="click")
         return true;

      switch(t.type)
      {
      case "button":
         if (!this.process_button(e,t))
            return false;
      case "submit":
         if (this.caller())
         {
            this.process_submit();
            e.preventDefault();
            return false;
         }
         break;
      }
      
      return true;
   };

   // With _form prototype complete, we can derive other classes from it:
   if(!SFW.derive(_form_new, "form-new", "form") ||
      !SFW.derive(_form_edit, "form-edit", "form"))
      return;
})();
