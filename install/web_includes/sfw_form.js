
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
   SFW.focus_on_first_field = _focus_on_first_field;
   SFW.process_ebutton      = _process_ebutton;
   SFW.get_form_data        = _get_form_data;
   SFW.get_form_data_xml    = _get_form_data_xml;

   function _find_first_editable_field(form)
   {
      var arr = SFW.collect_form_fields(form);
      if (arr)
      {
         for (var i=0,stop=arr.length; i<stop; ++i)
         {
            var el = arr[i];
            if (!("type" in el) || el.type!="button")
               return el;
         }
      }
      return null;
   }
   
   // Global, SFW member function
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
   }

   // Global, SFW member function
   function _process_ebutton(button, callback)
   {
      var calling_form, schema, label, fieldname;
      if ((calling_form = SFW.seek_event_object(button))
          && (schema = calling_form.schema())
          && (label = button.parentNode)
          && (fieldname = label.getAttribute("for"))
         )
      {
         if (label.tagName.toLowerCase()!="label")
            SFW.alert("Assumption that button is child of label is no longer valid.");

         var xpath = "field[@name='" + fieldname + "']";
         var field = schema.selectSingleNode(xpath);
         var input = SFW.next_sibling_element(label);

         // Note that properties of the following object must be
         // independent of the Javascript objects that are created
         // during an event.  "input" is an input field of the
         // calling form.  It will not go out of scope while the
         // subordinate form is displayed.
         var data_bundle = {
            input : input
         };

         // Setup source for transformation:
         field.setAttribute("construct_field_form", "true");
         var data_el = add_namespace_el("data", null, field);
         data_el.setAttribute(fieldname, input.value);

         SFW.create_interaction(field, calling_form, data_bundle);

         // Unsetup source for transformation:
         field.removeChild(data_el);
         field.removeAttribute("construct_field_form");
      }
      else
         SFW.alert("Couldn't find the label, schema, or form.");
   };


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

   // Global, SFW member function
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

   /**
    * This unusual function should collect all input-type elements
    * under a given element in case the "form" is not really a form.
    */
   function _get_element_elements(el)
   {
      var formels = [];
      function f(n)
      {
         if (n.nodeType==1 && "form" in n && "name" in n)
            formels.push(n);
      }
      SFW.find_child_matches(el, f, false, true);

      return formels;
   }

   /**
    * This function can be used to populate an XML element
    * with the fields of a form.  This can be used to
    * add new rows for local construction of a collection of
    * XML elements or for submission to a web services server.
    */
   // Global, SFW member function
   function _get_form_data_xml(form, outel, save_skips)
   {
      // Default value is true:
      if (arguments.length<3)
         save_skips = false;

      var el, els = form.elements || _get_element_elements(form);
      var skip;
      var noninputs = 'submit reset button';
      for (var i=0, stop=els.length; i<stop; i++)
      {
         el = els[i];
         skip = !save_skips && el.getAttribute("data-xml-skip");
         if (!skip && noninputs.search(el.type)==-1 && el.name.length)
         {
            if (el.type=="checkbox")
               outel.setAttribute(el.name, (el.checked?"1":"0"));
            else if ('value' in el && el.value.length>0)
            {
               if ("multiple" in el && el.multiple)
                  outel.setAttribute(el.name, _get_multiple_value(el));
               else
                  outel.setAttribute(el.name, encodeURIComponent(el.value));
            }
         }
      }
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

   _form.prototype.get_data_path_row = function()
   {
      var xpath,form = this.top();

      if ((xpath=form.getAttribute("data-path")))
         return SFW.xmldoc.selectSingleNode(xpath);
      else
         return null;
   };

   _form.prototype.get_context_row = function()
   {
      return SFW.get_property(this,"host","data","xrow");
   };

   _form.prototype.update_context_row = function(newrow)
   {
      var data = SFW.get_property(this,"host","data");
      if (data)
         data.xrow = newrow;
   };

   _form.prototype.get_saved_os = function()
   {
      return SFW.get_property(this,"host","data","os");
   };

   _form.prototype.find_form_input = function(name)
   {
      var labels = this.top().getElementsByTagName("label");
      for (var i=0, stop=labels.length; i<stop; ++i)
      {
         var f = labels[i];
         if (f.getAttribute("for")==name)
         {
            f = f.nextSibling;
            while (f)
            {
               if (f.nodeType==1 && f.getAttribute("name")==name)
                  return f;
               else
                  f = f.nextSibling;
            }
         }
      }
      return null;
   };

/**
   This function should branch based on the action taken
   by the returned_doc.
   1. Delete document: remove the row from any field views
   2. Add document: add new data to any field views
   3. Otherwise, it's an edit, just update.
   
   Note that the returned_doc will have already been incorporated
   into (or out of) the XML document.  This function is only 
   responsible for updating the display to conform to the existing data.
*/
   _form.prototype.update_contents = function(returned_doc, type, ichild)
   {
      var row, schema, fields;
      if ((row=this.get_data_path_row() || this.get_context_row()) &&
          (schema=this.schema())
          && (fields=schema.selectNodes("field[@type='linked']")))
      {
         for (var i=0, stop=fields.length; i<stop; ++i)
         {
            var fld = fields[i];
            var name = fld.getAttribute("name");
            var input = this.find_form_input(name);
            if (input)
            {
               row.setAttribute("lookup-field-match", name);
               SFW.xslobj.transformFill(input, row);
               row.removeAttribute("lookup-field-match");
            }
         }
      }
   };

   _form.prototype.preview_result = _form.prototype.update_contents;

   _form.prototype.use_process_submit = function _use_process_submit()
   {
      var form, sclass;
      if (this.caller())
         return true;
      else if ((form=this.top()) && (sclass=form.getAttribute("data-sfw-class")))
      {
         if (sclass=="form-jump")
            return true;
      }
      return false;
   };

   _form.prototype.process_form_jump = function _process_form_jump(doc)
   {
      var result, row, error, attr, jumps, url, msg;
      if ((result = doc.selectSingleNode("*/*[@rndx][@type='variables'][1]"))
          && (row=result.selectSingleNode(result.getAttribute("row-name"))))
      {
         if ((error=row.getAttribute("error")) || error=='0')
         {
            attr = "jump"+error;
            if ((jumps=result.selectSingleNode("*[@" + attr + "]")))
               url = jumps.getAttribute(attr);
         }

         msg = row.getAttribute("msg");
         if (msg)
            SFW.alert(msg);

         if (url)
            window.location = url;
         else
         {
            SFW.alert("Missing jump target for error number " + error);
            this.sfw_unhide();
         }
      }
   };

   _form.prototype.process_submit = function _process_submit()
   {
      var form = this.top();
      var type = form.getAttribute("data-sfw-class");
      var arr = _get_form_data(form);
      var url = form.getAttribute("action");
      var enctype = form.getAttribute("enctype");
      var headers = enctype ? [{name:"enctype", value:enctype}] : null;

      var ths = this;
      function cb_good(doc)
      {
         if (SFW.check_for_preempt(doc))
         {
            var dtype = doc.documentElement.getAttribute("mode-type");

            if (type=="form-jump" || dtype=="form-jump")
               ths.process_form_jump(doc);
            else
            {
               SFW.update_xmldoc(doc, ths, type);

               var caller = ths.caller();
               if (caller)
               {
                  // caller.preview_result(doc, ths);
                  // caller.child_finished(ths);

                  caller.cascade_updates(doc,type,ths);
                  caller.restart(ths);
                  ths.dismantle();
               }
            }
         }
      }

      function cb_bad(xhr)
      {
         console.error(xhr.ResponseText);
      }

      this.sfw_hide();

      xhr_post(url,arr.join("&"),
               cb_good, cb_bad, headers);
   };

   _form.prototype.process_button_delete = function(t,cb)
   {
      var url = t.getAttribute("data-task") || t.getAttribute("data-url");
      xhr_get(url,cb);
   };

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
         var attr, caller;

         // Look at _base.prototype.process_clicked_button()...

         // fdone is called with string=='cancel' is a confirmation
         // message was NOT confirmed.  We will need to rework this
         // code if fdone gets a string argument in a context other
         // than a declined confirmation dialog.
         if (typeof(cmd)==="string" && cmd=="cancel")
            return; // without closing the dialog

         var is_xmldoc = SFW.is_xmldoc(cmd);

         if (is_xmldoc)
         {
            if (SFW.check_for_preempt(cmd))
            {
               var mtype = cmd.documentElement.getAttribute("mode-type");
               var is_del_mode = mtype=="delete";
               var update_ok = SFW.update_xmldoc(cmd,ths,mtype);

               if (is_del_mode && !update_ok)
               {
                  SFW.alert("Delete operation failed");
                  return;  // return without closing the dialog
               }
            }
         }

         if ((caller=ths.caller()))
         {
            if (cmd!="cancel" && is_xmldoc)
            {
               var form = ths.top();
               var type = form.getAttribute("data-sfw-class");
               caller.cascade_updates(cmd,type,ths);
            }
            ths.dismantle();
            // caller.child_finished(ths,cmd=="cancel");
        }
      }
      return this.process_clicked_button(t, fdone);
   };

   _form.prototype.initialize = 
   _form.prototype.post_transform = function()
   {
      this.focus_on_first_field();
   };

   _form.prototype.closeable = function() { return true; };

   _form.prototype.process = function _form_process_message(e,t)
   {
      if (e.type=="focus")
      {
         SFW.ensure_element_visibility(t);
         return true;
      }

      if (e.type!="click")
         return true;

      switch(t.type)
      {
      case "button":
         if (!this.process_button(e,t))
            return false;
      case "submit":
         if (this.use_process_submit())
         {
            this.process_submit();
            SFW.cancel_event(e);
         }
         break;
      }
      
      return true;
   };

   function _form_single(actors) { SFW.base.call(this, actors); }
   _form_single.prototype.process_submit = function()
   {
      var form_data = _get_form_data(this.top());
      if (form_data.length == 0)
         return;

      var farr = form_data[0].split('=');
      var fname = farr[0];
      var fval = decodeURIComponent(farr[1]);

      var host_data, input, caller;
      if ((host_data=this.data())
          && (input=host_data.input)
          && (input.name == fname)
          && (caller=this.caller()))
      {
         input.value = fval;
         this.dismantle();
      }

   };

   // With _form prototype complete, we can derive other classes from it:
   if(!SFW.derive(_form_new, "form-new", "form")
      || !SFW.derive(_form_edit, "form-edit", "form")
      || !SFW.derive(_form_edit, "form-jump", "form")
      || !SFW.derive(_form_edit, "form-page", "form")
      || !SFW.derive(_form_single, "form-single", "form")
     )
      return;
})();
