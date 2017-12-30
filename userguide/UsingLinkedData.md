# Using Linked Data

PREMATURE:
While working on this page, I decided that I need to rethink the field attributes used to
setup the data relationships.  I hope to return to this page when these issues are resolved.




There are cases where it is inefficient to store details of an object in the object itself.  An
example would be a person record 

## Field Indirection

A linked field refers to external data objects.  The external data objects are identified by
an integer index value and a hosting result element.  The framework uses CSV (comma-separated values)
notation to indicate the links:

~~~xml
   <school>
      <classes>
         <schema>
           <field name="name" type="VARCHAR" />
           <field name="instructor"
         </schema>
         <class name="Algebra" instructor="5" students="3,6,8,15,93,12" />
         ⋮
      </classes>

      <instructors>
      ⋮
      </instructors>

      <students>
         <student id="1" fname="Harriet" lname="Johnson" />
         <student id="2" fname="Caitlin" lname="Rogers" />
         <student id="3" fname="Samantha" lname="Smith" />
         <student id="4" fname="Connor" lname="Albertson" />
         <student id="5" fname="John" lname="Doe" />
         <student id="6" fname="Mark" lname="Hunt" />
         ⋮
      </students>
   </school>
~~~

