# Indexed References

PREMATURE:
While working on this page, I decided that I need to rethink the field attributes used to
setup the data relationships.  I hope to return to this page when these issues are resolved.



A reference is an indirect indication of another object.  An identity reference uses an identity
value to specify a target object.  An identity value can be almost any data type, including
character- and number-based values.

Databases use identity references to specify data records.  Database servers provide index services
that quickly convert an identity value into an offset into a permanent (disk drive) or temporary
(memory map) data store.

Although almost any data type can be used for an identity value, for computer databases are

Using an integer for an identity value is very efficient because computers are especially adept at
comparing integer values.

Databases, including MySQL, can define a column to assign a unique integer identity value to each
data row as it is added to a table.

Based on the easy setup and extremely efficient performance, integer-based identity is the preferred
data type for a primary index into a table.


direct reference to data for very fast access to table rows.
identity references to rows of a table, using indexes to   The Schema Framework extends the This guide shows how the framework handles identity references,
and proposes a naming protocol to differentiate the varied nature of indexed references.

## Assumptions

MySQL, like most other relational databases, provides an **AUTO_INCREMENT** field attribute
for integer values for the purpose of uniquely identifying rows in a table.  The Schema Framework
assumes that most tables will take advantage of this feature and 


## Types of Indexed References

These indexed references are how they are represented in the XML data.  The developer
may use several different methods to generate the indirect references from the MySQL data.

### Field Indexed References

In most cases, an XML attribute value associated with a schema field will be rendered to the
client as it appears in the XML element.  Indexed references are different because the information
in the attribute point to data found elsewhere in the XML document.

#### Proxy Field

A proxy field identifies a field for which the data is found outside of the current result.  The
proxy field instruction will include a **link** section that indicates 