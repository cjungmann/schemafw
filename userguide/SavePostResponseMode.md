# Response Mode Type _save-post_

The __save-post__ response mode type will save the full contents of stdin to a
file for later use in debugging.  This is particularly useful in two cases,
for long or complicated forms, and for multipart/form-data forms.  The response
mode should be simple, with only two instructions, and should not be left in
place for a production website.

## Save the Post

~~~srm
# excerpt of test.srm
# This working response mode can be replace with the following for _save-post_.
mp_form
   type   : form-submit
   procedure : App_Form_Save
   result
      type : update

# Use this response mode by changing the *form-action* value in the *form-edit* mode
mp_form_save
   type   : save-post
   target : /tmp/savedPost.txt
~~~

The target instruction provides a path where the post data can be saved.  This
mode can be targeted by using its URL in a form action.  The following example will
save the post of a file upload for later debugging.

~~~html
<form method="post"
      action="test.srm?mp_form"
      enctype="multipart/form-data"
      >
   <fieldset>
      <legend>Upload the file</legend>
      <p>
         <label for="pname">Enter your name</label>
         <input type="text" name="pname" />
      </p>
      <p>
         <label for="pfile">Select a file</label>
         <input type="file" name="pfile" />
      </p>
      <p>
         <input type="submit" value="Submit" />
      </p>
   </fieldset>
</form>
~~~

## Debug With Saved Post

After having generated the post file, the developer can test the response
with the following command line call, invoking the original form response mode:

~~~sh
$ schema.fcgi -s test.srm -m mp_form -i /tmp/savedPost.txt
~~~

If the _schema.fcgi_ server component is causing the error, debug the C++ code
thus:

~~~sh
$ gdbtui --args schema.fcgi -s test.srm -m mp_form -i /tmp/savedPost.txt
~~~


