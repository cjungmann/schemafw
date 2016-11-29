var SFW = { };

function init_SFW(callback)
{
   SFW.alert                = _alert;
   SFW.confirm              = _confirm;
   SFW.seek_ancestor_anchor = _seek_ancestor_anchor;
   SFW.seek_child_anchor    = _seek_child_anchor;
   SFW.derive               = _derive;
   SFW.add_event            = _add_event;
   SFW.setup_event_handling = _setup_event_handling;
   SFW.process_event        = _process_event;
   SFW.translate_url        = _translate_url;
   SFW.open_interaction     = _open_interaction;
   SFW.get_director         = _get_director;
   SFW.alert_notice         = _alert_notice;
   SFW.check_for_preempt    = _check_for_preempt;
   SFW.base                 = _base;

   SFW.views                = [];
   
   SFW.types                = {};

   SFW.px                   = _px;

   function _px(num)    { return String(num)+"px"; };

   function call_SFW_init_functions()
   {
      // Other inits need the above functions, so only call after they're installed:
        var inits = ["init_SFW_Debug",
                     "init_SFW_DOM",
                     "init_SFW_Tables",
                     "init_SFW_Forms",
                     "init_SFW_Views"
                    ];
      
      // var inits = ["init_SFW_DOM", "init_SFW_Tables"];
      for (f in inits)
      {
         var s = inits[f];
         if (s in window)
            window[s]();
      }
   }

   function _alert(str)
   {
      window.alert(str);
   }

   function _confirm(str)
   {
      return window.confirm(str);
   }

   function _seek_ancestor_anchor(t)
   {
      var el = SFW.get_ancestor_anchor(t);
      if (el && el.sfwobj)
         return el;
      else
         return null;
   }

   function _seek_child_anchor(t)
   {
      function f(n) { return n.nodeType==1 && n.getAttribute("data-sfw-class"); }
      return SFW.find_child_matches(t, f, true);
   }

   // Add all base prototypes to dclass, which then can replace them:
   function _derive(dclass, base)
   {
      var pro_b = base.prototype;
      var pro_d = dclass.prototype;
      dclass._baseproto = pro_b;
      for (var p in pro_b)
      {
         if (!(p in pro_d))
            pro_d[p] = pro_b[p];
      }
   }

   function _add_event(name, f, el)
   {
      var target = el || document.body;
      if ("addEventListener" in target)
         target.addEventListener(name,f,true);
      else if ((target=el||document) && "attachEvent" in target)
         target.attachEvent("on"+name,f);
   }

   function _resize_table_heads()
   {
      var nl, t;
      if ((nl=document.getElementsByTagName("table")))
      {
         for (var i=0,stop=nl.length; i<stop; ++i)
            if ((t=nl[i]).getAttribute("data-sfw-class"))
               SFW.fix_table_heads(t);
      }
   }

   function _setup_event_handling()
   {
      function f(ev)
      {
         var e=ev||window.event;
         var t=e.target||e.srcElement;

         if (e.type=="resize")
         {
            _resize_table_heads();
            return true;
         }

         if (!process_dpicker(e,t))
            return false;
         if (!Moveable.process_event(e,t))
            return false;
         if (!SFW.process_event(e,t))
            return false;
         // if (!SchemaFW.process_button(e,t))
         //    return false;

         // I don't like this solution (simply moving to the end),
         // especially since the only thing custom_handler is doing
         // is handling the ESC key.  It should have a better name,
         // and there should be a preempt custom handler as well as
         // a handler for after SchemaFW.process_event().
         //
         // The problem is that this is a bigger design decision:
         // should _new_context() be changed into an object?  How does
         // the context object integrate with the rest of event handling?
         // How do contexts stack?  Too many questions need answers for
         // me to spend the time now when moving this code fixes the
         // problem of closing the context when the situation only calls
         // for closing a dialog.
         if ("custom_handler" in window && window.custom_handler)
         {
            if (!window.custom_handler(e,t))
               return false;
         }
         return true;
      }

      // ADD EVENTS HERE:
      Events.add_event("click",f);
      Events.add_event("mousedown",f);  // mousedown and mouseup for dragging dialog
      Events.add_event("mouseup",f);
      Events.add_event("keydown",f);
      Events.add_event("keyup",f);
      Events.add_event("keypress",f);
      Events.add_event("focus",f);

      window.onresize = f;
   }

   function _process_event(e,t)
   {
      var el,  obj;

      if ((el=_seek_ancestor_anchor(t)) && (obj=el.sfwobj))
         return obj.process(e,t);
      else
         return true;
   }

   function _translate_url(url, xmldoc)
   {
      var docel = xmldoc.documentElement;

      var refs = docel.selectNodes("*[@type='ref']");
      if (!refs || refs.length==0)
         return url;

      function cb(match)
      {
         var rnode, xpath = "*[@type='ref']/*[" + match + "]";
         if ((rnode=docel.selectSingleNode(xpath)))
         {
            var val = rnode.getAttribute(match.substring(1));
            return encodeURIComponent(val);
         }
         else
            return match;
      }

      url = url.replace(/@[a-z0-1_-]+/, cb);
      return url;
   }

   function _open_interaction(host, url, caller)
   {
      var newobj = null;
      
      function got(xdoc)
      {
         if (_check_for_preempt(xdoc))
         {
            var thost = addEl("div",host);
            thost.className = "SFW_Subhost";
            thost.style.height = SFW.px(host.offsetHeight);
            
            var xdocel = xdoc.documentElement;
            var type = xdoc.documentElement.getAttribute("mode-type");
            var xslo = SFW.xslobj;
            xslo.transformInsert(thost, xdocel);

            var anchor = SFW.seek_child_anchor(thost);
            if (anchor
                && type
                && type in SFW.types
                && (newobj = new SFW.types[type](anchor,xdoc,caller)))
            {
               ;
            }
            else
               host.removeChild(thost);
         }
      }

      xhr_get(url, got);
   }

   function _get_director(xmldoc, htmlanchor)
   {
      if (!htmlanchor)
         htmlanchor = document.sfwanchor;
      
      var docel, mtype;
      if (htmlanchor && xmldoc && (docel=xmldoc.documentElement))
      {
         if (!(mtype=docel.getAttribute("mode-type")))
            console.error("Untyped document");
         else if (!(mtype in SFW.types))
            console.error("SFW.types does not include " + mtype);
         else
            return new SFW.types[mtype](htmlanchor,xmldoc);
      }

      return null;
   }

   function _alert_notice(el)
   {
      var type = el.getAttribute("type");
      var msg = el.getAttribute("msg");
      var where = el.getAttribute("where");
      var detail = el.getAttribute("detail");
      var str = type + ":\n" + msg;
      if (where)
         str += "\nwhere: " + where;
      else if (detail)
         str += "\ndetail: " + detail;
      SFW.alert(str);
   }

   function _check_for_preempt(doc)
   {
      var docel = doc.documentElement;
      var tname = docel.tagName.toLowerCase();
      var jump;
      if (tname=="notice")
      {
         _alert_notice(docel);
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
         while(n)
         {
            if (n.nodeType==1 && (tname=n.tagName.toLowerCase())=="message")
            {
               _alert_notice(n);
               return false;
            }

            n = n.nextSibling;
         }
      }
      return true;
   }

   function _find_schema(doc)
   {
      var schema = doc.selectSingleNode("*/schema");
      if (!schema)
         schema = doc.selectSingleNode("*/*[@rndx=1]/schema");

      return schema;
   }

   function _find_subhost(el)
   {
      while(el && el.nodeType==1 && !class_includes(el,"SFW_Subhost"))
         el = el.parentNode;
      return el && el.nodeType==1 ? el : null;
   }
   
   function _base(html_base,xml_doc,caller)
   {
      this._top = html_base;
      this._host = _find_subhost(html_base);
      this._doc = xml_doc;
      this._caller = caller ? caller : null;
      this._schema = _find_schema(xml_doc);
      this._baseproto = null;
      this._super = null;
      html_base.sfwobj = this;
   };

   _base.prototype.top = function _top()       { return this._top; };
   _base.prototype.caller = function _caller() { return this._caller; };
   _base.prototype.stage = function _stage()   { return this._top.parentNode; };
   _base.prototype.doc = function _doc()       { return this._doc; };
   _base.prototype.schema = function _schema() { return this._schema; };
   _base.prototype.baseproto = function _baseproto() { return this._baseproto; };

   _base.prototype.button_processors = {};

   _base.prototype.close = function _close()
   {
      var v = this._host;
      if (v)
      {
         // This should release the closure:
         v.sfwobj = null;
         v.parentNode.removeChild(v);
      }
   };
   
   _base.prototype.confirm_owned = function _confirm_owned(el)
   {
      var anchor = SFW.get_ancestor_anchor(el);
      return (anchor && anchor==this._top);
   };

   _base.prototype.child_finished = function(child, cmd)
   {
      function f() { child.close(); }
      window.setTimeout(f, 100);
   };
    
  _base.prototype.process_clicked_button = function _process_clicked_button(b, cb)
   {
      var type = b.getAttribute("data-type");
      var url = b.getAttribute("data-task") || b.getAttribute("data-url");
      var cmsg = b.getAttribute("data-confirm");

      if (cmsg && !SFW.confirm(cmsg))
      {
         cb("cancel");
         return false;
      }

      switch(type)
      {
         case "jump":
         case "open":
            if (url)
            {
               window.location = url;
               return false;
            }
            break;
         case "cancel":
         case "close":
            if (this._caller)
               this._caller.child_finished(this,type);
            return false;
         
         default:
            var pbtype = "process_button_"+type;
            if (pbtype in this)
               return this[pbtype](b,cb);
            else if (url)
            {
               url = _translate_url(url, this._doc);
               _open_interaction(this._top.parentNode, url, this);
            }
            break;
      }

      return true;
   };

   _base.prototype.process = function _base_process(e,t)
   {
      return true;
   };

   // _base prototypes must be in place before this line:
   call_SFW_init_functions();

   function xmldocs_ready()
   {
      // These may change if I redo XML.js.  They should all
      // accept and use a callback in case the documents are
      // not already resident.
      SFW.xmldoc = getXMLDocument();
      if (SFW.xmldoc)
      {
         SFW.xsldoc = getXSLDocument();
         if (SFW.xsldoc)
            SFW.xslobj = new XSL(SFW.xsldoc);
      }

      callback();
   }

   getXMLDocs(xmldocs_ready);

}
