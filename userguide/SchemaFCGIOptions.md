# Schema FCGI Server Command-line Options

The program, _schema.fcgi_ normally runs as a FASTCGI processor, using redirected
streams for _stdin_, _stdout_, and _stderr_.  A debugging mode is provided, as well,
where the I/O is with the console in order to see exactly what is sent to a client
with a given set of posted values.

## The List of Options

- <b>--version</b> Shows the version number of _schema.fcgi_
- <b>-b</b> Enter debugging, or _batch_ mode.
- <b>-p</b> Use POST mode  (this is the default setting).
- <b>-b</b> Use GET mode.  Note that POST mode also processing query string values
    in the same manner as traditional GET mode, so this flag is technically not
    necessary.
- <b>-m</b> Name of the response mode to run.
- <b>-s</b> Name of the SRM file to run.
- <b>-v</b> Include the following value as a posted value

- <b>-d</b> Set debug action mode, followed by one of
   - <b>mode</b> print the contents of the named mode.
   - <b>modes</b> print a list of the response modes in the SRM file.
   - <b>all-modes</b> prints a list of all modes, including global and shared modes.
   - <b>types</b> prints a list of predefined response mode types.  User-defined types
      are allowed, but the the server will produce default output.  A user-defined
      type should expect to be caught in the XSL stylesheet and result in custom
      output.

## Examples

`$ ./schema.fcgi -b -m show_person -v id=1 -v fname=Chuck`
Describing the arguments in order:
- -b Use batch mode
- -m show_person    use response mode show_person
- -v id=1           POST variable `id` equals 1
- -v fname=Chuck    POST variable `fname' equals Chuck

Omitting -s means that `schema.fcgi` will attempt to open _default.srm_.

Using two parameters (id and fname) helps prevent accidental or unauthorized
access of the person record.

`$ ./schema.fcgi -b -s persons -m edit_person -v id=1 -v lname=Smith`
This command is typical for retrieving a record for editing.
- -b                Use batch mode
- -s persons        Looks for a file named _persons_ or _persons.srm_
- -m edit_person    use response mode show_person
- -v id=1           POST variable `id` equals 1
- -v lname=Smith    POST variable `lname' equals Smith

Using two parameters (id and fname) helps prevent accidental or unauthorized
access of the person record.

`$ ./schema.fcgi -b -s persons -m edit_person_submit -v id=1 -v fname=Chuck -v lname=Smith -v phone=555-555-1212`

