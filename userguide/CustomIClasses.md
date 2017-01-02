# Custom Interactive Classes

Interactive classes, AKA _iclasses_, animate views constructed from XML resultsets
using XSL.

The fundamental iclasses, _table_ and _form_, handle multiple or single records,
respectively.  These classes work on their own, but have been designed to be
extended using Javascript prototype inheritance.

## Definitions

<dl>
<dt>Class</dt>
<dd>
  A Javascript function with a defined prototype that includes methods and properties
  that are added to any object created using the class constructor.
</dd>
<dt>Object</dt>
<dd>
  An associated array, a list of named values.  Some of the named values may be
  functions, known as methods when applied to an object.
</dd>
<dt>IClass</dt>
<dd>
  A Javascript class designed to handle interactions in SchemaFW.
</dd>
<dt>IzObject</dt>
<dd>
  An object instantiated from an iclass.
</dd>
</dl>

An iclass must typically provide the follwing three methods:

- A **constructor** function that instantiates an object,
- a **process** method for handling events, and
- a **child_finished** method through which a child iclass object

## Creating a New IClass


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

})();
~~~