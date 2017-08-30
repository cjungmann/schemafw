
// sfw_tbase.js

(function _init()
{
   if ((!("SFW" in window) && setTimeout(_init,100))
       || SFW.delay_init("sfw_tbase",_init,"iclass"))
      return;

   if (!SFW.derive(_tbase, "tbase", "iclass"))
      return;

   function _tbase(actors)
   {
      SFW.base.call(this,actors);
   }

   _tbase.prototype.result_from_match = function(match)
   {
      var result = null;
      if (match)
      {
         var tag = ("tagName" in match) ? match.tagName : match;
         var xpath = "*[@rndx][@row-name='" + tag + "']";
         result = this.xmldocel().selectSingleNode(xpath);
      }
      return result;
   };

   _tbase.prototype.result = function(match)
   {
      if (match)
         return this.result_from_match(match);
      else
      {
         var xpath = this.get_result_path()
            || this.get_result_xpath_from_top()
            || "/*/*[@rndx=1][not(@merged)]";
         
         return this.xmldoc().selectSingleNode(xpath);
      }
   };

   _tbase.prototype.find_matching_data_row = function(cfobj)
   {
      var el =
         ("cdata" in cfobj && "xrow" in cfobj.cdata)
         ? cfobj.cdata.xrow
         : cfobj.update_row;

      if (el)
      {
         var id, res, xpath = "*[@rndx and @row-name='" + el.tagName + "']";
         res = this.xmldocel().selectSingleNode(xpath);
         id = el.getAttribute("id");
         if ((res=this.result(el)) && (id=el.getAttribute("id")))
         {
            function f(n) { return n.nodeType==1 && n.getAttribute("id")==id; }
            return SFW.find_child_matches(res, f, true, false);
         }
      }

      return null;
   };

   _tbase.prototype.replot = function(result)
   {
      console.error("This function should be overridden!");
   };

   _tbase.prototype.empty_node = function(node)
   {
      // Remove attributes separately, since they are not included in the child list:
      // Iterate in reverse or removing each attribute will change the position of
      // the remainder of the attributes.
      for (var s=node.attributes,i=s.length-1; i>=0; --i)
         node.removeAttribute(s[i].nodeName);
      
      var n, a = node.firstChild;
      while ((n=a))
      {
         a = a.nextSibling;
         node.removeChild(n);
      }
   };

   function _get_copied_node(target, source)
   {
      var doc = target.ownerDocument;
      function ca(target,source)
      {
         // Copy attribtes separately, since they are not included in the child list:
         for (var s,i=0,sa=source.attributes,stop=sa.length; i<stop && (s=sa[i]); ++i)
            target.setAttribute(s.nodeName,s.nodeValue); 
      }

      function cp(target,source)
      {
         ca(target,source);

         var node, next = source.firstChild;
         while((node=next))
         {
            next = node.nextSibling;
            switch(node.nodeType)
            {
               case 1:  // element
                  cp(doc.createElement(node.tagName),node);
                  break;
               // no case 2: ca() copies attributes
               case 3:  // text
                  target.appendChild(doc.createTextNode(node.nodeValue));
                  break;
               case 5: // EntityReference
                  target.appendChild(doc.createEntityReference(n.nodeName));
                  break;
               default: break;
            }
         }
      }

      var newnode = doc.createElement(source.tagName);

      cp(newnode, source);
      return newnode;
   }
   
   _tbase.prototype.delete_row = function(xrow)
   {
      if (this.result(xrow) && xrow)
         xrow.parentNode.removeChild(xrow);
   };

   _tbase.prototype.get_result_to_update = function(cfobj)
   {
      var result = null;
      var tname = cfobj.target_name;
      if (tname)
         result = this.xmldocel().selectSingleNode(tname+"[@rndx]");
      return result;
   };

   _tbase.prototype.update_row = function(cfobj, preserve_result)
   {
      var cdata = "cdata" in cfobj ? cfobj.cdata : null;
      var xrow = (cdata && "xrow" in cdata) ? cdata.xrow : null;
      var urow = ("update_row" in cfobj) ? cfobj.update_row : null;

      if ("docel" in cfobj)
      {
         if (cfobj.mtype=="delete")
         {
            if (cfobj.confirm_delete() && xrow)
               this.delete_row(xrow);
         }
         else if (cfobj.rtype=="update" && urow)
         {
            var target = this.get_result_to_update(cfobj);
            if (!target)
            {
               if (xrow)
                  target = xrow.parentNode;
               else
                  target = this.xmldocel().selectSingleNode("*[@rndx=1][1]");
            }

            if (target)
            {
               if (preserve_result || target.ownerDocument!=urow.ownerDocument)
                  urow = _get_copied_node(target,urow);

               target.insertBefore(urow, xrow);

               if (xrow)
                  target.removeChild(xrow);
            }
         }
      }
      else if ("cmd" in cfobj)
      {
         switch(cfobj.cmd)
         {
            case "delete": this.delete_row(cfobj.cdata.xrow); break;
            case "fail":
               SFW.alert(cfobj.rtype + " operation failed.");
               return;   // skip replot() at end of function
         }
      }
   };

   
})();
