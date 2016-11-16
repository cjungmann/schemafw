function init_SFW_Debug()
{
   var reFuncParams = /^[^\(]+\(([^\)]*)\)/;

   SFW.get_properties_info = function()
   {
      var arr = [];
      var type, func, params;
      for (prop in SFW)
      {
         if ((type=typeof(SFW[prop]))=="function"
             && (func=SFW[prop])
             && (params = reFuncParams.exec(func.toString())))
         {
            arr.push(type + ": " + prop + "(" + params[1] + ")");
         }
         else
            arr.push(type + ": " + prop);
      }

      return arr;
   };
}
