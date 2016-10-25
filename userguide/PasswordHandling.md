# Password Handling

Since many people use the same password on multiple online accounts, revealing
the password can compromise a user's online security.  For that reason, it is
important for any site to make it as difficult as possible to access the passwords.
This means that passwords must not be saved in plain text, but rather obscured
so that, even in the event of someone somehow getting full access to the database
in which the passwords are stored, it is a significant burden to discover the
real password.

Note that database security breaches are usually the result of an SQL-injection
attack, and SchemaFW can prevent SQL-injection attacks if it is set up according
to the recommendations ([Preparing to Use SchemaFW](PreparingToUseSchemaFW.md)).
However, SchemaFW cannot protect data if someone gets access to a server running
SchemaFW, so it is still necessary to protect passwords against that sort of an
attack.

This is widely done with some sort of hashing, where an algorithm is applied to
a given string to generate a virtually unique hash value.  It is a one-way exercise
to hashing strings or any other data stream: it is not possible to process a
hash to reveal the string or data from which the hash was derived.

Any hash is vulnerable to a so-called _brute-force_ attack, where every possible
string is hashed and compared to the password hash.  Shorter passwords are more
vulnerable, in this case, because it won't take as long to encounter a match.

It is possible to create a catalog of strings with their associated hash values.
The table can then be used to look up a password associated with a given hash value.

Random strings, called _salt_, are usually appended to a password to prevent
table matches.  If a database has been compromised, the salt and the algorithm
for hashing the string can be determined, but a lookup table will be useless
as a shortcut, and to discover the password would require applying the hash
routine to every possible string to find the matching hash value.

## How it Works

SchemaFW tries to make it easy to obscure passwords.  The built-in methods used
are _not_ the ultimate in security, but are a reasonable compromise given the
inherent database security that SchemaFW makes easy to accomplish.  

The password handling functions built into SchemaFW convert a password string
into a undeciperable string.

We do not want to expose users' passwords in case
of a database security breach: many people use the same password for several sites,
and exposing their password may compromise their other online accounts.
See [Wikipedia](https://en.wikipedia.org/wiki/Salt_(cryptography)) for an explanation
of salting.
