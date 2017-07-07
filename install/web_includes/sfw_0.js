
// sfw_0.js

var SFW = { types     : {},
            autoloads : {},
            callback  : null,
            continuing_autoloads : function()
            {
               for (a in this.autoloads)
                  if (this.autoloads[a]==false)
                     return true;
               return false;
            },
            check_and_callback : function(name)
            {
               this.autoloads[name] = true;

               if (SFW.continuing_autoloads())
                  return;
               
               // Callback after return to allow completion of last _init():
               setTimeout(this.callback, 125);
            },
            delay_init : function(name, callback, prereq)
            {
               if ("base" in SFW && (!prereq || prereq in SFW.types))
               {
                  // console.log("Loading " + name);
                  this.check_and_callback(name);
                  return false;
               }
               else
               {
                  if (!(name in this.autoloads))
                     this.autoloads[name] = false;
                  
                  // console.log("Waiting to load " + name);
                  setTimeout(callback, 125);
                  return true;
               }
            }
          };

function init_SFW(callback)
{
   SFW.callback             = callback;

   SFW.alert                = _alert;
   SFW.confirm              = _confirm;
   SFW.log_error            = _log_error;
   SFW.confirm_not_null     = _confirm_not_null;
   SFW.seek_top_sfw_host    = _seek_top_sfw_host;
   SFW.seek_page_anchor     = _seek_page_anchor;
   SFW.seek_child_anchor    = _seek_child_anchor;
   SFW.seek_event_actors    = _seek_event_actors;
   SFW.seek_event_object    = _seek_event_object;
   SFW.derive               = _derive;
   SFW.document_object      = _document_object;
   SFW.add_event            = _add_event;
   SFW.setup_event_handling = _setup_event_handling;

   SFW.keycode_from_event   = _keycode_from_event;

   SFW.set_view_renderer    = _set_view_renderer;
   SFW.update_selected_view = _update_selected_view;
   SFW.change_view          = _change_view;
   SFW.process_event        = _process_event;

   SFW.setup_sfw_host        = _setup_sfw_host;
   SFW.get_last_SFW_Host    = _get_last_SFW_Host;
   SFW.arrange_in_host      = _arrange_in_host;
   SFW.resize_page          = _resize_page;
   SFW.translate_url        = _translate_url;
   SFW.apply_row_context    = _apply_row_context;
   SFW.render_interaction   = _render_interaction;
   SFW.open_interaction     = _open_interaction;
   SFW.get_director         = _get_director;
   SFW.alert_notice         = _alert_notice;
   SFW.check_for_preempt    = _check_for_preempt;
   SFW.update_location_arg  = _update_location_arg;
   SFW.base                 = _base;  // "base class" for _form, _table, etc.
   SFW.types["iclass"]      = _base;

   SFW.show_string_in_pre   = _show_string_in_pre;

   SFW.get_object_from_host = _get_object_from_host;
   SFW.get_cfobj_result     = _get_cfobj_result;

   SFW.stage                = document.getElementById("SFW_Content");
   SFW.px                   = _px;
   SFW.addXMLEl             = _addXMLEl;

   function _px(num)    { return String(num)+"px"; };

   // Alternative to XML.js::addEl() to create elements without XHTML namespace:
   function _addXMLEl(tag,parent,before,ns)
   {
      // The following function is from XML.js (in case of error):
      return add_namespace_el(tag,ns||"",parent,before);
   }

   function _alert(str)
   {
      window.alert(str);
   }

   function _confirm(str)
   {
      return window.confirm(str);
   }

   function _log_error(msg)
   {
      console.error(msg);
      if (SFW.debugging)
         debugger;
   }

   function _show_string_in_pre(str)
   {
      var div = addEl("pre", document.body);
      // var div = document.body.appendChild(document.createElement("div"));
      div.appendChild(document.createTextNode(str));
   }

   function _confirm_not_null(obj, msg)
   {
      if (obj==null)
      {
         _log_error(msg);
         return false;
      }
      return true;
   }

   function _is_anchor(n)
   {
      return n.nodeType==1 && n.getAttribute("data-sfw-class");
   }

   function _find_anchor(el)
   {
      return SFW.find_child_matches(el, _is_anchor, true, true);
   }

   function _seek_top_sfw_host()
   {
      function f(node) { return node.nodeType==1 && node.className=="SFW_Host"; }
      return SFW.find_child_matches(document, f, true, true);
   }

   function _seek_page_anchor(levels, parent)
   {
      if (arguments.length==0)
         levels = 2;

      if (levels==0)
         return null;

      if (!parent)
         parent=SFW.stage;
      
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

   function _seek_event_actors(t)
   {
      var aval, rval = {};
      var is_control = false;
      var fcount = 0;
      while (t && t.nodeType < 9)
      {
         if (t.nodeType==1)
         {
            if ((aval=t.getAttribute("data-sfw-class")))
            {
               if (!is_control)
               {
                  rval["iclass"] = aval;
                  rval["anchor"] = t;

                  is_control=t.getAttribute("data-sfw-input");
                  if (is_control)
                     rval["input"] = t;

                  ++fcount;
               }
            }
            else if (t.className=="SFW_Host")
            {
               if (t.getAttribute("data-subview"))
                  rval["subhost"] = t;
               else
               {
                  rval["host"] = t;
                  ++fcount;
                  break;
               }
            }
         }
         t = t.parentNode;
      }
      return fcount>1 ? rval : null;
   }

   function _seek_event_object(t)
   {
      var actors = _seek_event_actors(t);
      if (actors && actors.iclass in SFW.types)
         return new SFW.types[actors.iclass](actors);
      return null;
   }

   function _seek_child_anchor(t)
   {
      return SFW.find_child_matches(t, _is_anchor, true);
   }

   function _base_class_exists(name)
   {
      if (!(name in SFW.types))
      {
         _alert("Base class \"" + name + "\" is undefined.");
         return false;
      }
      return true;
   }

   function _new_type_unique(name)
   {
      if (name in SFW.types)
      {
         _alert("IClass \"" + name + "\" is already defined.");
         return false;
      }
      return true;
   }

   /**
    * Registers new_class_name to SFW.types map with pointer to dclass constructor.
    *
    * @param dclass          constructor of object
    * @param new_class_name  name by which the constructor is accessed
    * @param base_class_name previously registered object from which the new
    *                        class is derived.
    * @param allow_replace   flag to allow new class to replace an existing
    *                        class with the same class_name.
    *
    * @return false if base_class does not exist
    *               OR if the new class name is already registered
    *                  AND allow_replace is false/undefined
    *         true  for success registering name and copying prototype members.
    */
   function _derive(dclass, new_class_name, base_class_name, allow_replace)
   {
      if (!_base_class_exists(base_class_name))
         return false;
      if (!allow_replace && !_new_type_unique(new_class_name))
         return false;

      this.types[new_class_name] = dclass;

      var pro_b = this.types[base_class_name].prototype;
      var pro_d = dclass.prototype;
      pro_d.class_name = new_class_name;
      pro_d._baseproto = pro_b;
      for (var p in pro_b)
      {
         if (p!="_baseproto" && !(p in pro_d))
            pro_d[p] = pro_b[p];
      }
      return true;
   }

   function _add_event(name, f, el)
   {
      var target = el || document.body;
      if ("addEventListener" in target)
         target.addEventListener(name,f,true);
      else if ((target=el||document) && "attachEvent" in target)
         target.attachEvent("on"+name,f);
   }

   function _manage_content_spacer(el)
   {
      var ghost;
      if (el && !class_includes(el,"ghost"))
      {
         if (!("ghost_el" in el))
         {
            var next_el = SFW.next_sibling_element(el);
            ghost = addEl(el.tagName.toLowerCase(), el.parentNode, next_el);
            if (ghost)
            {
               ghost.className = el.className;
               class_add(ghost,"ghost");
               
               el.ghost_el = ghost;
            }
         }
         else
            ghost = el.ghost_el;
         
         el.style.position = "fixed";
         el.style.width = "100%";
         ghost.style.height = _px(el.offsetHeight);
      }
   }

   function _get_last_SFW_Host()
   {
      var n = SFW.stage.lastChild;
      while(n)
      {
         if (n.nodeType==1 && class_includes(n,"SFW_Host"))
            return n;
         n = n.previousSibling;
      }
      return null;
   }

   function _resize_host_titles()
   {
      function f(n) {
         if (n.nodeType==1 && class_includes(n,"sfw_title"))
            _manage_content_spacer(n);
      }

      function g(n) {
         if (n.nodeType==1 && class_includes(n,"SFW_Host"))
            SFW.find_child_matches(n,f,false,true);
      }

      SFW.find_child_matches(SFW.stage,g,false,true);
   }

   function _resize_page_head()
   {
      _manage_content_spacer(document.getElementById("SFW_Header"));
      _resize_host_titles();
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
               {
                  return true;
               }
            }
         }
         return false;
      }
      var nl = SFW.find_child_matches(document.body,f,false,true);
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

   function _keycode_from_event(e)
   {
      if ("keyCode" in e)
         SFW.keycode_from_event = function(e) { return e.keyCode; };
      else if ("which" in e)
         SFW.keycode_from_event = function(e) { return e.which; };
      else if ("charCode" in e)
         SFW.keycode_from_event = function(e) { return e.charCode; };

      return SFW.keycode_from_event(e);
   }

   function _show_view_content(viewlist)
   {
      function istr(level)
      {
         var str = "";
         for (var i=0; i<level; ++i)
            str += "  ";
         return str;
      }
      
      var arr = [];
      function show_attr(attr, level)
      {
         arr.push(istr(level) + attr.name + "=\"" + attr.value + "\"");
      }
      
      function show_el(el, level)
      {
         arr.push(istr(level) + "<" + el.localName);
         var attrs = el.attributes;
         if (attrs.length)
         {
            for (var i=0,stop=attrs.length; i<stop; ++i)
               show_attr(attrs[i],level+3);
         }

         var child_count = 0;
         var c = el.firstChild;
         while (c)
         {
            if (c.nodeType==1)
            {
               if (!child_count)
                  arr.push(istr(level+1) + ">");
               ++child_count;
               
               show_el(c,level+1);
            }
            c = c.nextSibling;
         }

         if (child_count)
            arr.push(istr(level) + "<" + el.localName + ">");
         else
            arr.push(istr(level+1) + "/>");
      }

      show_el(viewlist,0);

      alert(arr.join("\n"));
   }

   function _get_view_renderer_element(xsldoc)
   {
      if (!("view_renderer" in xsldoc))
      {
         var arr = [ "/xsl:stylesheet",
                     "xsl:template[@match='*[@rndx]'][@mode='result_fill_sfw_host']",
                     "xsl:apply-templates[@select='.'][not(@mode)]" ];

         xsldoc.view_renderer = xsldoc.selectSingleNode(arr.join('/'));
      }

      return xsldoc.view_renderer;
   }

   function _set_view_renderer(view)
   {
      var el = _get_view_renderer_element(SFW.xsldoc);
      if (_confirm_not_null(el, "Unable to find view_renderer element."))
      {
         var mode = view.getAttribute("mode");
         if (mode)
            el.setAttribute("mode", mode);
         else if (el.getAttribute("mode"))
            el.removeAttribute("mode");
      }
   }

   function _update_selected_view(view)
   {
      var n = view.parentNode.firstChild;
      while(n)
      {
         if (n.nodeType==1)
         {
            if (n.getAttribute("selected"))
            {
               if (n==view)
                  break; // the only early-exit is if there is no view change
               else
                  n.removeAttribute("selected");
            }
            else if (n==view)
               n.setAttribute("selected", "true");
         }

         n = n.nextSibling;
      }
   }

   function _change_view(view_name)
   {
      var xpath = "/*/views/view[@name='" + view_name + "']";
      var view = SFW.xmldoc.selectSingleNode(xpath);
      if (view)
      {
         var host = _get_last_SFW_Host();
         if (host)
         {
            _set_view_renderer(view);
            _update_selected_view(view);

//            _show_view_content(view.parentNode);

            SFW.xslobj.transformFill(host,view);
         }
      }
   }

   function _process_event(e,t)
   {
      if (t.tagName.toLowerCase()=="img"
          && t.parentNode.tagName.toLowerCase()=="button")
         t = t.parentNode;

      // Preempt host search if clicking a view button
      if (class_includes(t,"view_selector"))
      {
         if (!class_includes(t,"selected"))
            _change_view(t.getAttribute("data-name"));

         return false;
      }

      var obj = _seek_event_object(t);
      if (obj)
         return obj.process(e,t);
      else
         return true;
   }

   function _translate_url(url, xmldocel)
   {
      var refs = xmldocel ? xmldocel.selectNodes("*[@type='ref']") : null;
      if (!refs || refs.length==0)
         return url;

      function cb(match)
      {
         var rnode, xpath = "*[@type='ref']/*[" + match + "]";
         if ((rnode=xmldocel.selectSingleNode(xpath)))
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

   var reRplRowVals = /\{\!([^\}]+)\}/;

   function _apply_row_context(url, row)
   {
      function rep(full, sub)
      {
         var aval = row.getAttribute(sub);
         if (aval)
            return aval;
         else
         {
            console.error("Unable to find attribute " + matches[1]);
            return "";
         }
      }

      return url.replace(reRplRowVals, rep);
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

   function _size_to_cover(over, under)
   {
      over.style.minHeight = SFW.px(under.offsetHeight);
   }

   function _setup_sfw_host(shost, xmldoc, caller, data)
   {
      shost.xmldoc = xmldoc;
      shost.caller = caller;
      shost.data = data;
   }

   function _make_sfw_host(host, xmldoc, caller, data)
   {
      var thost = addEl("div",host);
      thost.className = "SFW_Host";

      _setup_sfw_host(thost, xmldoc, caller, data);

      return thost;
   }

   function _find_insertbefore_able(doc)
   {
      function f(n)
      {
         return n.nodeType==1
            && (n.tagName.toLowerCase()=="schema" || n.getAttribute("rndx"));
      }
      return SFW.find_child_matches(doc.documentElement,f,true,false);
   }

   function _merge_into_pagedoc(pagedoc, newdoc)
   {
      var ibefore = _find_insertbefore_able(pagedoc);
      var docel = pagedoc.documentElement;
      function f(n)
      {
         if (n.nodeType==1)
         {
            var pl = make_importable_node(pagedoc, n, true);
            pl.setAttribute("merged","yes");
            docel.insertBefore(pl, ibefore);
         }
      }
      SFW.find_child_matches(newdoc.documentElement,f,false,false);
   }

   function _open_interaction(host, url, caller, data)
   {
      function got(xdoc)
      {
         var merge, pagedoc, node_to_remove;
         if (_check_for_preempt(xdoc))
         {
            if (xdoc.documentElement.getAttribute("mode-type")=="merge")
            {
               // Copy new stuff into page document so the new
               // stuff can access common data residing in the
               // page document (lookup results, etc):
               if ((pagedoc=caller.xmldoc()))
               {
                  _merge_into_pagedoc(pagedoc, xdoc);
                  xdoc = pagedoc;
               }
               else
                  console.error("Can't find caller document for xdoc merging.");
            }
            _render_interaction(xdoc, host, caller, data);
         }
      }

      console.log(url);

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

   function _update_location_arg(name, value)
   {
      var newarg = name + (value ? ("="+encodeURIComponent(value)):"");
      
      var loc = window.location;
      
      // strip leading '?' to avoid empty initial element
      var qsearch = ((loc.search.length) ? loc.search.substring(1) : "").split('?');

      var page = loc.pathname;
      var args_ndx = -1;
      if ("run_xdoc_loader" in window)
      {
         page += "?" + qsearch[0];
         if (qsearch.length>1)
            args_ndx = 1;
      }
      else if (qsearch.length==1)
         args_ndx = 0;

      var set=false, args = [];
      if (args_ndx>-1)
      {
         args = qsearch[args_ndx].split('&');
         if (args)
         {
            var aarr;
            for (var i=0,stop=args.length; !set && i<stop; ++i)
            {
               if ((aarr=args[i].split('=')) && aarr[0]==name)
               {
                  args[i] = newarg;
                  set = true;
               }
            }
         }
      }
      
      if (!set)
         args.push(newarg);

      return page + "?" + args.join('&');
   }

   var _re_func = /function\s*([^\(]+)\(([^\)]+)/;
   function _document_object(obj)
   {
      function funcargs(f)
      {
         var s = f.toString();
         var m = _re_func.exec(s);
         if (m)
            return m[2];
         else
            return "void";
      }
      
      arrF = [];
      arrO = [];
      arrS = [];
      arrX = [];
      
      var p = obj.prototype;
      for (prop in p)
      {
         switch(typeof(p[prop]))
         {
            case "function":
               arrF.push(prop + "(" + funcargs(p[prop]) + ")");
               break;
            case "object":
               arrO.push(prop);
               break;
            case "string":
               arrS.push(prop);
               break;
            default:
               arrX.push(prop + " (" + typeof(p[prop]) + ")");
               break;                             
         }
      }

      _alert( "Object " + ("class_name" in p ? p.class_name  : "") + "\n"
             + (arrF.length ? ("Functions\n   " + arrF.join("\n   ")) : "")
             + (arrO.length ? ("\nObjects\n   " + arrO.join("\n   ")) : "")
             + (arrS.length ? ("\nStrings\n   " + arrS.join("\n   ")) : "")
             + (arrX.length ? ("\nOther\n   "   + arrX.join("\n   ")) : ""));
   }

   function _get_object_from_host(host)
   {
      var anchor, type;
      if ((anchor=_seek_child_anchor(host))
          && (type=anchor.getAttribute("data-sfw-class"))
          && (type in SFW.types))
         return new SFW.types[type]({host:host});
      else
         return null;
   }

   function _get_cfobj_result(cfobj)
   {
      var cd, xrow, rowone;
      if ("cdata" in cfobj && (cd=cfobj.cdata) && "xrow" in cd)
         xrow = cd.xrow;
      if (cfobj.mtype!="delete" && "rowone" in cfobj)
         rowone = cfobj.rowone;

      function check(row)
      {
         var pn = row ? row.parentNode : null;
         if (pn && !pn.getAttribute("rndx"))
            pn = null;
         return pn;
      }

      return check(rowone) || check(xrow) || null;
   }

   function _base(actors)
   {
      this._host_el = actors.host;
      if ("input" in actors)
         this._input = actors.input;
   };

   _base.prototype.host     = function() { return this._host_el||null; };
   _base.prototype.xmldoc   = function() {
      var r=this.host(); return (r && (r=r.xmldoc))?r:null;
   };

   _base.prototype.xmldocel = function() {
      var r=this.xmldoc(); return r?r.documentElement:null;
   };

   _base.prototype.data     = function() { var r=this.host(); return r?r.data:null; };

   _base.prototype.class_name = "iclass";
   _base.prototype.top       = function() { return _find_anchor(this.host()); };
   _base.prototype.schema    = function() { return _find_schema(this.xmldocel()); };
   _base.prototype.baseproto = function() { return this._baseproto; };
   _base.prototype.button_processors = {};

   _base.prototype.input = function() { return ("_input" in this)?this._input:null;};
   _base.prototype.caller = function()
   {
      var h=this.host();
      return (h && "caller" in h) ? h.caller : null;
   };
   
   _base.prototype.setup = function(xmldoc, caller, data)
   {
      console.error("This function is obsolete.");
      var e = this._host_el;
      e._xmldoc = xmldoc;
      if (caller)
         e.caller = caller;
      if (data)
         e.data = data;
   };

   _base.prototype.get_input_field = function()
   {
      var schema = this.schema();
      if (schema && "input" in this)
      {
         var name = this.input().getAttribute("name");
         var xpath = "field[@name='" + name + "']";
         return schema.selectSingleNode(xpath);
      }
      else
         return null;
   };

   _base.prototype.get_field_name = function()
   {
      var field = this.get_input_field();
      return field ? field.getAttribute("name") : null;
   };

   _base.prototype.get_host_form_data_row = function()
   {
      var schema = this.schema();
      if (schema)
      {
         var xpath = "../*[local-name()=../@row-name]";
         return schema.selectSingleNode(xpath);
      }
      else
         return null;
   };

   /** Get value of data-{name} attribute from the top element. */
   _base.prototype.get_data_value = function(name)
   {
      var anchor, attr;
      if ((anchor=this.top()) && (attr=anchor.getAttribute("data-"+name)))
         return attr;
      else
         return null;
   };

   _base.prototype.sfw_close = function _sfw_close()
   {
      var v = this.host();
      if (v)
         v.parentNode.removeChild(v);
   };

   _base.prototype.sfw_hide = function _sfw_hide()
   {
      var v = this.host();
      if (v)
         v.style.display = "none";
   };

   function _child_close(child)
   {
      if ("sfw_close" in child)
         window.setTimeout(function(){child.sfw_close();}, 100);
   }

   function _child_hide(child)
   {
      if ("sfw_hide" in child)
         child.sfw_hide();
   }

   function _confirm_delete(rowone)
   {
      return rowone && ("0"!=rowone.getAttribute("deleted"));
   }

   _base.prototype.has_data = function() { return "data" in this.host(); };

   _base.prototype.cfobj_from_doc = function(doc)
   {
      var docel = doc.documentElement;
      var result = docel.selectSingleNode("*[@rndx=1]");
      
      var rval = { cfobj  : true,
                   child  : this,
                   docel  : docel,
                   mtype  : docel.getAttribute("mode-type"),

                   close  : function() { _child_close(this.child); },
                   hide   : function() { _child_hide(this.child); }
                 };
      
      if (this.has_data())
         rval.cdata = this.host().data;

      if (result)
      {
         var rname = result.getAttribute("row-name") || "row";
         var xpathrow = "*[local-name()='" + rname + "']";
         rval["result"] = result;
         rval["rtype"] = result.getAttribute("type") || null;
         rval["rname"] = rname;
         rval["rowone"] = result.selectSingleNode(xpathrow);
         rval["confirm_delete"] = function() { return _confirm_delete(this.rowone); };
      }

      return rval;
   };

   _base.prototype.cfobj_from_cmd = function(cmd)
   {
      var rval;
      if (typeof(cmd)=="object" && "documentElement" in cmd)
         rval =this.cfobj_from_doc(cmd);
      else
      {
         rval = { cfobj : true,
                  child : this,
                  cmd   : cmd || null,
                  close : function() { _child_close(this.child); },
                  hide  : function() { _child_hide(this.child); }
                };
         
         if (this.has_data())
            rval.cdata = this.host().data;
      }

      return rval;
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
         if (this.caller())
         {
            var child = this.cfobj_from_cmd(type);
            child.hide();
            this.caller().child_finished(child);
         }
            return false;
         
         default:
            // Gotta detect delete-type buttons and process without _open_interaction!

         
            var pbtype = "process_button_"+type;
            if (pbtype in this)
               return this[pbtype](b,cb);
            else if (url)
            {
               url = _translate_url(url, this.xmldocel());
               _open_interaction(this.top().parentNode, url, this);
            }
            break;
      }

      return true;
   };

   _base.prototype.pre_transform = function() { };
   _base.prototype.post_transform = function() { };
   _base.prototype.initialize = function() { };

   _base.prototype.clear_merged_elements = function()
   {
      var docel = this.xmldocel();
      function f(n)
      {
         if (n.nodeType==1 && n.getAttribute("merged")=="yes")
            docel.removeChild(n);
      }
      SFW.find_child_matches(docel,f,false,false);
      
   };

   _base.prototype.child_ready = function(child) { };

   _base.prototype.child_finished = function(cfobj)
   {
      this.clear_merged_elements();
      cfobj.close();
   };

   /** Buttons are all processed the same way, so handle consistently in base class. */
   _base.prototype.process = function _base_process(e,t)
   {
      if (e.type=="click")
      {
         function b(n) { return t.tagName.toLowerCase()=="button"?t:null; }
         var button = b(t) || b(t.parentNode);
         if (button)
            return this.process_clicked_button(button);
      }
      
      return true;
   };

   _base.prototype.call_super_event = function(itype, fname, args)
   {
      var proto = (itype in SFW.types) ? SFW.types[itype].prototype : null;
      if (proto && fname in proto)
         return proto[fname].apply(this, args);

      return true;
   };

   function _get_class_from_name(name)
   {
      if (name in SFW.types)
         return SFW.types[name];
      else
      {
         console.error("Failed to find registered class \"" + name + ".\"");
         return null;
      }
   }

   function _seek_current_view(doc)
   {
      var views = doc.documentElement.selectSingleNode("views");
      if (views)
      {
         var xpath = "view[@selected] | view[not(view[@selected])][1]";
         return views.selectSingleNode(xpath);
      }

      return null;
   }

   function _get_class_from_doc(doc)
   {
      var name, view;
      if ((name= doc.documentElement.getAttribute("mode-type")))
         return _get_class_from_name(name);
      else if ((view = _seek_current_view(doc)))
         return view.getAttribute("type");
      else
      {
         console.error("Missing mode-type attribute.");
         return null;
      }
   }

   function _render_interaction(doc,host,caller,data)
   {
      var lhost = _make_sfw_host(host,doc,caller,data);
      
      if (_confirm_not_null(lhost, "Failed to make a new SFW_Host."))
      {
         _size_to_cover(lhost, caller||host);

         var obj = null;
         var iclass = _get_class_from_doc(doc);
         if (iclass)
            obj = new iclass({host:lhost});

         if (obj)
            obj.pre_transform();
         
         SFW.xslobj.transformFill(lhost, doc.documentElement);

         // Uncomment for debugging problems with the stylesheet and document:
         // if (lhost.childNodes.length==0)
         // {
         //    _show_string_in_pre(serialize(doc));
         //    _show_string_in_pre(serialize(SFW.xslobj.doc));
         // }

         if (obj)
         {
            obj.post_transform();
            obj.initialize();

            if (caller)
               caller.child_ready(obj);
         }

         var anchor = SFW.seek_child_anchor(lhost);
         if (anchor)
            _arrange_in_host(host, anchor);
      }
   }

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
