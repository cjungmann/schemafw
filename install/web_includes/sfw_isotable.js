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
                   this.add_row();
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

   _isotable.prototype.get_matched_row = function(t)
   {
      var rowid = t.getAttribute("data-id");
      var schema, field, result, xpath;
      var xd = this.xmldoc();

      function get_field()  { return schema.selectSingleNode("field[@type='isotable']"); }
      function get_result() { return xd.selectSingleNode("/*/"+field.getAttribute("result")+"[@rndx]"); }

      function get_row_xpath()
      {
         var pkey = schema.selectSingleNode("field[@primary-key]");
         if (!pkey)
            SFW.alert(serialize(schema));

         var rowname = schema.getAttribute("row-name");
         var fname = pkey.getAttribute("name");
         return rowname + "[@" + fname + "='" + rowid + "']";

         // return schema.getAttribute("row-name")
         //    + "[@" +  pkey.getAttribute("name") + "='" + rowid + "']";
      }

      if ((schema=this.schema())
          && (field=get_field())
          && (result=get_result())
          && (xpath=get_row_xpath()))
      {
         SFW.alert("xpath is '" + xpath + "'");
      }
   };

   _isotable.prototype.add_row = function()
   {
      
   };

   _isotable.prototype.get_row = function(t)
   {
      var result = this.get_result();
      if (result)
      {
         var schema = result.selectSingleNode("schema");
         var keyfield = schema.selectSingleNode("field[@primary-key]");
         if (keyfield)
         {
            var rxpath = result.getAttribute("row-name")
                   + "[@" + keyfield.getAttribute("name")
                   + "='" + t.getAttribute("data-id") + "']";

            return result.selectSingleNode(rxpath);
         }
      }
      return null;
   };

   _isotable.prototype.remove_row = function(t)
   {
      var row = this.get_row(t);
      if (row)
      {
         SFW.alert("Found the row! '" + serialize(row) + "'");
      }
   };
  
 }
)();




