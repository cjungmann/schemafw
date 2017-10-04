# Minimum Custom Class

This page, along with [Custom Interactive Classes](CustomIClasses.md) explain how to
create custom classes, how they work, and the conventions to follow for successful
custom classes.

To create a new class, especially an input form widget, there are a few steps that must be
taken to ensure the new widget is loaded and recognized.

## Steps for Creating a New Class

### Create a Template to Build The HTML.

This example assumes that there will be a new XSL stylesheet for the new class.

To create the new form widget, it is necessary to create a template of mode **construct_input**
that matches a specific field.  Notice in the stylesheet fragment below how the template matches
a field of type *newtype*:

Example file *newtype.xsl*:
~~~xsl
<?xml version="1.0" encoding="UTF-8" ?>
<xsl:stylesheet
   version="1.0"
   xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
   xmlns="http://www.w3.org/1999/xhtml"
   xmlns:html="http://www.w3.org/1999/xhtml"
   exclude-result-prefixes="html">

    <xsl:output method="xml"
         doctype-public="-//W3C//DTD XHTML 1.0 Strict//EN"
         doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"
         version="1.0"
         indent="yes"
         omit-xml-declaration="yes"
         encoding="UTF-8"/>

    <xsl:template match="field[@type='newtype']" mode="construct_input">
       <xsl:param name="data" />

       <xsl:element name="div">
          <xsl:attribute name="class">newtype</xsl:attribute>
          <xsl:attribute name="data-sfw-class">newtype</xsl:attribute>
          <xsl:attribute name="data-sfw-input">input</xsl:attribute>
          .
          .
       </xsl:element>


       .
       .
       .
    </xsl:template>

</xsl:stylesheet>
~~~

Within the *construct_input* template, you will add the parts necessary to build the
control.  That might mean a *table*, with *tr* and *td* elements, or a *ul* with
*li* elements.  They can be in the template, or called from the template.

In the example above, a *div* element is created.  For flexibility in adding attributes,
It is better to use *xsl:element* rather than the element itself.

The three attributes of the example *div* element each contribute to the proper handling
of the class:

- **class="newtype"** is included to signal to a CSS stylesheet how to display the
  class.
- **data-sfw-class="newtype"** This attribute identifies the HTML element that defines
  it as the base of the custom class.  The value of the attribute is matched to the
  name of a class in the **SFW.types** array (see below).
- **data-sfw-input="input"** flags the element as an input element that is likely to be
  hosted by another *data-sfw-class* element.  This attribute is important to prevent
  the *data-sfw-class* attribute of the hosting element from overwriting the class
  name of the custom input class as the framework scans through its ancestors.

As an example of the possible need for additional attributes, and as justification for the
more verbose *xsl:element* construction, you might want to add the following:

~~~xsl
          <xsl:apply-templates select="." mode="add_on_line_click_attribute" />
~~~

to add the value of an `on_line_click` SRM instruction to the anchor element in order to use
`tbase.prototype.get_line_click_info()`.

### Create a Javascript Module

The new class must be registered into the *SFW.types* array.  This is done with the command
`SFW.derive()` as described below.

The first line in the following *_init()* function will delay until the **SFW** object has
been created, and further, if necessary, to ensure that the base class of the new widget has
been registered.

The second line registers the new element, copies the prototypes of the base class, *tbase* in
this example, to the new class, adding or overriding prototypes with the new ones defined in
the *init()* function.

The third command is the new class constructor.  Notice how it calls the base constructor
to ensure basic Schema Framework setup is completed.

The fourth command is a sample prototype.  The example shown would not be necessary because
the *tbase* *process()* prototoype would be called automatically if this *process() function
was not defined.  Notice how this function calls the parent class using the *SFW.types*
array.

~~~js
// newtype.js
(function _init()
{
   if ((!("SFW" in window) && setTimeout(_init,100))
       || SFW.delay_init("sfw_newtype", _init, "tbase"))
      return;

   if (!SFW.derive(_newtype, "newtype", "tbase"))
      return;

   function _newtype(actors)
   {
      SFW.types["tbase"].call(this,actors);
   }

   _newtype.prototype.process = function(e,t)
   {
      SFW.types["tbase"].prototype.process.call(this,e,t);
   }
  
})();
~~~

### CSS Stylesheet

There will be no examples here, but most projects will need to consider the need for
CSS rules to ensure the custom input looks and runs properly.

