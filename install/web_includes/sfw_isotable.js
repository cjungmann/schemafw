(function _init()
 {
    var file_name="sfw_isotable", base_class_name="iclass", class_name="isotable";
    
    if ((!("SFW" in window) && setTimeout(_init,100))
        || SFW.delay_init(file_name, _init, base_class_name))
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
          if (tagname=="img")
          {
             var title = t.getAttribute("title");
             var action = title?title.split(' ')[0]:null;
             switch(action)
             {
                case "add":
                   this.add_row(t);
                   break;

                case "remove":
                   this.remove_row(t);
                   break;
             }
          }
       }
       return true;
    };

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

   _isotable.prototype.get_tbody = function()
   {
      return SFW.find_child_matches(this.widget(), "tbody", true, true);
   };

   _isotable.prototype.replot = function()
   {
      var result, tbody;
      if ((tbody=this.get_tbody()) && (result=this.get_result()))
      {
         result.setAttribute("iso_replot","true");
         SFW.xslobj.transformFill(tbody, result);
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

   function get_tagged_parent(node, tag)
   {
      var p;
      if (node.tagName.toLowerCase()==tag)
         return node;
      else if ((p=node.parentNode))
         return get_tagged_parent(p,tag);
      else
         return null;
   }

   _isotable.prototype.save_form_row = function(tr)
   {
      function makel(r){return r.ownerDocument.createElement(r.getAttribute("row-name"));}

      var result, form, newel;
      if ((result = this.get_result())
          && (form = get_tagged_parent(tr,"form"))
          && (newel = makel(result)))
      {
         SFW.get_form_data_xml(form, newel);
         result.appendChild(newel);
         this.replot();
      }
   };

   _isotable.prototype.add_row = function(t)
   {
      var cname = "isotable_form";
      var tr = get_tagged_parent(t,"tr");

      if (class_includes(tr,cname))
         class_remove(tr,cname);
      else
      {
         if (this.save_form_row(tr))
            this.replot();
      }
      
      // var result = this.get_result();
      // if (result)
      // {
      // }
   };

   _isotable.prototype.get_xml_row = function(t)
   {
      var result, tr, pos, rname, xpath;
      if ((result=this.get_result())
          &&(tr=get_tagged_parent(t,"tr"))
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
  
 }
)();




