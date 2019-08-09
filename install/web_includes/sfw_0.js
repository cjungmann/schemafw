
// sfw_0.js

if ("SFW" in window)
   window._saved_SFW = window.SFW;

var SFW = { types     : {},
            preloads  : {},      /**< Object to associate preload names with callback functions. */
            start_app : null,    /**< callback for after all preloads are completed. */
            xmldoc    : null,
            xsldoc    : null,
            confirm_class : function(name) { return name in this.types; },
            seek_preload : function(name)
            {
               return (name in this.preloads) ? this.preloads[name] : null;
            },
            preloads_complete : function()
            {
               for (var a in this.preloads)
                  return false;
               return true;
            },
            add_preload : function(name, f)
            {
               if (name in this.preloads)
                  console.err("\"" + name + "\" is already in use.  Request ignored.");
               else
                  this.preloads[name] = f;
            },
            delete_preload : function(name, follow_on_function)
            {
               if (name in this.preloads)
               {
                  delete this.preloads[name];

                  if (follow_on_function)
                     follow_on_function();

                  if (this.preloads_complete())
                     this.start_app();
               }
               else
                  console.err("\""+name+"\" not found for deletion.");
            },
            
            /** Checks prerequisites for loading the class, setting a delay if necessary.
             *
             * @param name   Class name of object being initialized
             * @param init   Initialization function to call when prerequisites fulfilled.
             * @param prereq Class from which the named class is derived.
             *
             * @return TRUE if a delay is necessary, FALSE if prerequisite unfulfilled
             *         or if SFW not ready to derive stuff.
             */
            delay_init : function(name, init, prereq)
            {
               if (!prereq)
                  return false;

               if (!("derive" in SFW))
               {
                  setTimeout(init, 100);
                  return true;
               }
               else if (!prereq || this.confirm_class(prereq))
                  return false;
               else
               {
                  var f = (name in this.preloads) ? this.preloads.name : null;
                  if (!f)
                  {
                     var ths = this;
                     f = function()
                     {
                        if (ths.confirm_class(prereq))
                        {
                           ths.delete_preload(name, init);
                           return false;
                        }
                        else
                        {
                           setTimeout(f,125);
                           return true;
                        }
                     };

                     this.add_preload(name, f);
                  }

                  if (f)
                     return f();
               }
               return false;
            },
            add_handler : function(h, first)
            {
               var attempt_count = 0;
               function ready_to_use() { return "remove_handler" in SFW; }
               function try_adding()
               {
                  ++attempt_count;
                  if (ready_to_use())
                  {
                     SFW.add_handler(h,first);
                     console.log("Adding " + h.name + " after " + attempt_count + " tries.");
                  }
                  else
                     setTimeout(try_adding, 125);
               }
               try_adding();
            }
          };  // end of SFW definition

if ("_saved_SFW" in window)
{
   for (p in window._saved_SFW)
      SFW[p] = window._saved_SFW[p];

   delete window._saved_SFW;
}

