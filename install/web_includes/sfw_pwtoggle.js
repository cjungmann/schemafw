
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
      console.log("_pwtoggle, " + e.type);
      return true;
   };

})();


