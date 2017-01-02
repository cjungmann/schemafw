# Dependant Module Load

SchemaFW is designed for customization.  It includes several interactive classes
that can be extended to animate custom interactions.

In order to ensure custom modules are loaded, and to enforce load order, the
framework provides a code recipe for loading custom modules contained in
self-running functions.

## The delay_init Function

This function provides the means for enforcing a load order on the self-running
functions based on checking for the definition of a prerequisite iclass.

~~~{js}
// Returns _true_ if a delay is necessary, _false_ otherwise
function delay_init(name_token,   // Unique string for tracking. Use script file name.
                    init_function // Function to call after delay, if necessary.
                    prereq_class  // Name of iclass type that must be registered to continue.
                    )
~~~

When delay_init() determines that a delay is necessary because *prereq_class*, a
string value, is not already registered, it saves the *name_token* string and
sets a timer to call the *init_function* again.  The following example shows how
to set this up.

~~~{js}
// file demo.js

(function _init() {
   if (SFW.delay_init("demo.js", _init, "iclass"))
      return;

   // Continue with module      
})();
~~~

See [Subclassing page](Subclassing.md) to add a custom iclass to the dependent
module.

See [Custom IClasses](CustomIClasses.md) for setting up and using custom iclasses.

Since the condition that results in a delay is a registered iclass type, make
sure that the named type can be registered.  Misspellings, forgotting to register
the class, or circular dependencies are among problems that might prevent a
module from loading.
