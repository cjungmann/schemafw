# Introduction to Associations

An item is a discrete object, either concrete or abstract.  A person is an example of
a concrete object, a running club is an example of an abstract object.

Items have attributes.  An attribute may be a physical characteristic like size or color.
An attribute may be a location or a status, like cataloged, lost, or forgotten.  An
attribute might be a favorite color, birthday, or favorite meal.

## Attribute Assocations

The attributes are said to be _associated_ with the items.  Some attributes are unique
to an item.  A high-school class might have a specific teacher, classroom, and meeting
time.  Other attributes are shared.  The class will include several students, each of
which then share the class.

The class name, teacher, classroom, and meeting time are unique attributes of the class
and are considered to be one-to-one associations, while the many students enrolled in
the class have a many-to-one assocation with the class.

## Database Representation

A database uses tables to save collections of items.  A table is a collection of
records, each of which represents an item.  The table defines the names and data types
of the fields that each record will contain.  The fields store the attributes of the
items deemed important for the model.

### Data Modelling Trap

It may be tempting to include a phone number field to a person record.  There are two
problems with doing this.

1. A given person may not have any phone numbers.  For such persons, the phone number
   field is wasted space.
2. A given person may have several phone numbers.  They may include a land-line at
   home, a personal cell phone, a work phone, a work fax, etc.  A single phone
   number field would result in the abandonment of extra phone numbers.

### Many-to-One Associations

A better solution for associating phone numbers to persons is to have a third table
that maps the relationships.

## Presenting Associations

In the simplest construction, the framework presents an _item_ as a
form and a _collection_ as a table.

A table is not always the best presentation for a collection of data.  In certain
contexts, a comma-separated list is easier to read than a column of values.  When a
collection contains items with related numeric information, pie chars, bar charts,
histograms, and line graphs can quickly communicate important information.

