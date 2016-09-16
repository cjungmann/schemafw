# Cookie-setting Script

It's not very convenient to cut and paste cookie variables from calling
_schema.fcgi_ with login credentials, so the following script will automate
the process so you can concentrate on debugging response modes that assume
running sessions or logins.

After entering the following text, make sure that you _chmod_ to set the execution
flags.

~~~sh
#!/bin/bash
-*- mode: sh -*-

# Exit with hint if not called with 'source'
if [ "$0" != "bash" ]; then
  echo
  echo Invoke this script as source to retain the environment changes:
  echo source sfwlogin
  echo
  exit
fi

# Clear any previous cookies
export HTTP_COOKIE=''

# Set credentials (specific for example response mode):
em=youremail@gmail.com
pw=yourpassword

# Replace the following line with your specific login call,
# note that stderr is redirected to /tmp/liresult.txt
schema.fcgi -s login.srm -m login_submit -v email=${em} -v pword=${pw} 2> /tmp/liresult.txt

# Save stderr to variable and clean up
liresult=$(</tmp/liresult.txt)
rm /tmp/liresult.txt

# Start building the HTTP_COOKIE command

ecmd='export HTTP_COOKIE="'
found=

while read -r line; do
   if [[ "$line" =~ Set-Cookie:\ (.*) ]]
   then
       if [ -n "$found" ]; then
           ecmd="${ecmd}; "
       fi
       found=1
       ecmd="${ecmd}${BASH_REMATCH[1]}"
   fi
done <<< "$liresult"

ecmd="${ecmd}\""

eval ${ecmd}
~~~

To use the script, call it with _source_ so that the script is run in the current
process.  Otherwise, if the script is directly called, the environment variables will
be set in the child process running the script, and the new environment values will
be lost to the parent process.

Assuming that the script above is called _sfwlogin_, it should be called like this:

` $ source sfwlogin`