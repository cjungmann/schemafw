function init_SFW_Debug()
{
   var reFuncParams = /^[^\(]+\(([^\)]*)\)/;

   SFW.get_properties_info = function(obj)
   {
      var arr = [];
      var type, func, params, prop;
      for (prop in obj)
      {
         if ((type=typeof(obj[prop]))=="function"
             && (func=obj[prop])
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
