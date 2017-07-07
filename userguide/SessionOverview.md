# Session Overview

A session can be established to keep track of a user between requests.  Sessions
are useful for authorized access, allowing continuing privileged access to data
after successfully satisfying the authorization challenge (i.e. a login/password
combination).  Sessions are also recommended for file imports, give a user the
opportunity to confirm the imported data.

Session instructions can occur within a response mode, or they can be global
to a SRM file.  They can also be global across several SRM files by putting
the session instructions in a shared file and including it everywhere.

In practice, it usually makes sense to use global session instructions to
apply to several response modes in a SRM file.  The purpose of a session is
to track a user over multiple related requests, which suggests their being
in a single SRM file.

## SRM Session Instructions

The session instructions are global SRM instructions.  That is, they reside
outside of any response mode and apply to all response modes in the SRM file.
For this reason, the session commands should be in a common SRM file that's
included in all SRM files of a directory, or else in the few files where
a session is important.  

## Simple Unauthorized Session

~~~srm
$session-type: simple
~~~

For simple session type SRM scripts, the Framework either renews an existing
session or it assigns a new session for anyone who doesn't have one (be it new or
expired).  In contrast to the **identity** session type, the response modes in
a simple session type script are not restricted.

If a user encounters a simple session type script after previously using a
identity session, the identity session will be extended, even though the
authorization is not confirmed.

This [Import Case Study](ImportingData.md) shows the use of a simple session
type.  The anonymous session is used to provide feedback and a chance to abort
an import upload.

## Identity Sessions

~~~srm
$session-type        : identity
$test_authorized     : App_Session_OK
$jump_not_authorized : login.srm
~~~

Identity sessions require some authorization exercise.  Typically, and as a
built-in feature, the Framework provides the queries and other support for
authorizing a user via a user-name/password pair.  The use of these features
are covered in [SchemaFW Session Authorizations](SchemaFWAuthorizations.md).

In addition to declaring an identity session, the SRM script must include
an instruction to name the procedure that confirms an active session,
`$test_authorized: App_Confirmation_Proc` and a URL to jump to if an identity
session is expired or not yet established, `$jump_not_authorized: login.srm`.