# Showing XML Source With Framework Function

This method is used in Javascript while running a program.  See
[Showing XML Source with Template Output](XMLOutputTemplates.md) if the document
must be visible from a transform.

There will be times when a developer will need to study an XML document that
has been modified, and for some of those times, using the
[SchemaFW command line options](SchemaFCGIOptions.md) may be misleading.  The example
in mind when writing this guide is for [merged documents](MergingDocuments.md).

This is a simple construction, but hard to remember or find when needed:

~~~javascript
   SFW.show_string_in_pre(serialize(SFW.xmldoc));
~~~

This will write the XML document (or whateve node you serialize), to a new *pre*
element at the end of the HTML document.  From there, the document can be studied
or retrieved into the copy buffer to be copied to a text file for experimentation.