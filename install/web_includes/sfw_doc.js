
// sfw_doc.js

(function _init()
{
   if (!("fake_SFW" in window))
      if ((!("SFW" in window) && setTimeout(_init,100))
            || SFW.delay_init("sfw_doc",_init,"iclass"))
      return;

   if (!("SFW" in window))
      window.SFW = {};

   SFW.alert             = _alert;
   SFW.confirm           = _confirm;
   SFW.alert_notice      = _alert_notice;
   SFW.is_xmldoc         = _is_xmldoc;
   SFW.get_schema_idfield= _get_schema_idfield;
   SFW.get_result_idname = _get_result_idname;
   SFW.get_result_xpath  = _get_result_xpath;
   SFW.check_for_preempt = _check_for_preempt;
   SFW.update_xsl_keys   = _update_xsl_keys;
   SFW.remove_xsl_keys   = _remove_xsl_keys;
   SFW.integrate_element = _integrate_element;
      

   function _alert(str)
   {
      window.alert(str);
   }

   function _confirm(str)
   {
      return window.confirm(str);
   }

   function _alert_notice(el)
   {
      var type = el.getAttribute("type");
      var msg = el.getAttribute("message");
      var where = el.getAttribute("where");
      var detail = el.getAttribute("detail");
      var str = type + ":\n" + msg;
      if (where)
         str += "\n\nwhere:\n" + where;
      else if (detail)
         str += "\n\ndetail:\n" + detail;
      SFW.alert(str);
   }

   function _is_xmldoc(e)
   {
      return typeof(e)==="object" && "documentElement" in e;
   };

   /** Searches for an id field, using most to least explicit methods. */
   function _get_schema_idfield(schema)
   {
      var field = null;
      if (schema)
      {
         field = schema.selectSingleNode("field[@primary-key]")
            || schema.selectSingleNode("field[@xrow_id]")
            || schema.selectSingleNode("field[@name='id']")
            || schema.selectSingleNode("field[1]");
      }

      return field;
   }

   /** Attempts to get a result name from a result.
    *
    * Looks for, in this order, attempting each step ig the previous step failed.
    * - in _get_schema_idfield()
    *   - name of field with primary-key attribute
    *   - name of field with xrow_id attribute
    * - the name of the first attribute of the data element in the result
    * - if all else fails, return "id" for empty result with no schema.
    */
   function _get_result_idname(result, row)
   {
      var attr, field = _get_schema_idfield(result.selectSingleNode("schema"));

      if (field)
         return field.getAttribute("name");
      else if ((row || (row=result.selectSingleNode("*[local-name()=../@row-name]")))
               && (attr=row.selectSingleNode("@*[1]")))
         return attr.name;
      else
         return "id";
   }

   function _get_result_xpath(result)
   {
      var xpath = "/*/";
      var rname = result.localName;

      if (rname=="result")
      {
         if (result.getAttribute("merged"))
            throw "Can't key on anonymous merged results.";

         xpath += "*[@rndx=" + result.getAttribute("rndx") + "][not(@merged)]";
      }
      else
         xpath += rname;

      return xpath;
   }

   function _check_for_preempt(doc)
   {
      var docel = doc.documentElement;
      var tname = docel.tagName.toLowerCase();
      var jump;
      if (tname=="notice")
      {
         SFW.alert_notice(docel);
         return false;
      }
      else if ((jump=docel.getAttribute("meta-jump")))
      {
         window.location = jump;
         return false;
      }
      else
      {
         var n = docel.firstChild;
         var errnode;
         while(n)
         {
            if (n.nodeType==1)
            {
               if (n.getAttribute("rndx"))
                  errnode = n.selectSingleNode("message");
               else if ((tname=n.tagName.toLowerCase()=="message"))
                  errnode = n;

               if (errnode)
               {
                  _alert_notice(errnode);
                  return false;
               }
            }

            n = n.nextSibling;
         }
      }
      return true;
   }

   var key_prefix = "gndx_";

   function _generate_key_name(result)
   {
      var rname = result.localName;
      var dname = result.ownerDocument.documentElement.getAttribute("script").replace(".","");
      return key_prefix + dname + "_" + rname;
   }

   function _get_xsl_document()
   {
      if ("XSLDocument" in window)
         return window.XSLDocument;
      if ("SFW" in window)
         return SFW.xsldoc;
      else
         return getXSLDocument();
   }

   function _lacks_xsl_key(name, xsl)
   {
      var key = xsl.selectSingleNode("/xsl:stylesheet/xsl:key[@name='" + name + "']");
      return !key;
   }

   /**
    * This function adds, to the XSL stylesheet, key elements and templates that use the keys.
    *
    * The new templates must be at the bottom of the stylesheet in order to override other
    * templates with the same mode="use_linked_result", so both the keys and the templates
    * will go down there.
    */
   function _add_xsl_key(result, name, xsl)
   {
      function xsl_addEl(tag, parent)
      {
         var el = add_namespace_el(tag, nsXSL, parent, null, xsl);

         var a = arguments;
         for (var i=3, stop=a.length; i<stop; i+=2)
            el.setAttribute(a[i-1], a[i]);

         return el;
      }

      var xpath_result = _get_result_xpath(result);

      // key element attributes:
      var uses = "@"+SFW.get_result_idname(result);
      var match = xpath_result + "/" + result.getAttribute("row-name");

      var docel = xsl.documentElement;

      // Add the key:
      var newkey = xsl_addEl("xsl:key",docel,
                         "name",name,
                         "match",match,
                         "use", uses);
      addText("\n",docel);

      // Add template to use key:
      var newtemp = xsl_addEl("xsl:template", docel,
                          "match", "*[@rndx][@xslkey='" + name + "']",
                          "mode", "use_linked_result");
      addText("\n",docel);

      xsl_addEl("xsl:param",newtemp,"name","id");
      xsl_addEl("xsl:param",newtemp,"name","link");

      var keypath = "key('" + name + "', $id)";

      var apply = xsl_addEl("xsl:apply-templates", newtemp,
                            "select", "$link",
                            "mode", "use_linked_rows");

      xsl_addEl("xsl:with-param", apply, "name","rows","select",keypath);
   }

   function _update_xsl_keys(doc, xsl)
   {
      if (!xsl)
         xsl = _get_xsl_document();

      var kresults = doc.selectNodes("/*/*[@rndx][@xslkey]");
      for (var i=0,stop=kresults.length; i<stop; ++i)
      {
         var result = kresults[i];
         var keyname = _generate_key_name(result);
         if (_lacks_xsl_key(keyname, xsl))
         {
            _add_xsl_key(result, keyname, xsl);
            result.setAttribute("xslkey", keyname);
         }
      }
   }

   function _remove_xsl_keys(doc)
   {
      var xsl = _get_xsl_document();
      var kresults = doc.selectNodes("/*/*[@rndx][starts-with(@xslkey,'" + key_prefix + "')]");
      var stop = kresults.length;
      for (var i=0; i<stop; ++i)
      {
         var result = kresults[i];
         var key = xsl.selectSingleNode("/*/xsl:key[@name='" + result.getAttribute("xslkey") + "']");
         if (key)
         {
            key.parentNode.removeChild(key);
            result.setAttribute("xslkey", "auto");
         }
      }
   }

   function _integrate_element(result, el)
   {
      var schema, field, fieldname, old;

      if ((schema=result.selectSingleNode("schema"))
          && (field=SFW.get_schema_idfield(schema))
          && (fieldname=field.getAttribute("name")))
      {
         var id=el.getAttribute(fieldname);
         var xpath="*[local-name()=../@row-name][@" + fieldname + "='" + id + "']";
         old = result.selectSingleNode(xpath);

         if (old)
            result.replaceChild(el,old);
         else
            result.appendChild(el);

         return true;
      }

      console.error("Function integrate_element failed to find a fieldname.");
      return false;
   }

})();
