$database : LoginDemo
$xml-stylesheet : default.xsl
$default-mode : login

$session-type : login
# $test_authorized : App_Auth_Test
$jump_not_authorized : ?default:login

login
   procedure: App_Login_Submit
   type : form-login
   schema : handle
      form-action : ?default:submit_login
      buttons
         button
            label : Create New Account
            type : jump
            url : ?default:create

submit_login
   procedure: App_Login_Submit
   type : form-confirm
   jump : ?default:home

create
   procedure : App_Login_Create
   type : form-login
   schema : handle
      form-action : ?default:submit_create
      buttons
         button
            label : Login to Existing Account
            type : jump
            url : ?default:login

submit_create
   procedure : App_Login_Create
   type : form-confirm
   jump : ?default:home

home
   procedure : App_Home
   root
      root-procedure
