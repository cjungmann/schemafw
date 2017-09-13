
function classes_implement()
{
   // Fix IE5 missing push method:
   if (!Array.prototype.push)
   {
	   Array.prototype.push = function(v)
      {
         var ndx=this.length;
         this.length=ndx+1;
         this[ndx]=v;
         return this.length;
      };
   }
   // Fix IE<9 missing indexOf method:
   if (!Array.prototype.indexOf)
   {
      Array.prototype.indexOf = function(v)
      {
         for (var i=0,stop=this.length; i<stop; i++)
            if (this[i]==v)
               return i;
         return -1;
      };
   }

   function c_re(name) { return new RegExp('(^|\\s)' + name + '(\\s|$)'); }
   function spl(el) { return el?el.className.split(' '):[]; }
   
   function c_includes(el, name)
   {
      if (name && name.length>0)
      {
         var arr = spl(el);
         return arr.indexOf(name)!=-1;
      }
      return false;
   }

   function c_add(el, name)
   {
      if (name && name.length>0)
      {
         var arr = spl(el);
         if (arr.indexOf(name)==-1)
         {
            arr.push(name);
            el.className = arr.join(' ');
            return true;
         }
      }
      return false;
   }

   function c_remove(el, name)
   {
      if (name && name.length>0)
      {
         var i, arr = spl(el);
         if ((i=arr.indexOf(name))!=-1)
         {
            arr.splice(i,1);
            el.className = arr.join(' ');
            return true;
         }
      }
      return false;
   }

   class_add = c_add;
   class_remove = c_remove;
   class_includes = c_includes;
}

function class_includes(){}
function class_add(){}
function class_remove(){}

classes_implement();
