USE SessionDemo;

SET @session_string_seed = MD5(NOW());

SET @handle = 'chuckj';
SET @password = 'prunes2u';

CALL App_Login_Submit(@handle, @password);

SELECT id INTO @session_id
  FROM SSYS_SESSION
 WHERE hash = @session_string_seed;

SELECT * FROM SSYS_SESSION;

SELECT COUNT(*)
  FROM SSYS_SESSION
 WHERE id = @session_id
   AND hash = @session_string_seed;
   

CALL ssys_session_confirm(@session_id, @session_string_seed);

SELECT @session_confirmed_id, @session_still_good, @session_no_good;

SELECT "About to call App_Person_Values!";

CALL App_Person_Values(3);

SELECT "About to call App_Person_Submit!";

CALL App_Person_Submit(6, 'Mary', 'Jungmann');

-- SELECT * FROM SSYS_SESSION;
