
// sfw_0.js

var SFW = { types     : {},
            autoloads : {},
            callback  : null,
            alert : function(str)
            {
               window.alert(str);
            },
            add_type : function(name, constructor, allow_replace)
            {
               if (name in this.types && !allow_replace)
               {
                  SFW.alert("I-class \"" + name + "\" has already been defined.");
                  return false;
               }
               this.types[name] = constructor;
               return true;
            },
            check_and_callback : function(name)
            {
               this.autoloads[name] = true;
               
               for (a in this.autoloads)
                  if (this.autoloads[a]==false)
                     return;
               // Callback after return to allow completion of last _init():
               setTimeout(this.callback, 125);
            },
            delay_init : function(name, callback, prereq)
            {
               if ("base" in SFW && (!prereq || prereq in SFW.types))
               {
                  console.log("Loading " + name);
                  this.check_and_callback(name);
                  return false;
               }
               else
               {
                  // Save name only once
                  if (!(name in this.autoloads))
                     this.autoloads[name] = false;
                  
                  console.log("Waiting to load " + name);
                  setTimeout(callback, 125);
                  return true;
               }
            }
          };

function init_SFW(callback)
{
   SFW.callback             = callback;
   
   SFW.confirm              = _confirm;
   SFW.seek_page_anchor     = _seek_page_anchor;
   SFW.seek_event_host      = _seek_event_host;
   SFW.seek_child_anchor    = _seek_child_anchor;
   SFW.derive               = _derive;
   SFW.add_event            = _add_event;
   SFW.setup_event_handling = _setup_event_handling;
   SFW.process_event        = _process_event;
   SFW.resize_page          = _resize_page;
   SFW.translate_url        = _translate_url;
   SFW.open_interaction     = _open_interaction;
   SFW.get_director         = _get_director;
   SFW.alert_notice         = _alert_notice;
   SFW.check_for_preempt    = _check_for_preempt;
   SFW.base                 = _base;  // "base class" for _form, _table, etc.

   SFW.stage                = document.getElementById("SFW_Content");

   SFW.px                   = _px;

   function _px(num)    { return String(num)+"px"; };

   function _confirm(str)
   {
      return window.confirm(str);
   }

   function _is_anchor(n)
   {
      return n.nodeType==1 && null!=n.getAttribute("data-sfw-class");
   }

   function _find_anchor(el)
   {
      return SFW.find_child_matches(el, _is_anchor, true, true);
   }

   function _seek_page_anchor(levels, parent)
   {
      if (arguments.length==0)
         levels = 2;

      if (levels==0)
         return null;

      if (!parent)
         parent=document.getElementById("SFW_Content");
      
      var subel, el = SFW.first_child_element(parent);
      while (el)
      {
         if (_is_anchor(el))
            return el;
         else if ((subel = _seek_page_anchor(levels-1, el)))
            return subel;
         else
            el = SFW.next_sibling_element(el);
      }

      return null;
   }

   function _seek_event_host(t)
   {
      
      while (t)
      {
         if (_is_anchor(t))
         {
            if ("sfwobj" in t.parentNode)
               return t.parentNode.sfwobj;
            break;
         }
         t = t.parentNode;
      }
      return null;
   }

   function _seek_child_anchor(t)
   {
      return SFW.find_child_matches(t, _is_anchor, true);
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

   function _resize_page_head()
   {
      var ghost, head = document.getElementById("SFW_Header");
      if (head)
      {
         if (!("ghost_div" in head))
         {
            ghost = addEl(head.tagName.toLowerCase(), head.parentNode, head);
            if (ghost)
            {
               head.ghost_div = ghost;
               ghost.className = "SFW_Ghost";
            }
         }
         else
            ghost = head.ghost_div;
         
         ghost.style.height = _px(head.offsetHeight);
      }
   }

   function _resize_table_heads()
   {
      if ("fix_table_heads" in SFW)
      {
         var nl, t;
         if ((nl=document.getElementsByTagName("table")))
         {
            for (var i=0,stop=nl.length; i<stop; ++i)
               if ((t=nl[i]).getAttribute("data-sfw-class"))
                  SFW.fix_table_heads(t);
         }
      }
   }

   function _resize_hosts()
   {
      function f(n) {
         if (n.nodeType==1)
         {
            if (n.tagName.toLowerCase()=="div")
            {
               if (class_includes(n,"SFW_Host"))
                  return true;
            }
         }

         return false;
         
      }
      var nl = SFW.find_child_matches(document.body,f,false,true);
      debugger;
      if (nl)
      {
         var os = SFW.get_doc_offset(document.body);
         for (var i=0,stop=nl.length; i<stop; ++i)
         {
         }
      }
   }

   function _resize_page()
   {
      _resize_page_head();
      _resize_hosts();
      _resize_table_heads();
   }

   function _setup_event_handling()
   {
      function f(ev)
      {
         var e=ev||window.event;
         var t=e.target||e.srcElement;

         if (e.type=="resize")
         {
            _resize_page();
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
      var obj;

      if ((obj=_seek_event_host(t)))
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

   function _arrange_in_host(host, anchor)
   {
      var os_host = SFW.get_doc_offset(host);
      var center_host = host.offsetWidth / 2 + os_host.left;
      
      var s = anchor.style;
      s.top = _px(os_host.top);
      s.left = _px(center_host - anchor.offsetWidth/2);
      s.zIndex = 100;
   }

   function _open_interaction(host, url, caller, data)
   {
      var newobj = null;
      
      function got(xdoc)
      {
         if (_check_for_preempt(xdoc))
         {
            var thost = addEl("div",host);
            thost.className = "SFW_Host";
            thost.style.minHeight = SFW.px(host.offsetHeight);
            
            var xdocel = xdoc.documentElement;
            var type = xdoc.documentElement.getAttribute("mode-type");
            var xslo = SFW.xslobj;
            xslo.transformInsert(thost, xdocel);

            var anchor = SFW.seek_child_anchor(thost);
            if (anchor
                && type
                && type in SFW.types
                && (newobj = new SFW.types[type](thost,xdoc,caller,data)))
            {
               _arrange_in_host(host, anchor);
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

   function _base(host,xml_doc,caller,data)
   {
      if (!host)
      {
         host = addEl("div", SFW.stage);
         host.className = "SFW_Host";
      }
      
      host.sfwobj = this;
      
      this._baseproto = null;
      this._caller = caller ? caller : null;
      this._host = host;
      this._xmldoc = xml_doc;

      if (data)
         this.data = data;
   };

   _base.prototype.top = function _top()       { return _find_anchor(this._host); };
   _base.prototype.schema = function _schema() { return _find_schema(this._xmldoc.documentElement); };
   _base.prototype.baseproto = function _baseproto() { return this._baseproto; };

   _base.prototype.button_processors = {};

   _base.prototype.sfw_close = function _sfw_close()
   {
      var v = this._host;
      if (v)
      {
         // This should release the closure:
         v.sfwobj = null;
         v.parentNode.removeChild(v);
      }
   };
   
   _base.prototype.child_finished = function(child, cmd)
   {
      function f() {
         if ("sfw_close" in child)
            child.sfw_close();
      }
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
               url = _translate_url(url, this._xmldoc);
               _open_interaction(this.top().parentNode, url, this);
            }
            break;
      }

      return true;
   };

   _base.prototype.process = function _base_process(e,t)
   {
      return true;
   };


   function call_SFW_init_functions()
   {
      // Other inits need the above functions, so only call after they're installed:
        var inits = ["init_SFW_Debug",
                     "init_SFW_DOM"
                    ];
      
      for (f in inits)
      {
         var s = inits[f];
         if (s in window)
            window[s]();
      }
   }

   // _base prototypes must be in place before this line:
   call_SFW_init_functions();
   _resize_page_head();

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
