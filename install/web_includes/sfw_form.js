function init_SFW_Forms()
{
   SFW.types["form-new"] = _form;
   SFW.types["form-edit"] = _form;

   SFW.get_form_data        = _get_form_data;
   SFW.focus_on_first_field = _focus_on_first_field;

   function _form(base, doc, caller)
   {
      SFW.base.call(this, base,doc,caller);
      _focus_on_first_field(base);
   }

   SFW.derive(_form, SFW.base);
   
   function _find_first_editable_field(form)
   {
      var els = form.elements;
      for (ndx in els)
      {
         var el = els[ndx];
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

   function _get_form_data(form)
   {
      var el, els = form.elements;
      var arr = [];
      var noninputs = 'submit reset button';
      for (var i=0, stop=els.length; i<stop; i++)
      {
         el = els[i];
         if (noninputs.search(el.type)==-1)
         {
            if (el.type=="checkbox")
            {
               if (el.checked)
                  arr.push(el.name + "=1");
            }
            else if ('value' in el && el.value.length>0)
               arr.push(el.name + "=" + encodeURIComponent(el.value));
         }
      }
      return arr;
   }

   _form.prototype.process_submit = function _process_submit()
   {
      var form = this.top();
      var caller = this._caller;
      var arr = _get_form_data(form);
      var url = form.getAttribute("action");
      var enctype = form.getAttribute("enctype");
      var headers = enctype ? [{name:"enctype", value:enctype}] : null;

      function cb_good(doc)
      {
         if (SFW.check_for_preempt(doc))
         {
            if (caller)
               caller.child_finished(doc);
         }
      }

      function cb_bad(xhr)
      {
      }

      xhr_post(url,arr.join("&"),
               cb_good, cb_bad, headers);
   };

   _form.prototype.process_button = function _process_button(e,t)
   {
      var ths = this;
      function fdone(cmd) { if (ths._caller) ths._caller.child_finished(cmd||null); }
      
      return this.process_clicked_button(t, fdone);
   };

   _form.prototype.process = function _form_process_message(e,t)
   {
      if (!this.confirm_owned(t))
         return true;

      if (e.type!="click")
         return true;

      switch(t.type)
      {
         case "button":
            if (!this.process_button(e,t))
               return false;
         case "submit":
            if (this._caller)
            {
               this.process_submit();
               e.preventDefault();
               return false;
            }
            break;
      }
         
      return true;
   };

}