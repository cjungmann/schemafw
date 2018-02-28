
// sfw_lookup.js

(function _init()
{
   if ((!("SFW" in window) && setTimeout(_init,100))
       || SFW.delay_init("sfw_lookup",_init,"table"))
      return;

   if (!SFW.derive(_lookup, "lookup", "table"))
      return;

   function _lookup(actors)
   {
      SFW.base.call(this,actors);
   }

   _lookup.prototype.top = function() { return this.widget(); };

   function field_name_from_host(host)
   {
      var label = host.parentNode.selectSingleNode("label");
      return label ? label.getAttribute("for") : "";
   };

   _lookup.prototype.replot = function(result)
   {
      var top = this.top();
      var host = top.parentNode;
      var xpath = top.getAttribute("data-path");
      var node = SFW.xmldoc.selectSingleNode(xpath);

      node.setAttribute("lookup-field-match", field_name_from_host(host));

      this.pre_transform();
      SFW.xslobj.transformFill(host, node);
      this.post_transform();

      node.removeAttribute("lookup-field-match");
   };


})();
