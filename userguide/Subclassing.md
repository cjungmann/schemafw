# Subclassing

There are two interaction types, the **table** and the **form**.  The
framework also includes derived interaction types **calendar** and **form_view**.
The derived interactions.

A class must defined before it can serve as a base class for a derived class.
This [module dependency article](DependentModuleLoad.md] describes how to use the
SchemaFW method and how it works.

## Make a derived class

A derived class is created when the framework copies the prototype members of a base
class to the prototype object of new object constructor.  The derived class then
adds to or replaces its prototype values according to its unique mission.

Use **SFW.derive** function to initialize the prototype and register the name.

~~~{js}
SFW.derive =
function derive(constructor,     // Object constructor function
                new_class_name,  // string value 
                base_class_name, // string value
                override         // boolean, replace existing new_class_name if found.
               )
~~~

SFW.derive() returns false if *base_class_name* is not registered, or if
*new_class_name* is already registered.  If *override* is set to *true* and
*new_class_name" is already registered, the new *constructor* will replace the
existing iclass.


Except of a module based on contents of this page.
~~~{js}

// sfw_calendar.js

(function _init()
{
   // Wait if undefined base class
   if (SFW.delay_init("sfw_calendar.js",_init,"table"))
      return;

   // Register new class, skip if already registered
   if (!SFW.derive(_demo, "demo", "table"))
      return;

   // Constructor
   function _demo(base, doc, caller, data)
   {
      // Begin Initialization with base class constructor
      SFW.types["table"].call(this,base,doc,caller,data);
   }

   // Continue to define _demo prototypes and supporting local functions
      
})();

~~