### Ensure the New Scripts are Loaded.

There are two ways to do this.  For objects that will be part of the framework, update
files *sfw_scripts.xsl* and *sfw_debug.xsl*.  For one-off customization, add the references
to the *default.xsl* file.

#### Option 1: Add to Schema Framework

Add the new Javascript file to *sfw_scripts.xsl*.  The variable **jslist_sfw_debug** holds the
list of Javascript file names (without the *.js* extension).  Simply add the new script file name
to the end of the list.

~~~xsl
<xsl:stylesheet version="1.0" ...>
  .
  .
  <xsl:variable name="jslist_sfw_brief">sfw</xsl:variable>
  <xsl:variable name="jslist_sfw_minified">sfw.min</xsl:variable>
  <xsl:variable name="jslist_sfw_debug">sfw_0 sfw_tbase sfw_calendar sfw_debug sfw_dom sfw_form sfw_ulselect sfw_mixed_view sfw_onload sfw_table sfw_iltable</xsl:variable>
  
  <!-- Set jslist_sfw to one of the above variables for the desired script-set. -->
  <xsl:variable name="jslist_sfw" select="$jslist_sfw_debug" /> 
  .
</xsl:stylesheet>  
~~~

Note that when running `sudo make update-client`, a minimized Javascript file will be made from
the concatenated contents of all the *sfw*-prefixed, *js* extension files in the directory.  The
production version of your application should define the variable *jslist_sfw* to be
*$jslist_sfw_minified* to load the minimized version.  Use the debug version for development
because it's easier to debug.

Add the new XSL stylesheet as an xsl:import element to the *sfw_debug.xsl* file.  Pay
attention to template matching preferences when deciding where in the list the new file
should be added.

#### Option 2: Add to *default.xsl*

For one-off customization, update the *default.xsl* as follows:

~~~xsl
<xsl:stylesheet version="1.0" ...>
  .
  <xsl:import href="includes/sfw_debug.xsl" />
  <!-- <xsl:import href="includes/sfw_compiled.xsl" /> -->

  <xsl:import href="newtype.xsl" />
  .
  <xsl:template match="/">
     <html>
        <head>
          <title>Your Title</title>
          <xsl:apply-templates select="*" mode="fill_head" />
          <script type="text/javascript" src="newtype.js"></script>
        </head>
     .
     .
  </xsl:template>
  -
  -
</xsl:stylesheet>  
~~~

Note the `<xsl:import href="newtype.xsl" />` that adds the new stylesheet to the project,
and, the the *<head>* section, the `<script type="text/javascript" src="newtype.js"></script>`
statement that adds the Javascript file.

Also note the top import statements.  By default, it uses the *sfw_debug.xsl* stylesheet that
makes it easier to debug the stylesheet with a tool like [Oxygen](http://www.oxygenxml.com).

## Notes and Recommendations

### Schema Framework Event Handling

When the environment sends an event to the framework, it loops from the initial event
target through its ancestors to the document object.  At each stop, it checks for attributes
that indicate a SFW class is available to handle the event.  If an appropriate class is
defined, it will be instantiated and used to handle the event.

### The Anchor HTML Element and the *top()* Function.

An HTML element that defines the *data-sfw-class* attribute is known as an **anchor**
element.  Objects created from the *data-sfw-class* value hold a reference to the anchor
element.  This element can be found with the SFW base class function **top()**.

### Class Object Lifetime

The SFW class object lives from the time it is identified until the return of the framework's
event handler.

### Save Data in the HTML Element Anchor

Since the class object is short-lived, it is generally not appropriate to save state
information in the class. The rare exceptions would be if it is constructed to be a singleton
object that persists between events, or if the values are attached to the *class* rather than
the *object* constructed from the class.

While the class object is new and naive, the anchor HTML element that prompts the object
creation remains.  Get the anchor element with the *top()* function and add a new attribute:

~~~js
_newtype.prototype.process = function(e,t)
{
   if (e.type=="click")
   {
      // Get saved information
      var saved_info = this.top().getAttribute("saved_info");

      var new_info = process_the_information(saved_info);

      this.top().setAttribute("saved_info", new_info);

      return false;
   }

   return true;
};
~~~

### Take Care with Replot

When the anchor element is used to save persistent data, it is important to preserve the
element.  When defining a replot() function, make sure that the replot does not disturb
the anchor.  The replot function should refill the element with new contents as appropriate.

