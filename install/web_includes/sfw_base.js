var SFW = { };

function init_SFW(callback)
{
   SFW.focus_on_first_field = _focus_on_first_field;
   SFW.get_form_data        = _get_form_data;
   SFW.seek_anchor          = _seek_anchor;
   SFW.derive               = _derive;
   SFW.add_event            = _add_event;
   SFW.setup_event_handling = _setup_event_handling;
   SFW.process_event        = _process_event;
   SFW.translate_url        = _translate_url;
   SFW.base                 = _base;

   SFW.px = function(num) { return String(num)+"px"; };

   function call_SFW_init_functions()
   {
      // Other inits need the above functions, so only call after they're installed:
      //   var inits = ["init_SFW_DOM", "init_SFW_Tables", "init_SFW_Debug"];
      var inits = ["init_SFW_DOM", "init_SFW_Tables"];
      for (f in inits)
      {
         var s = inits[f];
         if (s in window)
            window[s]();
      }
   }

   function _focus_on_first_field(dlg)
   {
      var nl = dlg.getElementsByTagName("label");
      if (nl && nl.length>0)
      {
         var label = nl[0];
         var name = label.getAttribute("for");
         function f(n) { return n.getAttribute("name")==name; }
         var el = getFirstMatchingEl(label.parentNode, f);
         if (el)
         {
            el.focus();
            if (el.getAttribute("type")=="text")
            {
               el.selectionStart = 0;
               el.selectionEnd = el.value.length;
            }
         }
      }
   }

   function _get_form_data(form)
   {
      var el, els = form.elements;
      var arr = [];
      var noninputs = 'submit reset button';
      for (var i=0, stop=els.length; i<stop; i++)
      {
         el = els[i];
         if (noninputs.search(el.type)==-1)
         {
            if (el.type=="checkbox")
            {
               if (el.checked)
                  arr.push(el.name + "=1");
            }
            else if ('value' in el && el.value.length>0)
               arr.push(el.name + "=" + encodeURIComponent(el.value));
         }
      }
      return arr;
   }

   function _seek_anchor(t)
   {
      var el = SFW.get_ancestor_anchor(t);
      if (el && el.sfwobj)
         return el;
      else
         return null;
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

   function _setup_event_handling()
   {
      function f(ev)
      {
         var e=ev||window.event;
         var t=e.target||e.srcElement;

         if (e.type=="resize")
            resize_table_headers();

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
      if ((el=_seek_anchor(t)) && (obj=el.sfwobj))
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

   function _find_schema(doc)
   {
      var schema = doc.selectSingleNode("*/schema");
      if (!schema)
         schema = doc.selectSingleNode("*/*[@rndx=1]/schema");

      return schema;
   }
   
   function _base(base,doc)
   {
      this._top = base;
      this._doc = doc;
      this._schema = _find_schema(doc);
      this._baseproto = null;
      this._super = null;
      base.sfwobj = this;
   };


   _base.prototype.top = function _top()       { return this._top; };
   _base.prototype.stage = function _stage()   { return this._top.parentNode; };
   _base.prototype.doc = function _doc()       { return this._doc; };
   _base.prototype.schema = function _schema() { return this._schema; };
   _base.prototype.baseproto = function _baseproto() { return this._baseproto; };

   _base.prototype.process = function _base_process(e,t)
   {
      return true;
   };

   // _base prototypes must be in place before this line:
   call_SFW_init_functions();

   // These may change if I redo XML.js.  They should all
   // accept and use a callback in case the documents are
   // not already resident.
   SFW.xmldoc = getXMLDocument();
   SFW.xsldoc = getXSLDocument();
   SFW.xslobj = new XSL(SFW.xsldoc);

   callback();
}
