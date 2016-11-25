// Closure to setup dpicker, host singletons
function process_dpicker(e,t)
{
   var c_cur_input = null;
   var c_cur_calhost = null;

   if (!t)
      return true;

   // Use parts of today's date as replacements for missing date parts:
   var tdate = new Date();
   var c_rday = tdate.getDate();
   var c_rmon = tdate.getMonth()+1;
   var c_ryear = tdate.getFullYear();

   // make_calendar closure function
   var get_calendar_selected_td = null;
   

   // HTML-DOM-related stuff, prepare functions:
   // 
   // If not hosted with XML.js, implement the following:
   if (!("addEl" in window))
   {
      window.addEl = function(tag,parent,before)
      {
         var n = c_doc.createElement(tag);
         if (parent)
            parent.insertBefore(n,before||null);
         return n;
      };
      window.addText = function(text,el)
      {
         el.appendChild(c_doc.createTextNode(text));
      };
   }


   function z(n) { return String(n+100).substr(1); }
   function a(host, tag) { return addEl(tag,host); }
   function addtxt(host, str) { addText(str,host); }
   function a_td(host,text,clsname)
   {
      var td=a(host,"td");
      if (clsname)
         td.className = clsname;
      addtxt(td,text);
      return td;
   }

   function ditch_context()
   {
      // Save value, remove calendar, undo closure variables:
      parse_typed(c_cur_input.value);
      c_cur_input.value = get_parsed_as_iso();
      c_cur_input = null;
      
      var p = c_cur_calhost.parentNode;
      p.removeChild(c_cur_calhost);
      c_cur_calhost = null;
   }

   function establish_context(t)
   {
      parse_transmitted(t.value);
      t.value = get_parsed_as_value();

      var c = a(t.parentNode,"div");
      c.className = "dpicker_host";
      make_calendar(c);
      move_cal(c,t);
      c.style.display = "block";
      
      c_cur_input = t;
      c_cur_calhost = c;
   }

   function make_calendar(host)
   {
      var lenday = 24*60*60*1000;
      var lenweek = lenday * 7;
      
      var c_today = new Date();
      var c_selected = null;
      var c_cal_selected = null;
      var c_mon_start, c_mon_end;

      get_calendar_selected_td = function() { return c_cal_selected; };

      function date_as_iso(date)
      {
         return String(date.getFullYear())
            +'-'+z(date.getMonth()+1)
            +'-'+z(date.getDate());
      }

      function dmatch(lh,rh)
      {
         if (!rh || !lh)
            return false;
         
         if (lh.getDate()!=rh.getDate())
            return false;
         if (lh.getMonth()!=rh.getMonth())
            return false;
         if (lh.getFullYear()!=rh.getFullYear())
            return false;

         return true;
      }

      function is_today(d) { return dmatch(d,c_today); }
      function is_selected(d) { return dmatch(d, c_selected); }
      function is_in_month(ms) { return ms > c_mon_start && ms < c_mon_end; }
      
      function td_class(ms,d)
      {
         var c = "target";
         if (!is_in_month(ms)) c += " out_month";
         if (is_today(d))      c += " today";
         if (is_selected(d))   c += " select";
         
         return c;
      }

      // make a week
      function week(tr, t)
      {
         var limit = t+lenweek;
         while (t<limit)
         {
            var d = new Date(t);
            var td = a_td(tr,d.getDate());
            td.className = td_class(t,d);
            td.setAttribute("data-iso8601", date_as_iso(d));

            if (is_selected(d))
               c_cal_selected = td;

            t += lenday;
         }
      }

      function inforow(host, text)
      {
         var tr = a(host, "tr");
         var td = a_td(tr, text);
         td.setAttribute("colspan","7");
         td.className = "info";
      }

      var mnames = ["Jan","Feb","Mar","Apr","May","Jun",
                    "Jul","Aug","Sep","Oct","Nov","Dev"];

      function autocomprow(host)
      {
         var dobj = get_deduced_date_parts();
         var str = mnames[dobj.mon]+'-'+z(dobj.day)+'-'+dobj.year;
         inforow(host,str);
      }
      
      var jmpcls = "target center";
      function headrow(host, year, mon)
      {
         var td,tr = a(host,"tr");
         td = a_td(tr,"<<",jmpcls);
         td.setAttribute("data-replot","-10");
         td = a_td(tr,"<","target center", jmpcls);
         td.setAttribute("data-replot","-1");
         
         td = a_td(tr, mnames[mon]+" '"+z(year%100));
         td.setAttribute("colspan","3");
         td.className = "name";
         
         td = a_td(tr,">",jmpcls);
         td.setAttribute("data-replot","1");
         td = a_td(tr,">>",jmpcls);
         td.setAttribute("data-replot","10");
      }

      var dnames = ["su","mo","tu","we","th","fr","sa"];
      function weekdayrow(host)
      {
         var td, tr = a(host,"tr");
         for (var i=0; i<7; ++i)
         {
            td = a_td(tr,dnames[i]);
            td.className = "dhead";
         }
      }

      function get_month_last_second(year, mon)
      {
         var monplus = (mon+1)%12;
         var yearplus = year + ((monplus>mon)?0:1);
         return (new Date(yearplus, monplus,1)).getTime()-1;
      }

      // replaces make_calendar() at end of closure:
      function f(host)
      {
         var dobj = get_deduced_date_parts();

         c_cal_selected = null;
         c_selected = new Date(dobj.year, dobj.mon, dobj.day);
         
         var date_ref = new Date(dobj.year,dobj.mon,1);

         // Get range of seconds in month
         c_mon_start = date_ref.getTime();
         c_mon_end = get_month_last_second(dobj.year, dobj.mon);

         // Get range of seconds of weeks that include the month,
         // from Sunday before first day of month to end of
         // Saturday after last day of month:
         var ms_cur = c_mon_start + lenday/2 - date_ref.getDay()*lenday;
         var ms_limit = c_mon_end + (6-(new Date(c_mon_end)).getDay()) * lenday;

         host.innerHTML = "";
         var table = a(host, "table");
         table.className = "mycal";
         var tbody = a(table, "tbody");

         //      inforow(tbody,date_as_iso(c_selected));
         autocomprow(tbody);
         headrow(tbody,dobj.year,dobj.mon);
         weekdayrow(tbody);
         while (ms_cur < ms_limit)
         {
            var tr = a(tbody, "tr");
            week(tr, ms_cur);
            ms_cur += lenweek;
         }
      }
      
      make_calendar = f;
      f(host);
   }

   var c_pday, c_pmon, c_pyear, c_pdash1, c_pdash2;
   function clear_parsed() { c_pday=c_pmon=c_pyear=c_pdash1=c_pdash2=null; }

   function get_parsed_as_value()
   {
      if (!c_pdash1)
         return c_pday;
      else if (!c_pmon)
         return z(c_pday)+'-';
      else if (c_pyear)
         return z(c_pmon)+'-'+z(c_pday)+'-'+c_pyear;
      else if (c_pdash2)
         return z(c_pmon)+'-'+z(c_pday)+'-';
      else
         return z(c_pmon)+'-'+c_pday;
   };

   function get_parsed_as_iso()
   {
      if (c_pyear)
      {
         var y = c_pyear<30?c_pyear+2000: c_pyear<100 ? c_pyear+1900 : c_pyear;
         return String(y)+'-'+z(c_pmon)+'-'+z(c_pday);
      }
      else
         return "";
   }

   function get_parsed_year()
   {
      if (c_pyear<30)     return c_pyear+2000;
      if (c_pyear>100)    return c_pyear;
      return c_pyear + 1900;
   }

   function get_deduced_year()
   {
      if (c_pyear===null) return c_ryear;
      return get_parsed_year();
   }

   // Returns 0-based month, matching Date():
   function get_deduced_date_parts()
   {
      return {year : get_deduced_year(),
              mon : (c_pmon || c_rmon)-1,
              day : c_pday || c_rday };
   };

   function iso_set_parsed_values(iso)
   {
      c_pdash1 = c_pdash2 = true;
      c_pyear = Number(iso.substr(0,4));
      c_pmon = Number(iso.substr(5,2));
      c_pday = Number(iso.substr(8,2));
   };

   function shift_parsed_values(dir)
   {
      // Get selected date in case field is empty:
      var td = get_calendar_selected_td();
      if (td)
         iso_set_parsed_values(td.getAttribute("data-iso8601"));
      else
         return;
      
      var jump = Number(dir);
      var ajump = Math.abs(jump);
      
      // Treat c_pmon as 0-based index
      if (ajump==1)
      {
         var savedmon = c_pmon;
         if (jump>0)
         {
            c_pmon %= 12;
            if (c_pmon<savedmon)
               ++c_pyear;
         }
         else
         {
            c_pmon = (c_pmon+10)%12;
            if (c_pmon>savedmon)
               --c_pyear;
         }

         // Restore c_pmon to 1-based index
         ++c_pmon;
      }
      else if (ajump==10)
      {
         // fix leapyear:
         if (c_pmon==2 && c_pday==29)
            c_pday=28;
         if (jump>0)
            ++c_pyear;
         else
            --c_pyear;
      }
   };

   function ymd_set_parsed_values(y,m,d)
   {
      c_pdash1 = c_pdash2 = true;
      c_pyear = y;
      c_pmon = m;
      c_pday =d;
   }

   // positioning functions
   function px(n) { return String(n)+"px"; }
   function win_pos(e)
   {
      var rval = {top:e.offsetTop, left:e.offsetLeft};
      
      var p = e.offsetParent;
      if (p && p.nodeType<9)
      {
         var r = win_pos(p);
         rval.top+=r.top;
         rval.left+=r.left;
      }
      return rval;
   }

   function move_cal(host,t)
   {
      var pos = win_pos(t);
      var high = t.offsetHeight;
      host.style.top = px(pos.top + high);
      host.style.left = px(pos.left);
   }
   
   function parse_transmitted(s)
   {
      var re = /(\d{4})-(\d\d)-(\d\d)/;
      function f(s)
      {
         clear_parsed();
         var m = re.exec(s);
         if (m)
         {
            c_pyear = Number(m[1]);
            c_pmon = Number(m[2]);
            c_pday = Number(m[3]);
            c_pdash1=c_pdash2=true;
         }
      }

      parse_transmitted = f;
      f(s);
   }

   
   var monmax = [31,29,31,30,31,30,31,31,30,31,30,31];
   function fix_parsed()
   {
      var max;
      if (c_pdash1) // month + date
         max = monmax[c_pmon-1];
      else          // day only
         max = monmax[c_rmon-1];

      if (c_pday>max)
         c_pday = max;
   }

   function parse_typed(s)
   {
      var arr_re = [
         "(([0123]\\d)|(\\d))",
         "(",
            "(\\/|-)",
            "(([0123]\\d)|(\\d))?",
            "(",
               "(\\/|-)",
               "(\\d{1,4})?",
            ")?",
         ")?"
      ];
      var re = new RegExp(arr_re.join(""));
      
      function f(s)
      {
         clear_parsed();
         
         var m = re.exec(s);
         if (m)
         {
            c_pdash1 = !!m[5];
            c_pdash2 = !!m[10];
            if (m[6])
            {
               c_pday = Number(m[6]);
               c_pmon = Number(m[1]);
               
               if (m[11])
                  c_pyear = Number(m[11]);
               else if (m[10])
                  c_pyear = null;
            }
            else
               c_pday = Number(m[1]);

            fix_parsed();
         }
      }
      
      parse_typed = f;
      return f(s);
   }

   function get_key(e)
   {
      if ("keyCode" in e)
         get_key = function(e) { return e.keyCode; };
      else if ("which" in e)
         get_key = function(e) { return e.which; };
      return get_key(e);
   }

   function in_calendar(t)
   {
      if (t.nodeType>3)
         return false;
      else if (t.className=="dpicker_host")
         return t;
      else
         return in_calendar(t.parentNode);
   }

   function _focus(e,t)
   {
      if (t==c_cur_input)
      {
      }
      else if (c_cur_calhost==in_calendar(t))
      {
      }
      else  // not in previous picker
      {
         if (c_cur_input)
            ditch_context();

         // If new focus is dpicker, make a new context:
         if (t.className=="dpicker")
            establish_context(t);
      }

      return true;
   }

   function _keyup(e,t)
   {
      parse_typed(t.value);
      t.value = get_parsed_as_value();
      make_calendar(c_cur_calhost);
      return true;
   }

   function process_calendar(e,t)
   {
      var cal = null;
      function finish()
      {
         make_calendar(c_cur_calhost);
         c_cur_input.value = get_parsed_as_value();
         c_cur_input.focus();
         return false;
      }

      if (e.type=="click" && (cal=in_calendar(t)))
      {
         var val;
         if ((val=t.getAttribute("data-iso8601")))
         {
            iso_set_parsed_values(val);
            return finish();
         }
         else if ((val=t.getAttribute("data-replot")))
         {
            shift_parsed_values(val);
            return finish();
         }
      }
      
      return true;
   }
   
   // Message handler, replacing the global run_dpicker function:
   function f(e,t)
   {
      if (!process_calendar(e,t))
         return false;

      // Process every focus
      if (e.type=="focus") return _focus(e,t);
      
      if (t.className=="dpicker")
      {
         switch(e.type)
         {
         case "keyup": return _keyup(e,t);
         default: break;
         }
      }
      
      return true;
   }

   // Install an execute the replacement function
   process_dpicker = f;
   return f(e,t);
}

