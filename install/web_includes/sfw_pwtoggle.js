
// sfw_password.js

(function _init()
{
   var bclass="iclass";
   if ((!("SFW" in window) && setTimeout(_init,100))
       || SFW.delay_init("sfw_pwtoggle", _init, bclass))
      return;

   if (!SFW.derive(_pwtoggle, "pwtoggle", bclass))
      return;

   function _pwtoggle(actors) { SFW.base.call(this,actors); }

   _pwtoggle.prototype.process = function(e,t)
   {
      if (e.type=="click")
      {
         this.toggle(t);
         return false;
      }
      return true;
   };

   _pwtoggle.prototype.seek_mate = function(t)
   {
      var parent = t.parentNode;
      if (parent)
         return parent.getElementsByTagName("input")[0];
      return null;
   };

   _pwtoggle.prototype.toggle = function(t)
   {
      var mate = this.seek_mate(t);
      if (mate)
      {
         var state = mate.type;
         if (state=="password")
         {
            mate.type = "input";
            t.src = "includes/pwhide.png";
         }
         else
         {
            mate.type = "password";
            t.src = "includes/pwshow.png";
         }
      }
   };

})();


