function init_SW_Forms()
{
   SFW.form_class = _form;

   function _form(url, cb)
   {
      function got_doc(doc)
      {
         
      }

      xhr_get(url, got_doc);
   }

}
