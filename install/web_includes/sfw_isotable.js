(function _init()
 {
    var file_name="sfw_isotable", base_class_name="iclass", class_name="isotable";
    
    if ((!("SFW" in window) && setTimeout(_init,100))
        || SFW.delay_init(file_name, _init, base_class_name)
        || SFW.delay_init(file_name, _init, "form"))    // for _isotable_form at bottom
       return;

    var ctor = _isotable;

    if (!SFW.derive(ctor, class_name, base_class_name))
       return;

    function _isotable(actors)
    {
       SFW.base.call(this,actors);
    }

    _isotable.prototype.process = function(e,t)
    {
       // Allow base class to process generic buttons:
       if (!SFW.base.prototype.process.call(this,e,t))
          return false;

       var tagname = t.tagName.toLowerCase();

       if (e.type=="click")
       {
          var tr, rpos;
          if ((tr=((tagname=="tr") ? t : SFW.ancestor_by_tag(t, "tr")))
              && (rpos=tr.getAttribute("data-pos")) > 0)
             this.create_form(tr);
       }

       if (tagname=="button")
       {
          if (e.type=="mouseup")
          {
             var title = t.getAttribute("title");
             var action = title?title.split(' ')[0]:null;
             switch(action)
             {
                case "add":
                   this.create_form();
                   break;
             }
          }
       }
       return false;
    };

   function makel(r){return r.ownerDocument.createElement(r.getAttribute("row-name"));}

   _isotable.prototype.get_result = function()
   {
      if (!this._result)
      {
         var xpath = "/*/schema/field[@type='isotable'][@name='"
                + this.widget().getAttribute("name")  + "']";

         var doc, field, rxpath, result;
         if ((doc=this.xmldoc())
             && (field=doc.selectSingleNode(xpath)))
         {
            xpath="/*/" + field.getAttribute("result") + "[@rndx]";
            this._result = doc.selectSingleNode(xpath);
         }
      }
      return this._result;
   };

   _isotable.prototype.get_host_form = function()
   {
      return SFW.ancestor_by_tag(this.widget(), "form");
   };

   _isotable.prototype.get_tbody = function()
   {
      return SFW.find_child_matches(this.widget(), "tbody", true, true);
   };

   _isotable.prototype.get_input = function()
   {
      var form_row = this.widget().parentNode;
      function f(n)
      {
         return n.nodeType==1 && n.tagName.toLowerCase()=="input" && n.getAttribute("type")=="hidden";
      }
      return SFW.find_child_matches(form_row, f, true, true);
   };

   _isotable.prototype.replot = function()
   {
      var result, input, tbody;
      if ((tbody=this.get_tbody())
          && (input=this.get_input())
          && (result=this.get_result()))
      {
         // replot the table rows:
         result.setAttribute("iso_replot","table");
         SFW.xslobj.transformFill(tbody, result);

         // Calculate and replace the input value attribute:
         result.setAttribute("iso_replot","value");
         var str = SFW.xslobj.transformToString(result);
         input.value = str;

         // Restore result to original state:
         result.removeAttribute("iso_replot");
      }
   };
   

   _isotable.prototype.get_linked_result = function()
   {
      var schema, field, result=null;
      if ((schema=this.schema())
          && (field=schema.selectSinglenode("field[@type='isotable']")))
      {
         var xpath = "/*/" + field.getAttribute("result") + "[@rndx]";
         result = this.xmldoc().selectSingleNode(xpath);
      }

      return result;
   };

   _isotable.prototype.create_form = function(tr)
   {
      var row, result;
      if (!(tr && (row=this.get_xml_row(tr)) && (result=row.parentNode)))
         result = this.get_result();

      if (!result)
      {
         console.error("Failure to find a schema prevents building the form.");
         return;
      }

      var uschema = row?row.selectSingleNode("../schema"):result.selectSingleNode("schema");

      var hform = this.get_host_form();
      // var fhost = SFW.make_sfw_host(SFW.stage, this.xmldoc());
      var fhost = SFW.make_sfw_host(SFW.stage, this.xmldoc(), this, {row:row});
      SFW.size_to_cover(fhost, hform);

      result.setAttribute("iso_replot","form");
      SFW.xslobj.transformFill(fhost,(row?row:result));
      result.removeAttribute("iso_replot");
      SFW.arrange_in_host(fhost, SFW.seek_child_anchor(fhost));
   };

   _isotable.prototype.save_form_row = function(tr)
   {
      var result, form, newel;
      if ((result = this.get_result())
          && (form = SFW.ancestor_by_tag(tr,"form"))
          && (newel = makel(result)))
      {
         SFW.get_form_data_xml(form, newel);
         result.appendChild(newel);
         this.replot();
      }
   };

   _isotable.prototype.get_xml_row = function(tr)
   {
      var result, pos, rname, xpath;
      if ((result=this.get_result())
          && (pos=tr.getAttribute("data-pos"))>=0
          && (rname=result.getAttribute("row-name"))
          && (xpath=rname+"["+pos+"]"))
         return result.selectSingleNode(xpath);
      return null;
   };

   _isotable.prototype.remove_row = function(t)
   {
      var row = this.get_xml_row(t);
      if (row)
      {
         row.parentNode.removeChild(row);
         this.replot();
      }
   };

   // Second class in closure
   function _isotable_form(actors) { SFW.base.call(this,actors); }

   _isotable_form.prototype.process_button_delete = function(t,cb)
   {
      var data, row;
      if (this.has_data() && (data=this.data()))
         row = data["row"];

      if (row)
      {
         row.parentNode.removeChild(row);
         this.caller().replot();
         this.dismantle();
      }
   };

   _isotable_form.prototype.process_submit = function()
   {
      var form, caller, result, newel;
      if ((form = this.top())
          && (caller = this.caller())
          && (result = caller.get_result())
          && (newel = makel(result)))
      {
         SFW.get_form_data_xml(form, newel);
         if (SFW.integrate_element(result, newel))
         {
            caller.replot();
            this.dismantle();
         }
      }

      return false;
   };

   if (!SFW.derive(_isotable_form, "isotable_form", "form"))
      return;
 }
)();




