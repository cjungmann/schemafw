# Debugging Response Modes

In order to make debugging response modes easier, the web server program,
`schema.fcgi`, can also run on the command line.  There are various techniques
that can be used to see the result of requests.  Using `xsltproc` with the
output can also help debug XSL stylesheets.

Most of the examples on this page will not include _how_ the command line options
work, but only what to type for certain outputs.  For a more complete description,
see [Schema FCGI Server Command-line Options](SchemaFCGIOptions.md).

## Command Line Responses

The first examples will target the [L-CRUD Case Study](LCRUDInteractions.md).  To see
the examples working on your computer,

1. The Case Study is running on your computer. See [L-CRUD Case Study](LCRUDInteractions.md)

2. Your current working directory is the _./site_ directory.

   The SRM files must be in the current directory.  To debug the XSL, the applications's
   default XSL file must be available along with any include files it may reference.

### View the List Mode

To view the XML file response that is rendered into a list table:

~~~
schema.fcgi -s contacts.srm -m list
~~~

To see the HTML, pipe that response to `xsltproc`

~~~
schema.fcgi -s contacts.srm -m list | xsltproc default.xsl -
~~~

### Test a Form-submit Response

#### Simple Case

To submit data and see the response:

~~~
schema.fcgi -s contacts.srm -m edit_submit -v id=1 -v fname=James -v lname=Bond -v phone=555-555-1212
~~~

#### Saved Response

Sometimes, a form can be submitted only once.  An example of this would be a create
form, because the second time you call it, a unique key may prevent duplicate entries.
In that case, it may be best to save the output to a file, especially if you need to
debug the XSL stylesheet with the output.

~~~
schema.fcgi -s contacts.srm -m create_submit -v fname=Sydney -v lname=Bristow -v phone=555-555-2121 > /tmp/ctest.xml
~~~

For security, write the output outside of the _site_ directory, in case you forget to
delete the file later.

Now you can open _ctest.xml_ to look at the output, or transform it multiple times
if the XSL stylesheet is causing errors.

~~~
xsltproc default.xsl /tmp/ctest.xml
~~~

#### Cookie Request

For session-enabled sites, the dependence on cookies complicates matters.  It is not
convenient, but it is possible to fake cookies.  For a login-secured site, you might
follow these steps:

1. Login.  Note the two _Set-Cookie_ header lines.

   ~~~
   schema.fcgi -s email.srm -m login_submit -v email=jbond007@mi6.gov.uk -v pword1=Q pword2=Q

   Status: 200 OK
   Content-Type: text/xml
   Set-Cookie: SessionId=2
   Set-Cookie: SessionHash=sUXa2ajlVTdQiK6EvaWsFMGBJMZgn5ou

   <?xml version="1.0" ?>
   <?xml-stylesheet type="text/xsl" href="default.xsl" ?>
   <resultset post="true" mode-type="form-view">
      <result ndx="1">
         <row id="7" fname="james" lname="bond" email="jbond007@mi6.gov.uk" />
      </result>
   .
   .
   </resultset>
   ~~~

2. Change the Password: Set Cookie

   Use the console copy buffer (Shift-Ctrl-C to copy, Shift-Ctrl-V to paste) to
   create a cookie environment variable.  Pay attention to the semicolon that separates
   multiple values.
   
   ~~~
   export HTTP_COOKIE="SessionId=2; SessionHash=sUXa2ajlVTdQiK6EvaWsFMGBJMZgn5ou"
   schema.fcgi -s email.srm -m change_pword_submit -v id=7 -v pword=Q -v newpword1=SecurityBreached -v newpword2=SecurityBreached
   ~~~

   This is another case where it may be useful to save the output to a file, because
   the second time you call `schema.fcgi -m change_pword_submit ...`, the output
   should indicate a password failure.

   To delete the HTTP_COOKIE environment variable,

   ~~~
   export -n HTTP_COOKIE
   ~~~
