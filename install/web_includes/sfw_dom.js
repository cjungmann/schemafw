function init_SFW_DOM()
{
   set_missing_doc_props();
   
   // SFW.get_page_xoffset() AND
   // SFW.get_page_yoffset()
   prepare_page_offset_funcs();
   SFW.get_doc_offset       = _get_doc_offset;
   SFW.get_relative_offset  = _get_relative_offset;
   
   SFW.el_name              = _el_name;
   SFW.find_child_matches   = _find_child_matches;
   SFW.first_child_element  = _first_child_element;
   SFW.get_page_anchor      = _get_page_anchor;
   SFW.get_ancestor_anchor  = _get_ancestor_anchor;
   SFW.next_sibling_element = _next_sibling_element;

   // Sets SFW.get_page_xoffset and SFW.get_page_yoffset
   function prepare_page_offset_funcs()
   {
      if ('pageXOffset' in window)
      {
         SFW.get_page_xoffset = function() { return window.pageXOffset; };
         SFW.get_page_yoffset = function() { return window.pageYOffset; };
      }
      else if ('body' in document && 'scrollLeft' in document.body)
      {
         SFW.get_page_xoffset = function() { return document.body.scrollLeft; };
         SFW.get_page_yoffset = function() { return document.body.scrollTop;  };
      }
      else
      {
         SFW.get_page_xoffset = void(0);
         SFW.get_page_yoffset = void(0);
      }
   };

   function _get_doc_offset(el)
   {
      if (el===null)
         return {left:0, top:0};
      
      var os = el.offsetParent ? _get_doc_offset(el.offsetParent) : {left:0,top:0};
      return {left:os.left+el.offsetLeft, top:os.top+el.offsetTop };
   }

   function _get_relative_offset(target, ref)
   {
      var ost = _get_doc_offset(target);
      var osr = _get_doc_offset(ref);
      ost.left -= osr.left;
      ost.top -= ost.top;
      return ost;
   }

   function _el_name(el)
   {
      if ("localName" in el)
         _el_name = function(e) { return e.localName; };
      else if ("baseName" in el)
         _el_name = function(e) { return e.baseName; };
      else
         _el_name = void(0);

      SFW.el_name = _el_name;
      return _el_name(el);
   }

   function _find_child_matches(parent, tag_or_func, first_only, recursive)
   {
      var compfunc;
      if (typeof(tag_or_func)=="function")
         compfunc = tag_or_func;
      else
         compfunc = function(el) {
            return el.nodeType==1 && el.tagName.toLowerCase()==tag_or_func;
         };
      
      var rval = [];
      var node = parent.firstChild;
      while (node)
      {
         if (compfunc(node))
         {
            if (first_only)
               return node;
            else
               rval.push(node);
         }
         else if (recursive)
         {
            var recur =_find_child_matches(node, compfunc, first_only, true);
            if (recur)
            {
               if (first_only)
                  return recur;
               else
                  rval.concat(recur);
            }
         }
         node = node.nextSibling;
      }
      return rval.length ? rval : null;
   }

   function _first_child_element(el)
   {
      var n = el ? el.firstChild : null;
      if (n)
      {
         if (n.nodeType==1)
            return n;
         else
            return _next_sibling_element(n);
      }
      else
         return null;
   }

   function _next_sibling_element(el)
   {
      if ("nextElementSibling" in el)
         _next_sibling_element = function(e) { return e.nextElementSibling; };
      else
      {
         _next_sibling_element = function(e)
         {
            var n = e.nextSibling;
            while (n)
            {
               if (n.nodeType==1)
                  return n;
               else
                  n = n.nextSibling;
            }
            return null;
         };
      }

      // Also reset the member function, now that the function has been reset:
      SFW.next_sibling_element = _next_sibling_element;
      return _next_sibling_element(el);
   }

   function _get_page_anchor(levels, parent)
   {
      if (arguments.length==0)
         levels = 2;

      if (levels==0)
         return null;

      if (!parent)
         parent=document.getElementById("SFW_Content");
      
      var subel, el = _first_child_element(parent);
      while (el)
      {
         if (el.getAttribute("data-sfw-class"))
            return el;
         else if ((subel = _get_page_anchor(levels-1, el)))
            return subel;
         else
            el = _next_sibling_element(el);
      }

      return null;
   }

   function _get_ancestor_anchor(el)
   {
      while (el)
      {
         if (el.nodeType==1 && el.getAttribute("data-sfw-class"))
            return el;
         else if (el.nodeType==9)
            break;

         el = el.parentNode;
      }
      return null;
   }

   function set_missing_doc_props()
   {
      var html = document.documentElement;
      if (!("head" in document))
         document.head = _find_child_matches(html, "head", true);
      if (!("body" in document))
         document.body = _find_child_matches(html, "body", true);
      if (!("sfwanchor" in document))
      {
         var a = _get_page_anchor();
         if (a)
            document.sfwanchor = a;
      }
   }
}
