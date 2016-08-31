# User Guide

This User Guide is intended to help a website developer understand how to use
the Schema Framework, also known as _SchemaFW_, to create a website.

There are several sections.

- [Preparing to Use SchemaFW](PreparingToUseSchemaFW.md)
  - [Building the Framework](BuildingTheFramework.md)
  - [Configuring Apache](ConfiguringApache.md)
  - [Create MySQL User](CreateWebUser.md)
- [SchemaFW Basics](SchemaFWBasics.md)
  - [Establish a New SchemaFW Site](CreateNewSite.md)
  - [Using MySQL Procedures](UsingMySQLProcedures.md)
  - [Understanding SRM Files](SRMFiles.md)
  - [Definitions](Definitions.md)
- [SchemaFW Case Study](SchemaFWCaseStudy.md)   
  - [A Read Interaction](CSReadInteraction.md)
  - [A List Interaction](CSListInteraction.md)
  - [A Create Interaction](CSCreateInteraction.md)
  - [An Update Interaction](CSUpdateInteraction.md)
  - [A Delete Interaction](CSDeleteInteraction.md)
  - [L-CRUD Files Listings](LCRUDInteractions.md)
- Advanced Topics
  - [Running Websites on LocalHost](RunningOnLocalhost.md)
  - [Debugging with Command Line Options](SchemaFCGIOptions.md)
  - [Debugging Response Modes](DebuggingResponseModes.md)
  - [Debugging Hints for SQL Procedures](DebuggingSQLHints.md)
  - [Adding Buttons](AddingButtons.md)
  - [Context References](ContextReferences.md)
  - [Result Dialogs (type:form-result)](ResultDialogs.md)
  - Session Stuff
    - [Session Login Procedure](SessionLoginProc.md)
    This must be fleshed out.  Assumptions I made about how this is
    set up turn out to be obsolete.  Refine the process while working
    on menu order and return to describe the conditions that dictate
    how and when the procedures are called.
    - [COMPLETE AFTER menu-order LOGIN Sessions and Authorization](SessionsAndAuthorization.md)
    - [COMPLETE AFTER menu-order LOGIN SchemaFW Session Processing Procedures](SchemaFWSessionProcs.md)
    - [Cookie-setting Script](CookieSettingScript.md)
  - [Multipart Form](MultipartForm.md)
  - [Multipart Form Section](FormSection.md)
  - [Importing Data](ImportingData.md)
  - [Composite Dialogs **NOT DONE**](CompositeDialogs.md)
  - [Using LOAD DATA LOCAL INFILE **NOT DONE**](LoadDataLocalInfile.md)
  - [Importing Files **NOT DONE**](ImportingFiles.md)
  - [MySQL Hints](MySQLHints.md)
