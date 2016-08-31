# Importing Data

SchemaFW supports importing data from a limited set of spreadsheet formats.
SchemaFW uses [Gnumeric](www.gnumeric.org) to convert spreadsheet formats, so
refer to the Gnumeric [importing documentation](https://help.gnome.org/users/gnumeric/stable/gnumeric.html#sect-files-ssconvert)
for information that SchemaFW uses to use _ssconvert_ and for how to determine
the supported file formats (i.e. `ssconvert --list-importers`).

The import function is built-in and somewhat rigid, but the included
features should make it relatively easy and safe to offer importing
for your web application.

## Setup For Importing

Install [Gnumeric](www.gnumeric.org) to get the _ssconvert_ utility:

~~~
sudo apt-get install gnumeric
~~~

## From The User's View

After several adventures in writing importing code, the biggest issue
I've faced is with inappropriate importing formatting.  That is, the user
provides the wrong data, or puts it in the wrong column.  If the imported
data is immediately incorporated into other live data, this can result
in corrupted data that is difficult to extricate.

SchemaFW "quarantines" the imported data and presents to the user a subset
of the quarantine table, with column heads, to confirm that the submitted
spreadsheet is formatted correctly.  At this point, the data can be
incorporated or abandoned.

Importing data requires an active session in order to keep track of the
quarantined data.

## Preparing To Import

### MySQL Resources

The following MySQL resources are required to run an import response mode:

- A **quarantine table** that includes an _id_session_ field.  This table
  will hold the data pending acceptance or refection of the imported data.
- A **confirm procedure** that incorporates the data once it's been
  accepted.
- A **removal_procedure** that will remove the imported data from the
  quarantine table.  This procedure will be used to clean the imported
  data from the quarantine table.  This can happen after imported has
  been incorporated, or when the user abandons their imported data.

### HTML Resources

At this time, response modes do not create an appropriate form for
importing the file.  Use the following as an example until this situation
is corrected:

~~~html
<!DOCTYPE html>
<html xmlns="http://www.w3.org/1999/xhtml">
  <head>
    <title>File Upload Test/Demonstration</title>
  </head>
  <body>
    <h1>File Upload Test/Demonstration</h1>
    <p>Let's try to do a multipart form for a file upload.</p>
    <form method="post"
          action="import.srm?upload"
          enctype="multipart/form-data">
      <fieldset>
        <legend>Upload a CSV file</legend>
        <div>
          <label for="upfile">Upload the file</label>
          <input type="file" name="upfile" />
        </div>
        <div>
          <input type="submit" value="Submit"/>
        </div>
      </fieldset>
    </form>
  </body>
</html>
~~~



## The Response Mode

The response mode of an import operation is unique in that it does not
include a _procedure_ or _schema-proc_ operation.  Here is an example:

~~~srm
# SRM file import.srm

upload
   type              : import
   target            : QT_People  # Name the quarantine table in a target instruction
   confirm-procedure : App_Import_People_Save
   removal-procedure : App_Import_People_Abandon
   schema
      button
         label : Abandon
         type  : call
         task  : import.srm?abandon
      button
         label : Confirm
         type  : call
         task  : import.srm?confirm

abandon
   removal-procedure : 
~~~