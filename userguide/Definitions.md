# Definitions

<dl>
  <dt id="class">Class</dt>
  <dd>
    A Javascript function with a defined prototype that includes methods and properties
    that are added to any object created using the class constructor.
  </dd>
  <dt id="object">Object</dt>
  <dd>
    An associated array, a list of named values.  Some of the named values may be
    functions, known as methods when applied to an object.
  </dd>
  <dt id="iclass">IClass</dt>
  <dd>
    A Javascript class designed to handle interactions in SchemaFW.
  </dd>
  <dt id="iobject">IObject</dt>
  <dd>
    An object instantiated from an iclass.
  </dd>
  <dt id="dialog">dialog</dt>
  <dd>
    Any display that presents information to the user.
  </dd>
  <dt id="form">form</dt>
  <dd>
    A [dialog](#dialog) that accepts input from the user.
  </dd>
  <dt id="srm">SRM file</dt>
  <dd>
    A text file with instructions that SchemaFW follows to build an XML response.  SRM
    stands for **S**chemaFW **R**esponse **M**odes.
  </dd>
  <dt id="srm_branch">SRM branch</dt>
  <dd>
    A group of instructions consisting of the branch head line and the following lines
    that are more deeply indented than the branch, up to, but not including a following
    line that is at the same indentation as the branch head.
  </dd>

  <dt id="srm_mode">SRM mode</dt>
  <dd>
    An SRM mode is an SRM branch whose branch head begins on the left-most column
    (column 0).
  </dd>
  <dt id="response_mode">Response Mode</dt>
  <dd>
     An SRM mode that is designed to be invoked for preparing an XML response
     document.
  </dd>
  <dt id="resultset">Resultset</dt>
  <dd>
    A document returned by SchemaFW; an XML file that contains one or more
    results of queries.
  </dd>
</dl>

composite dialog
   : A _dialog_ consisting of multiple display contexts.

display context
   : The formated presentation of a query result.

query result
   : The output of a query within a procedure.

procedure
   : A stored procedure accepts parameters and executes one or more
     queries.  When called from SchemaFW, each result-returning SELECT query
     (or call to another procedure that has result-returning SELECT queries)
     in the procedure will render a result element in the resultset.

resultset
   : The complete output of a procedure, including as many results as are
     produced in the procedure.

--------------------------------------------------------------------------------

Back: [Understanding SRM Files](SRMFiles.md)
  
Up: [SchemaFW Basics](SchemaFWBasics.md)
  
Top: [Main Page](UserGuide.md)

