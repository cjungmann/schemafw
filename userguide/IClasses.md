# IClass

In this documentation, a **class** refers to a Javascript function with a prototype
that includes methods.

An **interactive-class**, or **iclass**, is a Javascript class that handles the
user interaction with an HTML view of a dataset.  The framework provides two
fundamental iclasses, a _table_ class for multi-element resultsets and a _form_
class for creating, viewing, or editing a single conceptual record.

IClasses are designed to be base-classes for other iclasses.  The steps for deriving
a custom iclass are covered in [Custom Interactive Classes](CustomIClasses.md).

The framework includes two derived classes (when this document was created), the
_form-view_ class for handling composite forms, and the _calendar_ class for
displaying items associated with dates in a calendar format.  Discussions of these
derived classes can be found at [The Form-view Class](FormViewCaseStudy.md) and
[Creating a Calendar](CalendarCaseStudy.md), respectively.