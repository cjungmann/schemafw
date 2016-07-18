use SessionDemo;

SET @my_session_string = '12345678901234567890123456789012';

CALL ssys_seed_session_string(@my_session_string);
CALL ssys_session_start();
