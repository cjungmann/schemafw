# SRM Files

SRM stands for **S**chema **R**esponse **M**ode, which describes the contents
of an SRM file.  An SRM file is a formatted text file that contains instructions
about how to interpret URLs by the _schema.fcgi_ server application.

## Format

An SRM file is a hierarchical document, with each line being either an
instruction or a node.  The relationship between lines is defined by the
relative level of indentation between the lines.  A line that is more
indented than its predecessor is interpreted to be a child, and lines
that match the indentation of a previous line is considered to be a sibling.

~~~
mode
   child1 : Mark
   child2 : Judy
      grandchild1 : Ellen
      grandchild2 : Ralph
   child3 : 
