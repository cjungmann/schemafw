# Definitions

<dl>
   <dt id="dialog">dialog</dt>
   <dd>
      Any display that presents information to the user.
   </dd>

   <dt id="form">form</dt>
   <dd>
     A [dialog](#dialog) that accepts input from the user.
   </dd>

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

Back: [Establish a New SchemaFW Site](CreateNewSite.md)
&nbsp;
&nbsp;
Up: [SchemaFW Basics](SchemaFWBasics.md)
&nbsp;
&nbsp;
Top: [Main Page](UserGuide.md)

