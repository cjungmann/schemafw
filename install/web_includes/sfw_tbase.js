
// sfw_tbase.js

(function _init()
{
   if (SFW.delay_init("sfw_tbase",_init,"iclass"))
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
         var xpath = "/*/*[@rndx][@row-name='" + tag + "']";
         result = this.xmldoc().selectSingleNode(xpath);
      }
      return result;
   };

   // This method should be overridden, but here's an implementation just in case.
   _tbase.prototype.result = function(match)
   {
      return this.result_from_match(match);
   };

   _tbase.prototype.find_matching_data_row = function(cfobj)
   {
      var el =
         ("cdata" in cfobj && "xrow" in cfobj.cdata)
         ? cfobj.cdata.xrow
         : (("rowone" in cfobj)
            ? cfobj.rowone
            : null);

      if (el)
      {
         var id, res, xpath = "*/*[@rndx and @row-name='" + el.tagName + "']";
         res = this.xmldoc().selectSingleNode(xpath);
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

   _tbase.prototype.update_row = function(cfobj, preserve_result)
   {
      var xrow = this.find_matching_data_row(cfobj);

      if ("docel" in cfobj)
      {
         if (cfobj.mtype=="delete")
         {
            if (cfobj.confirm_delete() && xrow)
               this.delete_row(xrow);
         }
         else if (cfobj.rtype=="update")
         {
            var r_result = cfobj.result;
            var r_row = SFW.first_child_element(r_result);

            if (r_row)
            {
               // If target name, get the named result
               var target = r_result.getAttribute("target");
               if (target)
                  target = this.xmldocel().selectSingleNode(target+"[@rndx]");
               // If target name or named target not available, get alternative:
               if (!target && xrow)
                  target = xrow.parentNode;

               if (target)
               {
                  if (preserve_result)
                     r_row = _get_copied_node(target,r_row);

                  target.insertBefore(r_row, xrow);

                  if (xrow)
                     target.removeChild(xrow);
               }
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
