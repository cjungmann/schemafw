# Custom Interactive Classes

Interactive classes, AKA _iclasses_, animate views constructed from XML resultsets
using XSL.

The fundamental iclasses, _table_ and _form_, handle multiple or single records,
respectively.  These classes work on their own, but have been designed to be
extended using Javascript prototype inheritance.

## Overview

There are several steps to implementing a custom interaction.

(See the [Definitions](Definitions.md) page for an explanation of the following
terms, **class**, **object**, **iclass**, and **iobject**, used on this page.)

### The Basic Interaction

The typical SchemaFW interaction consists of the following events:

1. User sends http request to SchemaFW server.
2. SchemaFW returns an XML document based on the indicated _response mode_.
3. The browser renders the XML using the *xml-stylesheet* processing instruction
   included in a SchemaFW _resultset_
   a. Client-side software opens the xslt stylesheet, reconciles imports.
   b. The _resultset_ document is submitted to the transformation processor
      to render the page.
   c. The stylesheet renders that page according to the content and optional
      included hints to generate tables or forms.  **The is the step where
      customization begins.**
4. The client-side framework _animates_ the page by activating a message
   processing Javascript function.  **Custom interactions will provide custom
   message-processing functions.**
5. The browser waits for user input.   

### Custom Interaction

A custom interaction will typically include several parts, though not all are
necessary.

1. Custom XSL templates to render the data as HTML.
   - The custom template should match the name of an SFW result element that
     has been renamed in the response mode.
   - The custom template must identify the _iclass_ that will animate the HTML.
   - Consult [Templates for Custom Interactions](CustomTemplates.md) for a discussion
     of the issues around ensuring your custome templates are recognized.
2. A custom IClass to _animate_ the HTML.  The custom class should be ultimately
   derived from "SFW.iclass", but that is likely to be through intermediate classes.

### Custom IClass

Prototype methods to be handled by an *iobject* instantiated from an *iclass*.  In
order of importance rather than invocation.
1. Method **process**: to which all windows messages will be sent.  The base-class's
   *process* method will be used if *process* is not defined for the custom iclass.
2. Method **child_finished** is called when a child interaction is transferring
   control back to the custom iclass.  This is **the opportunity to update data
   based on the outcome of the child interaction**. The default *child_finished*
   method closes the child interaction, and a custom *child_finished* method should
   either invoke its base-class *child_finished* or explicitely close te child.
3. Method **child_ready** is called when the XSL transformation is complete
   and the framework.  This is an opportunity to do **final preparation** or to
   **stash some data** for the eventual replot.
4. Method **pre_transform** will be called just before an XSL transformation
   for an **opportunity to set flags in the resultset or XSL file** for additional
   control over the output.

The **child_ready** and **pre_transform** procedures often work together.
**please fill this out when you have a prototype.**




### Packaging the IClass

The new iclass must be enclosed in a self-running function with precedent-enforcing
code to ensure the base class is defined before defining the derived iclass.  Refer
to the [Dependent Module](DependentModuleLoad.md) and [Subclassing](Subclassing.md)
pages for explanations of *delay_init* and *derive* functions in the sample below.

~~~{js}
(function _init() {
   // Wait for "table" class to be defined
   if (SFW.delay_init("idemo_js", _init, "table"))
      return;

   if (!SFW.derive(_demo, "idemo", "table"))
      return;
      
   function _demo(base,doc,caller,data)
   {
      SFW.types["table"].call(this, base,doc,caller,data);
   }

   // Add or replace prototypes after this.

   // Define this function to make temporary changes before a transformation.
   _demo.prototype.pre_transform = function()
   {
   };

   // Define this function to undo changes by pre_transform after the transformation.
   _demo.prototype.post_transform = function()
   {
   };

   // Define this function if steps are required after an object is up-and-running.
   _demo.prototype.child_ready = function(child)
   {
   };

   // Called by the framework when a child is about to be closed.
   // This is the chance to harvest info from the child and restore
   // the parent to operational mode, like default table that is cleared
   // when a dialog is opened and restored when the child is done.
   _demo.prototype.child_finished = function(child, cmd)
   {
      this.replot();
      SFW.base.prototype.child_finished.call(this,child,cmd);
   };

   // Define to handle custom events, passing unhandled events to the base class.
   _demo.prototype.process = function(e,t)
   {
      return this.call_super_event("table","process",arguments);
   };

   // For calendar-derived classes
   _demo.prototype.process_day_click = function(target_td, dayid)
   {
   };

})();
~~~