function init_SFW(callback)
{
   SFW.start_app            = callback;


   SFW.log_error            = _log_error;
   SFW.has_value            = _has_value;
   SFW.get_property         = _get_property;
   SFW.confirm_not_null     = _confirm_not_null;
   SFW.prepare_top_sfw_host = _prepare_top_sfw_host;
   SFW.get_el_sfw_host      = _get_el_sfw_host;
   SFW.seek_top_sfw_host    = _seek_top_sfw_host;
   SFW.seek_page_anchor     = _seek_page_anchor;
   SFW.seek_child_anchor    = _seek_child_anchor;
   SFW.seek_event_actors    = _seek_event_actors;
   SFW.seek_event_object    = _seek_event_object;
   SFW.derive               = _derive;
   SFW.document_object      = _document_object;
   SFW.add_event            = _add_event;
   SFW.setup_event_handling = _setup_event_handling;
   SFW.cancel_event         = _cancel_event;

   // functions found in sfw_doc.js:
   // SFW.alert
   // SFW.confirm
   // SFW.alert_notice
   // SFW.is_xmldoc
   // SFW.get_schema_idfield;
   // SFW.get_result_idname
   // SFW.check_for_preempt
   // SFW.update_xsl_keys

   SFW.keycode_from_event   = _keycode_from_event;
   SFW.keychar_from_event   = _keychar_from_event;

   SFW.close_subhosts       = _close_subhosts;
   SFW.set_view_renderer    = _set_view_renderer;
   SFW.update_selected_view = _update_selected_view;
   SFW.change_view          = _change_view;
   SFW.return_callback_func = _return_callback_func;
   SFW.process_event        = _process_event;
   SFW.process_rogue_button = _process_rogue_button;
   SFW.process_host_keydown = _process_host_keydown;
   SFW.get_dup_node         = _get_dup_node;
   SFW.get_deleted_attribute= _get_deleted_attribute;
   SFW.remove_deleted_row   = _remove_deleted_row;
   SFW.update_xmldoc        = _update_xmldoc;

   // The following commonly-used search function is found in sfw_dom.js:
   // SFW.find_child_matches(parent, function, first_only, recurse)

   SFW.setup_sfw_host        = _setup_sfw_host;
   SFW.make_sfw_host         = _make_sfw_host;
   SFW.get_last_SFW_Host    = _get_last_SFW_Host;
   SFW.keep_top_merged_element = _seek_top_merged_element;
   SFW.arrange_in_host      = _arrange_in_host;
   SFW.size_to_cover        = _size_to_cover;
   SFW.resize_page          = _resize_page;
   SFW.translate_url        = _translate_url;
   SFW.apply_row_context    = _apply_row_context;
   SFW.create_custom_form   = _create_custom_form;
   SFW.create_interaction   = _create_interaction;
   SFW.render_interaction   = _render_interaction;
   SFW.open_interaction     = _open_interaction;
   SFW.get_result_to_replace= _get_result_to_replace;
   SFW.run_task             = _run_task;
   SFW.get_director         = _get_director;
   SFW.update_location_arg  = _update_location_arg;
   SFW.base                 = _base;  // "base class" for _form, _table, etc.
   SFW.types["iclass"]      = _base;

   SFW.show_string_in_pre   = _show_string_in_pre;
   SFW.remove_string_pres   = _remove_string_pres;

   SFW.copy_attributes      = _copy_attributes;
   SFW.replace_results      = _replace_results;
   SFW.get_row_id_value     = _get_row_id_value;

   SFW.get_row_from_result_id = _get_row_from_result_id;

   SFW.row_name_from_schema = _row_name_from_schema;

   SFW.get_tasks_from_host  = _get_tasks_from_host;
   SFW.get_object_from_host = _get_object_from_host;
   SFW.get_cfobj_result     = _get_cfobj_result;
   SFW.get_update_row       = _get_update_row;
   SFW.get_urow_from_cfobj  = function(o) {
      return ("update_row" in o)?o.update_row:null; };

   SFW.get_xrow_from_cfobj  = function(o) { return _get_property(o,"cdata","xrow"); };

   SFW.stage                = document.getElementById("SFW_Content");
   SFW.px                   = _px;
   SFW.addXMLEl             = _addXMLEl;

   SFW.collect_form_fields  = _collect_form_fields;
   SFW.get_shift_tab_field  = _get_shift_tab_field;

   function _px(num)    { return String(num)+"px"; };

   // Alternative to XML.js::addEl() to create elements without XHTML namespace:
   function _addXMLEl(tag,parent,before,ns)
   {
      // The following function is from XML.js (in case of error):
      return add_namespace_el(tag,ns||"",parent,before);
   }

   function _log_error(msg)
   {
      console.error(msg);
      if (SFW.debugging)
         debugger;
   }

   function _show_string_in_pre(str)
   {
      var pre = addEl("pre", document.body);
      pre.className = "root_pre";
      pre.appendChild(document.createTextNode(str));
      return pre;
   }

   function _remove_string_pres()
   {
      var b = document.body;
      function f(n) { if (n.nodeType==1 && n.className=="root_pre") b.removeChild(n); }
      SFW.find_child_matches(b,f);
   }


   /**
    * Confirms existence and value of an object's properties.
    *
    * The arguments that follow *obj* are property names, and if there
    * are more than one property name, each following name is tested against
    * the previous discovered objects.
    *
    * For example, `_has_value(obj, "cdata", "os");`  will check for obj.cdata,
    * and if that's found, will go on to check for obj.cdata.os.
    *
    * @return true if found and not null, false otherwise.
    */
   function _has_value(obj)
   {
      for (var i=1, stop=arguments.length; i<stop; ++i)
      {
         var name = arguments[i];
         if (!(name in obj) || obj[name]===null)
            return false;
         obj = obj[name];
      }
      return true;
   }

   /**
    * Returns property if it exists, otherwise returns null.
    * 
    * Like _has_value(), gets successive properties according to the argument list,
    * returning the property that matches the final argument if found.  If any
    * of the named properties are missing, the function immediately returns null.
    *
    * If a property is a function, a function object will be created that calls
    * the function as a zero-parameter member function of the current rval value.
    *
    * If the function is the last property requested, it will be returned as a
    * function object, otherwise it will be evaluated before extracting the next
    * property in the list.
    *
    * Add a null value after a method name if you want to return the result of the
    * method rather than a function reference.
    */
   function _get_property(obj)
   {
      var rval = obj;
      if (obj)
      {
         for (var i=1, stop=arguments.length; i<stop; ++i)
         {
            if (typeof(rval)=="function")
               rval = rval();

            if (!rval)
               break;

            var name = arguments[i];

            // Don't allow 0 to bypass the evaluation
            if (name==0 || name)
            {
               if (typeof(rval[name])=="function")
               {
                  var o = rval;
                  var f = rval[name];
                  rval = function() { return f.call(o); };
               }
               else if (typeof(rval)!="object" || !(name in rval))
                  return null;
               else
                  rval = rval[name];
            }
         }
      }
      return rval;
   }

   /** Sends an error message if the specific object value if null. */
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

   function _get_el_sfw_host(el)
   {
      var n = el.parentNode;
      while (n.nodeType <= 3)
      {
         if (class_includes(n,"SFW_Host"))
            return n;
         n = n.parentNode;
      }
      return null;
   }

   function _prepare_top_sfw_host()
   {
      var anchor, sfwhost = SFW.seek_top_sfw_host();
      if (sfwhost)
      {
         var obj = SFW.get_object_from_host(sfwhost);
         if (obj)
         {
            SFW.base.call(obj, {host:sfwhost});
            SFW.arrange_in_host(sfwhost, obj.top());

            SFW.setup_sfw_host(sfwhost, SFW.xmldoc);
            obj.initialize();
         }
         else if ((anchor=_find_anchor(sfwhost)))
         {
            SFW.arrange_in_host(sfwhost, anchor);
         }
      }
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
                  fcount+=2;

                  is_control=t.getAttribute("data-sfw-control");
                  if (is_control)
                  {
                     rval["widget"] = t;
                     ++fcount;
                  }
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
      if (actors)
      {
         var iclass=actors.iclass;
         if (iclass in SFW.types)
            return new SFW.types[iclass](actors);
         else
         {
            SFW.alert("iclass '" + iclass + "' is not registered in SFW.types.");

            // Squelch further warnings:
            SFW.derive(_base, iclass, "iclass");
         }
      }
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
         SFW.alert("Base class \"" + name + "\" is undefined.");
         return false;
      }
      return true;
   }

   function _new_type_unique(name)
   {
      if (name in SFW.types)
      {
         SFW.alert("IClass \"" + name + "\" is already defined.");
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
 
   function _seek_top_merged_element(doc)
   {
      if (!doc)
         doc = SFW.xmldoc;
      return doc.selectSingleNode("/*/*[@merged]");
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
      // Standard process handlers, in order of precedence:
      var handlerarr = [process_dpicker, Moveable.process_event, SFW.process_event];
      var handler_count = handlerarr.length;

      function update_handler_count() { handler_count = handlerarr.length; }

      SFW.add_handler = function(handler, first)
      {
         if (first)
            handlerarr.splice(0,0,handler);
         else
            handlerarr.push(handler);
         update_handler_count();
      };

      SFW.remove_handler = function(handler)
      {
         for (var i=0; i<handler_count; ++i)
         {
            if (handler===handlerarr[i])
            {
               splice(i, 1);
               update_handler_count();
               break;
            }
         }
      };

      function f(ev)
      {
         var e=ev||window.event;
         var t=e.target||e.srcElement;

         if (e.type=="resize")
         {
            _resize_page();
            return true;
         }

         for (var i=0; i<handler_count; ++i)
         {
            if (!handlerarr[i](e,t))
               return false;
         }

         if (e.type=="keydown")
         {
            var kcode=_keycode_from_event(e);
            var key_y=89, key_u=85;
            if (e.ctrlKey && e.altKey && (kcode==key_y || kcode==key_u))
            {
               if (e.shiftKey)
                  _remove_string_pres();
               else if (kcode==key_y)
                     _show_string_in_pre(serialize(SFW.xsldoc));
               else
                     _show_string_in_pre(serialize(SFW.xmldoc));
               return true;
            }

            var host, obj;
            if ((host=_get_last_SFW_Host()) && (obj=_get_object_from_host(host)))
               return _process_host_keydown(e, host,obj);
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
      Events.add_event("blur",f);

      window.onresize = f;
   }

   function _cancel_event(e)
   {
      function common(e) { e.cancelBubble=true; e.returnValue=false; return false;}
      if ("stopPropagation" in e)
         _cancel_event = function(e) { e.stopPropagation(); return common(e); };
      else if ("preventDefault" in e)
         _cancel_event = function(e) { e.preventDefault(); return common(e); };
      else
         _cancel_event = common;

      return _cancel_event(e);
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

   function _keychar_from_event(e)
   {
      if ("key" in e)
         SFW.keychar_from_event = function(e) { return e.key; };
      else if ("char" in e)
         SFW.keychar_from_event = function(e) { return e.char; };

      return SFW.keychar_from_event(e);
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

   function _close_subhosts()
   {
      var host, obj;
      while ((host=_get_last_SFW_Host()) && (obj=_get_object_from_host(host)))
      {
         if (obj.closeable())
            obj.selfclose();
         else
            break;
      }
      return host;
   }

   function _get_view_renderer_element(xsldoc)
   {
      if (!("view_renderer" in xsldoc))
      {
         var arr = [ "/xsl:stylesheet",
                     "xsl:template[@match='*[@rndx]'][@mode='result_fill_host']",
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

   function _return_callback_func(call, args_arr)
   {
      var curname, curobj, tof, curctx;
      var arr = call.split(".");

      function unrec(name)
      {
         return function() { SFW.alert("Unrecognized type ("
                                       + name
                                       + ") while parsing call='"
                                       + call
                                       + "')");
                           };
      }

      for (var i=0,stop=arr.length; i<stop; ++i)
      {
         curobj = null;
         curname = arr[i];

         if (!curctx)
         {
            if (curname in this)
               curctx = this;
            else if (curname in window)
               curctx = window;
         }

         if (!curctx)
            return unrec(curname);

         if (curctx && curname in curctx)
         {
            curobj = curctx[curname];
            switch(typeof(curobj))
            {
               case "object":
                  curctx = curobj;
                  break;
               case "function":
                  return function() { curobj.apply(curctx,args_arr); };
               default:
                  return unrec(curname);
            }
         }
      }
      return null;
   }

   function _process_event(e,t)
   {
      // Get parent if text or attribute node:
      if (t.nodeType==2 || t.nodeType==3)
         t = t.parentNode;

      if (t.nodeType==1 && t.tagName.toLowerCase()=="img"
          && t.parentNode.tagName.toLowerCase()=="button")
         t = t.parentNode;

      // Preempt host search if clicking a view button
      if (t.nodeType==1 && class_includes(t,"view_selector"))
      {
         if (!class_includes(t,"selected"))
            _change_view(t.getAttribute("data-name"));

         return false;
      }

      var obj = _seek_event_object(t);
      if (obj)
         return obj.process(e,t);
      else if (t.tagName.toLowerCase()=="button")
         return ! _process_rogue_button(t);
      else
         return true;
   }

   /**
    * Process unaffiliated (not child of an object) buttons.
    * Returns true if button will be handled (in future by timeout),
    * and false if not handled.  Reverse value if called by event
    * handler.
    */
   function _process_rogue_button(b, cb)
   {
      var type = b.getAttribute("data-type");
      var url = b.getAttribute("data-task") || b.getAttribute("data-url");
      var cmsg = b.getAttribute("data-confirm");

      var cfunc = cmsg ? function(){return SFW.confirm(csmg);} : function() { return true; };

      var delayed_func = null;

      switch(type)
      {
         case "jump":
         case "import":
            if (url)
               delayed_func = function(){ if (cfunc()) window.location=url; };
            break;
         case "call":
            if (url)
            {
               var dfunc = _return_callback_func(url, [b,cb]);
               delayed_func = function(){ if (cfunc()) dfunc(); };
            }
            break;
         case "launch":
            if (url)
               delayed_func = function(){ if (cfunc()) window.open(url); };
            break;
         default:
            var pb_name = "process_button_" + type;
            if (pb_name + type in window)
               delayed_func = function() { if(cfunc()) window[pb_name](b,cb); };
      }

      if (delayed_func)
      {
         setTimeout(delayed_func);
         return true;
      }
      else
         return false;
   }

   // The first three characters of the keyname is unique enough to
   // identify the keys .
   var _keymap = {  8:"bac",   // backspace
                    9:"tab",
                    13:"ent",  // enter
                    27:"esc",  // escape
                    32:"spa",  // space
                    33:"pgu",  // pgup
                    34:"dow",  // down
                    35:"end",
                    36:"hom",  // home
                    37:"lef",  // left
                    38:"up",
                    39:"rig",  // right
                    40:"pgd",  // pgdn
                    45:"ins",  // insert
                    46:"del",  // delete
                    108:"min", // minus
                    187:"plu", // plus
                    189:"min"  // minus
                 };

   function _key_matches_code(key,code)
   {
      if (code>=112 && code<=123)
         return key=="f"+String(code-111);
      else
         return key.substring(0,3)==_keymap[code];
   }

   function _is_shiftable(ch)
   {
      return (ch>=65 && ch<=90)  // A-Z
         || (ch>=97 && ch<=122)  // a-z (likely not used for keycode);
         || (ch>=32 && ch<=46)   // keypad keys
         || (ch<=20)             // control keys like backspace, space, tab, enter
         || ch==45 || ch==46     // insert and delete
      ;
   }

   /**
    * Reports whether or not the keyboard event matches the keystroke described by key_spec.
    *
    * @param keyspec String of the form *ctrl-alt-delete* or *plus* or *alt-g*
    * @param event   The keyboard event
    */
   function _keyspec_matches_event(keyspec, event)
   {
      var ev_code = _keycode_from_event(event);
      var ev_char = _keychar_from_event(event).toLowerCase();

      var keylist = keyspec.split("-");
      var key = keylist.splice(-1,1)[0].toLowerCase();
      var map = {c:false, a:false, s:false};

      var match = key.length==1?(key==ev_char):_key_matches_code(key,ev_code);

      if (match)
      {
         for (var i=0; i<keylist.length; ++i)
         {
            switch(keylist[i][0].toLowerCase())
            {
               case "c": map.c = true; break;
               case "a": map.a = true; break;
               case "s": map.s = true; break;
               default : break;
            }
         }

         map.c = (map.c == event.ctrlKey);
         map.a = (map.a == event.altKey);
         map.s = (!_is_shiftable(ev_code) || map.s == event.shiftKey);

         return map.c && map.a && map.s;
      }

      return false;
   }

   function _event_matches_el(event, el)
   {
      var key_array = el.getAttribute("data-key").split(/\s/);

      for (var i=0,stop=key_array.length; i<stop; ++i)
      {
         if (_keyspec_matches_event(key_array[i], event))
            return true;
      }

      return false;
   }

   function _process_host_keydown(e, host, obj)
   {
      var kc = _keycode_from_event(e);

      if (kc==27 && obj.closeable())
      {
         obj.selfclose();
         return false;
      }
      // Ignore alt, control, and shift keys until processing the key
      else if (kc!=16 && kc!=17 && kc!=18)
      {
         var task_els = _get_tasks_from_host(host);
         if (task_els)
         {
            for (var i=0,stop=task_els.length; i<stop; ++i)
            {
               var el = task_els[i];

               if (!class_includes(_get_property(el,"parentNode", "parentNode"), "floater"))
               {
                  if (_event_matches_el(e, el))
                  {
                     obj.process_clicked_button(el);
                     return false;
                  }
               }
            }
         }
      }
      return true;
   };

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

   var reRplRowVals = /\{\!([^\}]+)\}/g;

   function _apply_row_context(url, row)
   {
      function rep(full, sub)
      {
         var aval = row.getAttribute(sub);
         if (aval)
            return encodeURIComponent(aval);
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
      var s = anchor.style;

      var os_host = SFW.get_doc_offset(host);
      var center_host = Math.round(host.offsetWidth / 2 + os_host.left);
      var center_anchor = Math.round(center_host - anchor.offsetWidth/2);
      
      s.top = _px(os_host.top);
      s.left = _px(center_anchor);
      // s.zIndex = 100;
   }

   function _size_to_cover(over, under)
   {
      var os_host = SFW.get_doc_offset(under);
      var combined = os_host.top + under.offsetHeight;
      over.style.height = SFW.px(combined);
   }


   function _setup_sfw_host(shost, xmldoc, caller, data)
   {
      shost.xmldoc = xmldoc;
      shost.caller = caller;
      shost.data = data;
   }

   function _make_sfw_host(host, xmldoc, caller, data)
   {
      // Get last host before creating a new one.
      // A new host becomes the new last host, making
      // _get_last_SFW_Host() return the wrong element.
      var lhost = _get_last_SFW_Host();

      var thost = addEl("div",host);
      thost.className = "SFW_Host";

      if (lhost)
      {
         thost.style.height = _px(lhost.offsetHeight);
         thost.style.width  = _px(lhost.offsetWidth);
      }

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

   function _get_dup_node(target, source)
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

   function _replace_element(newel, oldel)
   {
      var parent = oldel.parentNode;

      var before = null;
      if (oldel)
      {
         before = SFW.next_sibling_element(oldel);
         parent.removeChild(oldel);
      }

      if (before)
         parent.insertBefore(newel, before);
      else
         parent.appendChild(newel);
   }

   function _get_row_id_value(row)
   {
      var val;
      var schema = row.selectSingleNode("../schema");
      if (schema)
      {
         var field = SFW.get_schema_idfield(schema);
         if (field)
            val = row.getAttribute(field.getAttribute("name"));
      }
      else if (!(val=row.getAttribute("id")))
         SFW.alert("Failed to find a usable id value for row named \""
                   + row.tagName
                   + "\"");
      return val;
   }

   function _get_oldrow(target, row)
   {
      var idname, idval, rowname;
      if (row
          && (idname=SFW.get_result_idname(target,row))
          && (idval=row.getAttribute(idname))
          && (rowname=target.getAttribute("row-name")))
         return target.selectSingleNode(rowname + "[@" + idname + "='" + idval + "']");

      return null;
   }

   function _get_target_result(update, pagedoc)
   {
      var rname, target = null;
      if ((rname=update.getAttribute("target")))
         target = pagedoc.selectSingleNode("/*/" + rname);
      else if ((rname=update.getAttribute("row-name")))
         target = pagedoc.selectSingleNode("/*/*[@rndx][not(@merged)][@row-name='" + rname + "']");

      return target;
   }

   function _get_result_from_field(field, docel)
   {
      var result = docel.selectSingleNode(field.getAttribute("result"));
      if (result.getAttribute("type")=="association")
         result = docel.selectSingleNode(result.getAttribute("result"));

      return result;
   }

   function _put_row_into_target(target, row)
   {
      var oldrow = _get_oldrow(target,row);
      var nrow = _get_dup_node(target,row);
      var rname = nrow.localName;
      var trname = target.getAttribute("row-name");

      if (rname==trname)
      {
         if (oldrow)
            _replace_element(nrow,oldrow);
         else
            target.appendChild(nrow);
      }
      else
         SFW.alert("The update row name (" + (rname||"NULL") +
                   ") does not match the result's row-name (" + trname +").");
   }

   function _get_deleted_attribute(doc)
   {
      var attr;
      if (SFW.is_xmldoc(doc))
      {
         var xpath = "/*/*[@rndx]/*[local-name()=(../@row-name)]/@deleted";
         attr = doc.selectSingleNode(xpath);
      }
      return attr;
   }

   function _remove_deleted_row(doc, form)
   {
      var xrow = form.get_context_row();
      if (xrow)
         xrow.parentNode.removeChild(xrow);
   }

   function _remove_deletes(pagedocel, newdocel, form)
   {
      var deleted_count = 0;
      var xpath_id = "@*[local-name()!='deleted']";
      var dresults = newdocel.selectNodes("*[@rndx][*[local-name()=../@row-name][@deleted]]");
      for (var i=0,stop=dresults.length; i<stop; ++i)
      {
         var dresult = dresults[i];
         var dels = dresult.selectNodes(dresult.getAttribute("row-name")+"[@deleted]");

         if (dels.length==0)
            continue;

         var rowname, idname;
         var target = pagedocel.selectSingleNode(dresult.getAttribute("target")||"*[1=0]");
         if (target)
         {
            rowname = target.getAttribute("row-name");
            idname = SFW.get_result_idname(target);
         }

         var id_attr, row_id;
         for (var j=0, jstop=dels.length; j<jstop; ++j)
         {
            var row_to_remove;
            var del = dels[j];
            var is_del = del.getAttribute("deleted");
            if (is_del > 0)
            {
               if (i==0 && j==0 && !target && form)
                  row_to_remove = form.get_context_row();
               else if (target)
               {
                  if ((id_attr=del.selectSingleNode(xpath_id)) && (row_id=id_attr.value))
                  {
                     var xpath_row = rowname + "[@" + idname + "='" + row_id + "']";
                     row_to_remove = target.selectSingleNode(xpath_row);
                  }
               }
            }

            if (row_to_remove)
            {
               ++deleted_count;
               row_to_remove.parentNode.removeChild(row_to_remove);
            }
         }  // end of for(var j...) loop

      } // end of for(var i...) loop

      return deleted_count;
   }

   function _update_result_from_result(target, source)
   {
      var rowname = target.getAttribute("row-name");
      var idname = SFW.get_result_idname(target);

      function mm(dom,l,r) { SFW.alert("Mismatched "+dom+", "+l+"!="+r); }
      function matched()
      {
         var msg;
         var rname=source.getAttribute("row-name");
         if (rname==rowname)
         {
            var iname = SFW.get_result_idname(source);
            if (iname!=idname)
            {
               mm("idname",idname,iname);
               return false;
            }
         }
         else
         {
            mm("row-name",rowname,rname);
            return false;
         }
         return true;
      }

      if (!matched())
         return;

      var urows = source.selectNodes(rowname);
      var xpathbase = rowname + "[@" + idname + "='";

      for (var j=0,jstop=urows.length; j<jstop; ++j)
      {
         var urow = _get_dup_node(target, urows[j]);
         var id = urow.getAttribute(idname);
         var oldrow = target.selectSingleNode(xpathbase+id+"']");

         if (oldrow)
            _replace_element(urow,oldrow);
         else
            target.appendChild(urow);
      }
   }

   function _process_updates(pagedocel, newdocel, form)
   {
      var caller, xpath, xrow, xresult;

      // xrow and xpath will be undefined if form is undefined:
      if (form && (xrow=form.get_context_row()))
         xresult = xrow.parentNode;
      if (form && !xresult)
         if ((caller=form.caller()) && (xpath=caller.get_result_path_from_top()))
            xresult = pagedocel.selectSingleNode(xpath);

      var updates = newdocel.selectNodes("*[@rndx][@type='update']");
      for (var i=0, stop=updates.length; i<stop; ++i)
      {
         var result = updates[i];
         var updater_name = result.getAttribute("updater");
         var updater_func = (updater_name in window) ? window[updater_name] : null;

         if (updater_func)
         {
            updater_func(pagedocel, result);
            continue;
         }

         var target, rowname = result.getAttribute("row-name");

         var tname = result.getAttribute("target");
         if (tname)
         {
            xpath = tname?tname:"*[@rndx][not(@merged)][@row-name='"+rowname+"']";
            target = pagedocel.selectSingleNode(xpath);
         }
         else if (xresult)
            target = xresult;

         if (target)
         {
            if (updater_name)
            {
               SFW.alert("Going to use '" + updater_name + "' to update the data.");
            }
            else
            {
               var xrow_path;
               // If form is undefined, xrow is undefined, the condition fails
               if (xrow)
               {
                  var idname = SFW.get_result_idname(target);
                  var xrowname = xrow.parentNode.getAttribute("row-name");
                  var idval = xrow.getAttribute(idname);
                  xrow_path = SFW.get_result_xpath(target);
                  xrow_path += "/" + xrowname + "[@" + idname + "='" + idval + "']";
               }

               _update_result_from_result(target, result);

               // If form is undefined, xrow_path is undefined because it wasn't set
               // in condition above for xrow:
               if (xrow_path)
               {
                  var newxrow = target.selectSingleNode(xrow_path);
                  if (newxrow)
                     form.update_context_row(newxrow);
               }
            }
         }
      }
   }

   /**
    * Now does both delete and add/replace updates.
    *
    * To facilitate appropriate behavior in form.prototype.process_button(), this
    * function will return false if it failed to delete any record when mode-type=="delete".
    *
    * If a record delete succeeds, the deleted record form should close to avoid attempts
    * to modify the now missing record.
    */
   function _update_xmldoc(doc, form, type)
   {
      var pagedocel, caller;

      if ((caller=form.caller()))
         pagedocel = _get_property(form, "caller","xmldoc","documentElement");

      if (!pagedocel)
         pagedocel = SFW.xmldoc.documentElement;

      var newdocel = doc.documentElement;

      _process_updates(pagedocel, newdocel, form, type);

      if (newdocel.getAttribute("mode-type")=="delete")
         return _remove_deletes(pagedocel, newdocel, form) > 0;

      return true;
   }

   /**
    * Modifies and merges new document contents into root document.
    * 
    * Before the elements of the merged document are incorporated into the
    * root document, they must be massaged to carry some extra information to
    * facilitate their use and for housekeeping when the merged interactions
    * are terminated.
    *
    * The elements are tagged with a *merged* integer attribute that links the
    * elements together and distinguishes them from other merged elements.
    *
    * The other merge attribute is the *script* attribute in case the SRM file
    * of the merge document is different than that root document.  An accurate
    * *script* attribute is necessary for self-referencing URLs (that begin with ?).
    */
   function _merge_into_pagedoc(pagedoc, newdoc)
   {
      var ibefore = _find_insertbefore_able(pagedoc);
      var docel = pagedoc.documentElement;
      var merge_number = 1;
      var el = _seek_top_merged_element(pagedoc);
      if (el)
         merge_number = 1+Number(el.getAttribute("merged"));
      
      function f(n)
      {
         if (n.nodeType==1)
         {
            var pl = make_importable_node(pagedoc, n, true);
            pl.setAttribute("merged",merge_number);
            var script = newdoc.documentElement.getAttribute("script");
            if (script)
               pl.setAttribute("script",script);
            docel.insertBefore(pl, ibefore);
         }
      }
      SFW.find_child_matches(newdoc.documentElement,f,false,false);

      return merge_number;
   }

   function _remove_merged_elements(pagedoc, merge_number)
   {
      var docel = pagedoc.documentElement;
      var el = _seek_top_merged_element(pagedoc);
      if (el)
      {
         var num = el.getAttribute("merged");
         function f(n)
         {
            if (n.nodeType==1 && n.getAttribute("merged")==num)
               docel.removeChild(n);
         }
         SFW.find_child_matches(docel,f,false,false);
      }
   }

   function _open_interaction(host, url, caller, data)
   {
      function got(xdoc)
      {
         var merge, pagedoc, node_to_remove;
         if (SFW.check_for_preempt(xdoc))
         {
            // Allow for updates from a new interaction.  In particular, this
            // is an opportunity to refresh data that may be new since the
            // page was rendered.
            _process_updates(SFW.xmldoc.documentElement, xdoc.documentElement);

            SFW.update_xsl_keys(xdoc);
            var modetype = xdoc.documentElement.getAttribute("mode-type");

            if (modetype=="form-jump")
            {
               var root="Mode-type form-jump failed to find the ";
               var result=xdoc.selectSingleNode("/*/*[@rndx][jumps]");
               if (result)
               {
                  var row=result.selectSingleNode("*[local-name()=../@row-name][@error]");
                  if (row)
                  {
                     var xpath="jumps/@jump" + row.getAttribute("error");
                     var destination=result.selectSingleNode(xpath);
                     if (destination)
                        window.location = destination.value;
                     else
                        console.error(root+"destination.");
                  }
                  else
                     console.error(root+"error attribute.");
               }
               else
                  console.error(root+"jumps element.");

              return;
            }

            if (modetype=="merge")
            {
               // Copy new stuff into page document so the new
               // stuff can access common data residing in the
               // page document (lookup results, etc):
               if ((pagedoc=caller.xmldoc()))
               {
                  // Ensure that a data object exists to which the merge number will be attached.
                  if (!data)
                     data = {};

                  data.merge_number = _merge_into_pagedoc(pagedoc, xdoc);
                  xdoc = pagedoc;
               }
               else
                  console.error("Can't find caller document for xdoc merging.");
            }

            _render_interaction(xdoc, host, caller, data);
         }
      }

      function fail(xhr) { console.error("_open_interaction() Failed to get " + url); }

      xhr_get(url, got, fail);
   }

   function _get_result_to_replace(replacement_result)
   {
      var name = replacement_result.getAttribute("target") || replacement_result.localName;
      var xpath = "/*/*[@rndx][local-name()='" + name + "']";
      return SFW.xmldoc.selectSingleNode(xpath);
   }

   function _run_task(url, callback)
   {
      function fail(xhr)
      {
         console.error("_run_task() Failed to get " + url);
         if (callback)
            callback(null);
      }

      /**
       * This function will replace the contents of a result element in
       * the source document with the contents of another result, which
       * will typically be from a document returned from form data submission.
       *
       * I added this function because I couldn't remember or find the
       * function cascade_updates(), which might have been a better fit.  I'm
       * keeping this function, for now, because it doesn't rely on a caller
       * argument.
       */
      function process_replace(result)
      {
         var target = _get_result_to_replace(result);
         if (target)
            _update_result_from_result(target, result);
         else
            console.error("Unable to find result to replace.");
      }
     
      function got(xdoc)
      {
         if (SFW.check_for_preempt(xdoc))
         {
            SFW.update_xsl_keys(xdoc);
            var modetype = xdoc.documentElement.getAttribute("mode-type");
            var type, target, result, results = xdoc.selectNodes("/*/*[@rndx]");
            for (var i=0,stop=results.length; i<stop; ++i)
            {
               result = results[i];
               type = result.getAttribute("type");
               if (type=="replace")
                  process_replace(result);
            }

            if (callback)
               callback(xdoc);
         }
      }

      xhr_get(url, got, fail);
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

   function _find_schema(doc, merge_number)
   {
      var docel = doc.documentElement;
      var schema = null;
      var merge_attr = merge_number ? "[@merged='"+merge_number+"']" : "[not(@merged)]";
      var arrx = ["schema"+merge_attr+"[1]", "*"+merge_attr+"[@rndx=1]/schema"];

      for (var i=0,stop=arrx.length; !schema && i<stop; ++i)
         schema = docel.selectSingleNode(arrx[i]);

      return schema;
   }

   function _update_location_arg(name, value)
   {
      var newarg = name + (value ? ("="+encodeURIComponent(value)):"");
      
      var loc = window.location;

      // Make array (that might be empty) of the arguments of the current location:
      var qsearch = loc.search.length==0
         ? ""
         : loc.search[0]=='?' ? loc.search.substring(1) : loc.search;
      qsearch = qsearch.length==0 ? [] : qsearch.split('?');

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

      SFW.alert( "Object " + ("class_name" in p ? p.class_name  : "") + "\n"
             + (arrF.length ? ("Functions\n   " + arrF.join("\n   ")) : "")
             + (arrO.length ? ("\nObjects\n   " + arrO.join("\n   ")) : "")
             + (arrS.length ? ("\nStrings\n   " + arrS.join("\n   ")) : "")
             + (arrX.length ? ("\nOther\n   "   + arrX.join("\n   ")) : ""));
   }

   function _get_row_from_result_id(result, idval)
   {
      var idattr = result.selectSingleNode("schema/field[@xrow-id or @auto-increment]/@name");
      if (_confirm_not_null(idattr, "Unable to identify id field."))
      {
         var xpath = result.getAttribute("row-name") + "[@" + idattr.value + "='" + idval + "']";
         return result.selectSingleNode(xpath);
      }
      else
         return null;
   }

   function _row_name_from_schema(schema)
   {
      var parent, name = null;
      if (schema)
      {
         name = schema.getAttribute("row-name") || schema.getAttribute("name");
         if (!name && (parent=schema.parentNode).getAttribute("rndx"))
            name = parent.getAttribute("row-name");
      }

      return name;
   }

   function _get_tasks_from_host(host)
   {
      function f(n) { return n.nodeType==1 && n.getAttribute("data-key"); }
      return SFW.find_child_matches(host,f,false,true);
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

   function _get_update_row(doc)
   {
      var xpath = "/*/*[@rndx][@type='update']/*[local-name()=../@row-name]";
      return doc?doc.selectSingleNode(xpath):null;
   }

   /**
    * Copies attributes from source to target elements.
    *
    * Since the framework stashes persistent status values in the result element,
    * those values must be copied to any new result that will replace the original
    * result element.
    *
    * For now, all attributes in the original result (which should be the source
    * argument) are copied to the replacement result (target).  In the future, it
    * may be necessary to indicate new attributes that should override the old by
    * listing their names in the SRM.  I won't over-engineer this right now as I
    * don't recognize any cases where this would be useful.
    */
   function _copy_attributes(target, source)
   {
      var s_attr, t_val, s_val;
      for (var i=0, stop=source.attributes.length; i<stop; ++i)
      {
         s_attr = source.attributes[i];
         target.setAttribute(s_attr.localName, s_attr.value);
      }
   }

   /**
    * Scans newdoc for results, replacing like-named results in olddoc
    * with modified (attriubtes updated) newdoc results.
    *
    * @param olddoc May be null/undefined and then will be replace with
    *               SFW.xmldoc;
    */
   function _replace_results(newdoc, olddoc)
   {
      if (!olddoc)
         olddoc = SFW.xmldoc;

      var rndx, xpath, oldnode;
      var newdocel = newdoc.documentElement;
      var olddocel = olddoc.documentElement;
      function f(n)
      {
         if (n.nodeType==1 && (rndx=n.getAttribute("rndx")))
         {
            var xpath = n.tagName + "[@rndx]";
            if ((oldnode=olddocel.selectSingleNode(xpath)))
            {
               _copy_attributes(n, oldnode);
               olddocel.insertBefore(n, oldnode);
               olddocel.removeChild(oldnode);
            }
            return true;
         }
         return false;
      }
      SFW.find_child_matches(newdocel, f);
   }

   function _get_cfobj_result(cfobj)
   {
      var cd, xrow, update_row;
      if ("cdata" in cfobj && (cd=cfobj.cdata) && "xrow" in cd)
         xrow = cd.xrow;
      if (cfobj.mtype!="delete")
         update_row = cfobj.update_row;

      function check(row)
      {
         var pn = row ? row.parentNode : null;
         if (pn && !pn.getAttribute("rndx"))
            pn = null;
         return pn;
      }

      return check(update_row) || check(xrow) || null;
   }

   var html_itypes = ["input","select","textarea","button"];

   // Call after confirming nodeType==1
   function _is_tabable_input(n)
   {
      return n.nodeType==1 && 
         (
            n.getAttribute("data-sfw-control") ||
            (
               html_itypes.includes(n.tagName.toLowerCase())
                  && !n.getAttribute("disabled")
                  && !n.getAttribute("readOnly")
                  && n.type!="hidden"
            )
         );
   }

   function _get_host_form(el)
   {
      while (el)
      {
         if (el.nodeType==1 && el.tagName.toLowerCase()=="form" && el.getAttribute("data-sfw-class"))
            return el;
         el = el.parentNode;
      }

      return null;
   }

   function _collect_form_fields(form)
   {
      return SFW.find_child_matches(form,_is_tabable_input,false,true);
   }

   function _find_tabable_ancestor(el)
   {
      while(el && el.nodeType==1)
      {
         if (_is_tabable_input(el))
            return el;
         else if (el.tagName.toLowerCase()=="form")
            break;

         el = el.parentNode;
      }
      return null;
   }

   // Returns the tabable field, in the same form, that immediately preceeds
   // the passed field.
   // Returns NULL if the field:
   //   - is not in a form
   //   - is not a tabable field
   //   - is already the first tabable field in a form.
   function _get_shift_tab_field(field)
   {
      var input, form;
      if ((form=_get_host_form(field)) && (input=_find_tabable_ancestor(field)))
      {
         var arr = _collect_form_fields(form);
         for (var i=0,stop=arr.length; i<stop; ++i)
         {
            if (arr[i]==input)
            {
               if (i)
                  return arr[i-1];
               else
                  break;
            }
         }
      }
      return null;
   }

   function _base(actors)
   {
      this._host_el = actors.host;
      if ("widget" in actors)
         this._widget = actors.widget;
   };

   _base.prototype.host     = function() { return this._host_el||null; };
   _base.prototype.xmldoc   = function() {
      var r=this.host(); return (r && r.xmldoc) ? r.xmldoc : SFW.xmldoc;
   };

   _base.prototype.xmldocel = function() {
      var r=this.xmldoc(); return r?r.documentElement:null;
   };

   _base.prototype.data     = function() { var r=this.host(); return r?r.data:null; };

   _base.prototype.class_name = "iclass";
   _base.prototype.top       = function() { return _find_anchor(this.host()); };
   _base.prototype.schema    = function() { return _find_schema(this.xmldoc(), this.merge_number()); };
   _base.prototype.baseproto = function() { return this._baseproto; };
   _base.prototype.button_processors = {};

   _base.prototype.widget = function() { return ("_widget" in this)?this._widget:null; };
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
   // _base.prototype.closeable = function() { return false; };
   _base.prototype.closeable = function() { return this.caller() != null; };
   _base.prototype.selfclose = function()
   {
      var c;
      this.sfw_hide();
      if ((c=this.caller()))
         c.child_finished(this, true);
      this.sfw_close();
   };

   _base.prototype.do_proxy_override = function(name,args)
   {
      var field = this.get_schema_field();
      if (field)
      {
         var fname = field.getAttribute("proxy_" + name);
         if (fname && fname in window)
         {
            window[fname](this, args);
            return true;
         }
      }
      return false;
   };

   /**
    * Returns a populated object, or if actor_name set, the named sub-object.
    *
    * This function makes an object with as many of the following as it can find:
    * - input  (the HTML input element, if any)
    * - name   (the name of the pertinent field)
    * - schema (the schema element that governs the input)
    * - field  (the field element in the schema)
    * - result (if the field has a result attribute, this points to the named result)
    *
    * The existence of a later object (in the above list)  proves the existence of
    * the earlier objects, so it is only necessary to check for the latest object
    * required.
    *
    * The function looks for a field name, first by an input field, and if that fails,
    * by trying to get the @for value from the label elemtn.
    *
    * If a field name is found, the function continues to find and save field actors.
    *
    * By returning several objects, this function streamlines execution by
    * retaining the useful objects that give access to subsequent objects.
    * For example, you need the schema and the input element to get the field
    * element.  If you call an explicit function to get each of these objects,
    * you will discard the schema twice and the input element once to get the
    * field element.
    *
    * Additionally, the function caches the field_actors, so repeated calls
    * only acquires them once per object instantiation.
    */
   _base.prototype.get_field_actors = function(actor_name)
   {
      var rval = ("_f_actors" in this) ? this._f_actors : null;
      if (!rval)
      {
         this._f_actors = rval = {};

         var input = ("input" in this) ? this.input() : null;
         if (input)
         {
            rval.input = input;
            rval.name = input.getAttribute("name");
         }
         else
         {
            var widget = this.widget();
            if (widget)
            {
               var label = widget.parentNode.getElementsByTagName("label")[0];
               if (label)
                  rval.name = label.getAttribute("for");
            }
         }

         if (rval.name)
         {
            var schema = this.schema();
            if (schema)
            {
               rval.schema = schema;
               var field = schema.selectSingleNode("field[@name='"+rval.name+"']");
               if (field)
               {
                  var result = _get_result_from_field(field, this.xmldocel());
                  if (result)
                     rval.result = result;
               }
            }
         }
      }

      if (actor_name)
         return (actor_name in rval) ? rval[actor_name] : null;
      else
         return rval;
   };

   _base.prototype.get_field_name   = function() { return this.get_field_actors("name");   };
   _base.prototype.get_ref_result   = function() { return this.get_field_actors("result"); };
   _base.prototype.get_schema_field = function() { return this.get_field_actors("field");  };
   _base.prototype.get_schema_field_attribute = function(name)
   {
      var f = this.get_schema_field();
      if (f)
      {
         var v = f.getAttribute(name);
         if (v && v.length>0)
            return v;
      }
      return null;
   };

   _base.prototype.get_result_path_from_top = function()
   {
      var top = this.top();
      return top ? top.getAttribute("data-result") : null;
   };

   _base.prototype.get_data_path_from_top = function()
   {
      var top = this.top();
      return top ? top.getAttribute("data-path") : null;
   };

   _base.prototype.get_host_form = function() { return _get_host_form(this.top()); };
   
   _base.prototype.get_host_form_data_row = function()
   {
      var xpath, schema = this.schema();
      if (schema)
      {
         // If schema is child of documentElement,
         if (schema.parentNode.parentNode.nodeType==9)
            xpath = "/*/*[@rndx][1]/*[1]";
         else
         {
            var rname = _row_name_from_schema(schema);
            xpath = "../" + rname;
         }

         return schema.selectSingleNode(xpath);
      }
      else
         return null;
   };

   /** Get value of data-{name} attribute from the top element. */
   _base.prototype.get_data_value = function(name)
   {
      function rdata(a) { return a?a.getAttribute("data-"+name):null; }
      return rdata(this.top()) || rdata(this.widget());
   };

   /** Checks the argument list for a name that replaces "xxx" a on_xxx_click instruction. */
   _base.prototype.get_on_click_value = function(cname)
   {
      var ndx, rval;
      for (ndx in arguments)
      {
         if ((rval=this.get_data_value("on_" + arguments[ndx] + "_click")))
            return rval;
      }
      return null;
   };

   _base.prototype.add_schema_shadow = function(name, value)
   {
      var s, n;
      if ((s=this.schema()) && (n=addEl("shadow", s)))
         n.setAttribute(name, value);
      return n;
   };

   _base.prototype.sfw_close = function _sfw_close()
   {
      var v,p;
      if ((v=this.host()) && (p=v.parentNode))
         p.removeChild(v);
   };

   _base.prototype.sfw_hide = function _sfw_hide()
   {
      var v = this.host();
      if (v)
         v.style.display = "none";
   };

   _base.prototype.sfw_unhide = 
      _base.prototype.sfw_reveal = function _sfw_reveal()
   {
         var v = this.host();
         if (v)
            v.style.display = "";
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

   function _confirm_delete(update_row)
   {
      return update_row && ("0"!=update_row.getAttribute("deleted"));
   }

   _base.prototype.has_data = function() { return "data" in this.host(); };

   _base.prototype.cfobj_from_doc = function(doc)
   {
      var docel = doc.documentElement;
      var result = docel.selectSingleNode("*[@rndx=1][1]");
      
      var rval = { cfobj      : true,
                   child      : this,
                   docel      : docel,
                   mtype      : docel.getAttribute("mode-type"),
                   rtype      : null,
                   result     : null,
                   target_name: null,
                   update_row : null,

                   close : function() { _child_close(this.child); },
                   hide  : function() { _child_hide(this.child); }
                 };

      
      if (this.has_data())
         rval.cdata = this.host().data;

      if (result)
      {
         var rname = result.getAttribute("row-name") || "row";
         var target_name = result.getAttribute("target");
         rval.result = result;
         rval.update_row = result.selectSingleNode(rname);
         rval.rtype = result.getAttribute("type") || null;
         rval.target_name = result.getAttribute("target");
         rval["rname"] = rname;
         rval["confirm_delete"] = function() { return _confirm_delete(this.u_row); };
      }

      return rval;
   };

   _base.prototype.cfobj_from_cmd = function(cmd)
   {
      var rval;
      if (SFW.is_xmldoc(cmd))
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

   _base.prototype.process_open_button = function _process_open_button(b, cb)
   {
      var os = SFW.get_page_offset();  // Get offset before discarding contents
      var host = this.host();
      var url = b.getAttribute("data-task") || b.getAttribute("data-url");

      if (url)
      {
         // this.empty_node(host);

         SFW.open_interaction(SFW.stage,
                              url,
                              this,
                              { os:os, host:host, button:b }
                             );
         return false;
      }
      
      return true;
   };

   _base.prototype.process_clicked_button = function _process_clicked_button(b, cb)
   {
      if (_process_rogue_button(b,cb))
         return true;

      var type = b.getAttribute("data-type");
      var url = b.getAttribute("data-task") || b.getAttribute("data-url");
      var cmsg = b.getAttribute("data-confirm");

      // For setTimeout callback functions:
      var funcForTimeout = null;
      var ths = this;

      if (cmsg && !SFW.confirm(cmsg))
         funcForTimeout = function(){cb("cancel");};
      else
      {
         var caller;

         switch(type)
         {
            case "open":
               if (url)
                  funcForTimeout = function(){ths.process_open_button(b,cb);};
               break;
            case "cancel":
            case "close":
               if ((caller=this.caller()))
               {
                  ths.sfw_hide();
                  funcForTimeout = function(){caller.child_finished(ths,true);};
               }
               break;
            // The following have moved to _process_rogue_button:
            // case "jump":
            // case "import":
            //    if (url)
            //       funcForTimeout = function(){window.location=url;};
            //    break;
            // case "call":
            //    if (url)
            //       funcForTimeout = _return_callback_func(url, [b, cb]);
            //    else
            //       funcForTimeout = function(){SFW.alert("Call button type without url or task."); };
            //    break;
            // case "launch":
            //    if (url)
            //       funcForTimeout = function(){window.open(url);};
            //    break;

            default:
               // Gotta detect delete-type buttons and process without _open_interaction!

               var pbtype = "process_button_"+type;
               if (pbtype in this)
                  funcForTimeout = function(){ths[pbtype](b,cb);};
               else if (url)
               {
                  funcForTimeout = function()
                  {
                     url = _translate_url(url, ths.xmldocel());
                     _open_interaction(SFW.stage, url, ths);
                  };
               }
               break;
         }
      }

      if (funcForTimeout)
         setTimeout(funcForTimeout);

      return false;
   };

   /** Remove associated merged_data and remove merged_data value to prevent second attempt. */
   _base.prototype.remove_merged_results = function()
   {
      var data = _get_property(this,"host","data");
      var mnum = _get_property(data,"merge_number");
      if (mnum)
      {
         delete data.merge_number;
         _remove_merged_elements(this.xmldoc(), mnum);
      }
   };

   _base.prototype.merge_number = function()
   {
      return _get_property(this,"host","data","merge_number") || 0;
   };
   
   _base.prototype.pre_transform = function() { };
   _base.prototype.post_transform = function() { };
   _base.prototype.initialize = function() { };

   _base.prototype.child_ready = function(child) { };

   _base.prototype.preview_result = function(returned_doc) { };

   _base.prototype.child_finished = function(child, cancelled)
   {
      if (!cancelled)
         // this.update_associations(child);
         this.update_links(child);

      child.remove_merged_results();
      child.sfw_close(); 
   };

   _base.prototype.update_contents = function(newdoc,type,child) {SFW.alert("missing update_contents()");};

   /**
    * It's a bit confusing, but *this* is the object that must be updated,
    * and the *child* is the object that is requesting the update.  In the
    * basic case, *this* is a table object, *child* is a form object that
    * was spawned by the table to do an operation.  This *child* object holds
    * information about the context of the *this* when the *child* was spawned, 
    * in particular, the selected HTML table row, context XML row, and a
    * scroll position.  The *this* object that spawned the *child* adds the
    * data to the *child* and should therefore be prepared for and satisfied
    * with the data the *child* preserves.
    *
    * The function cascades, in the for each *this*, it first looks for
    * a caller that spawned *this*, giving the originator a change to update.
    * For each caller that updates, the *child* must be the *this*, which
    * was the result of the spawn of the caller and thus preserves data the
    * caller needs to recall the specifics saved during the spawn.
    *
    * The *type* parameter is passed down to all spawned updates because it
    * shares the context of the server's response.  All levels of a cascade
    * need to know if the operation is an add, delete, or update operation.
    *
    * See also: closure function process_replace() in function _run_task().
    *           It's likely that process_replace() could be replacedd with
    *           cascade_updates().
    */
   _base.prototype.cascade_updates = function(newdoc, type, child)
   {
      var caller = this.caller();
      if (caller)
         caller.cascade_updates(newdoc,type,this);

      this.update_contents(newdoc,type, child);
   };

   _base.prototype.restart = function(child) { };

   _base.prototype.dismantle = function()
   {
      this.remove_merged_results();
      this.sfw_close();
   };

   _base.prototype.update_field_association = function(xrow, child)
   {
      function f(n) { return n.nodeType==1 && n.getAttribute("data-sfw-assoc"); }
      var assoc_hosts = SFW.find_child_matches(this.host(), f, false, true);
      var schema = this.schema();
      if (assoc_hosts)
      {
         for (var i=0,stop=assoc_hosts.length; i<stop; ++i)
         {
            var host = assoc_hosts[i];
            var fname = host.getAttribute("data-sfw-assoc");
            var field = schema.selectSingleNode("field[@name='" + fname + "']");
            if (field)
            {
               field.setAttribute("data-id", xrow.getAttribute("id"));
               SFW.xslobj.transformFill(host, field);
               field.removeAttribute("data-id");
            }
         }
      }
   };

   _base.prototype.update_hosted_associations = function()
   {
      var schema = this.schema();
      var field, fname, fields = schema.selectNodes("field[@type='assoc']");
      var fcount = fields.length;

      function f(n)
      {
         var aname;
         if (n.nodeType==1 && (aname=n.getAttribute("data-sfw-assoc")))
         {
            for (var i=0; i<fcount; ++i)
            {
               field = fields[i];
               fname = field.name;
               if (aname == fname)
               {
                     // A "td" suggests a "tr" host that should have a data-id attribute:
                     if (n.tagName.toLowerCase()=="td")
                     {
                        var id = n.parentNode.getAttribute("data-id");
                        if (id)
                           field.setAttribute("data-id", id);
                        else
                        {
                           SFW.alert("update_associations failed to find a data-id"
                                     + " attribute for \"" + fname+".\"");
                           field.removeAttribute("data-id");
                        }
                     }

                     SFW.xslobj.transformFill(n, field);
               }
            }
         }
      }

      if (fcount>0)
         SFW.find_child_matches(this.host(), f, false, true);

      // Clean-up after last data-id is used:
      for (var i=0; i<fcount; ++i)
         fields[i].removeAttribute("data-id");
   };


   _base.prototype.update_associations = function(child)
   {
      var fu = _get_property(this,"caller","update_associations");
      if (fu)
         fu();

      var xrow = _get_property(this,"host", "data","xrow");
      if (xrow)
         this.update_field_association(xrow, child);
      else
         this.update_hosted_associations();
   };

   _base.prototype.update_field_links = function(data)
   {
      if ("replace_row" in this)
      {
         var xrow = data.xrow;
         var urow = data.target;
         if (xrow && urow)
            this.replace_row(urow, xrow);
      }
   };

   _base.prototype.update_hosted_links = function()
   {
   };


   _base.prototype.update_links = function(child)
   {
      var ul = _get_property(this,"caller","update_links");
      if (ul)
         ul();

      var data = _get_property(this,"host","data");
      if (data && "xrow" in data)
         this.update_field_links(data, child);
      else
         this.update_hosted_links();
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
      var xpath = "/*/*[@merge-type][1]/@merge-type";
      var attr, name, view;
      if (((attr=doc.selectSingleNode(xpath)) && (name=attr.value))
          || (name=doc.documentElement.getAttribute("mode-type")))
         return _get_class_from_name(name);
      else if ((view = _seek_current_view(doc)))
         return view.getAttribute("type");
      else
         return null;
   }

   function _create_custom_form(node, caption, caller, data)
   {
      if (!caption)
         caption = "Dialog";

      var newhost = SFW.make_sfw_host(SFW.stage, node.ownerDocument, caller, data);
      SFW.size_to_cover(newhost, caller);

      // build form
      var form = addEl("form", newhost);
      form.setAttribute("class", "Moveable");
      form.setAttribute("data-sfw-class", "form-view");
      var fset = addEl("fieldset", form);
      fset.setAttribute("class", "Schema");
      var legend = addEl("legend", fset);
      legend.setAttribute("class", "Moveable");
      addText(caption, legend);
      var target= addEl("div", fset);
      var b_row = addEl("p", fset);
      b_row.setAttribute("class", "buttons");
      var button = addEl("input", b_row);
      button.setAttribute("type", "button");
      button.setAttribute("data-type", "close");
      button.setAttribute("value", "Close");

      SFW.xslobj.transformFill(target, node);
      
      var anchor = SFW.seek_child_anchor(newhost);
      if (anchor)
         SFW.arrange_in_host(newhost, anchor);
   }

   function _create_interaction(node, caller, data)
   {
      var newhost = SFW.make_sfw_host(SFW.stage, node.ownerDocument, caller, data);
      SFW.size_to_cover(newhost, caller);

      SFW.xslobj.transformFill(newhost, node);

      var obj = SFW.get_object_from_host(newhost);
      if (obj)
      {
         obj.initialize();
         if (caller)
            caller.child_ready(obj);
      }

      var anchor = SFW.seek_child_anchor(newhost);
      if (anchor)
         SFW.arrange_in_host(newhost, anchor);
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

         if (lhost.children.length==0)
         {
            host.removeChild(lhost);
         }
         else
         {
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

   var xdocs_preload = "xmldocs_loaded";

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

      SFW.delete_preload(xdocs_preload);
   }

   SFW.add_preload(xdocs_preload);

   getXMLDocs(xmldocs_ready);
}
