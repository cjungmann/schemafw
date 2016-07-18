USE SessionDemo;

CREATE TABLE IF NOT EXISTS SSYS_SESSION
(
   id         INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
   hash       CHAR(32) NULL,
   expires    DATETIME NULL,
   available  BOOLEAN,

   INDEX (available)
);


DELIMITER $$

-- @session_confimed_id will be set to an INT value or NULL,
-- according to whether or not a session was confirmed.  This
-- session variable should be used to access application-specific
-- tables with session information.


-- Not sure if we'll need this to clear previous session.
-- ssys_confirm_session set or clears @session_confirmed_id according
-- to the session_id and session_string values.  There shouldn't be
-- any way to bypass that function.
DROP PROCEDURE IF EXISTS ssys_clear_session $$
CREATE PROCEDURE ssys_clear_session()
BEGIN
   SET @session_confirmed_id = NULL;
END $$

-- Like an ASSERT, makes a signal if id doesn't match @session_confirmed_id
DROP PROCEDURE IF EXISTS ssys_confirm_session_id $$
CREATE PROCEDURE ssys_confirm_session_id(id INT UNSIGNED)
BEGIN
  IF NOT(id = @session_confirmed_id) THEN
     SIGNAL SQLSTATE 'ERROR' SET MESSAGE_TEXT = 'id doesn''t match session confirmed id.';
  END IF;
END $$


DROP PROCEDURE IF EXISTS ssys_seed_session_string $$
CREATE PROCEDURE ssys_seed_session_string(session_string CHAR(32))
BEGIN
   -- variable names that begin with @ are session variables,
   -- and persist as long as the connection is open.  For
   -- FASTCGI, we'll have to be careful to clear or at least
   -- not reuse the session string seed.
   SET @session_string_seed = session_string;
END $$

-- Return time value of 20 minutes in the future
DROP FUNCTION IF EXISTS ssys_calc_session_expires $$
CREATE FUNCTION ssys_calc_session_expires()
RETURNS DATETIME
BEGIN
   RETURN DATE_ADD(NOW(), INTERVAL 20 MINUTE);
END $$

-- Checks all sessions, clearing the record, except for the id,
-- for all sessions that have expired.
DROP PROCEDURE IF EXISTS ssys_session_cleanup $$
CREATE PROCEDURE ssys_session_cleanup()
BEGIN
   UPDATE SSYS_SESSION
      SET hash=NULL,
          expires=NULL,
          available=1
    WHERE expires < NOW();
END $$

SET GLOBAL event_scheduler = TRUE;

DROP EVENT IF EXISTS ssys_session_event $$
CREATE EVENT ssys_session_event
    ON SCHEDULE EVERY 5 MINUTE
    DO CALL ssys_session_cleanup() $$

-- Returns TRUE if record found and it hasn't expired, FALSE otherwise.
-- Always updates the session record, either extending the session if the
-- session is still valid, or clearing the record (except the id key field)
-- and marking it available if the session has expired.
--
-- Changed this to a procedure because functions are hard/impossible to
-- access with the C API.
DROP FUNCTION IF EXISTS ssys_session_confirm $$
DROP PROCEDURE IF EXISTS ssys_session_confirm $$
CREATE PROCEDURE ssys_session_confirm(session_id INT UNSIGNED,
                                     session_string CHAR(32))
BEGIN
   DECLARE still_good INT;

   SELECT COUNT(*) INTO @still_good
     FROM SSYS_SESSION
    WHERE id = session_id
      AND hash = session_string;

   IF @still_good=1 THEN
      UPDATE SSYS_SESSION
         SET expires = ssys_calc_session_expires()
       WHERE id = session_id;
      SET @session_confirmed_id = session_id;
   ELSE
      SET @session_confirmed_id = NULL;
   END IF;

   SELECT @still_good;
END $$

-- Provide this function to standardize the query result
-- returned from any procedure that uses ssys_session_create:
DROP PROCEDURE IF EXISTS ssys_session_announce $$
CREATE PROCEDURE ssys_session_announce(session_id INT UNSIGNED)
BEGIN
   SELECT id, hash
     FROM SSYS_SESSION
    WHERE id=session_id;
END $$

-- Produces a fresh session record and return its id as an indication of success
DROP FUNCTION IF EXISTS ssys_session_create $$
CREATE FUNCTION ssys_session_create()
   RETURNS INT
BEGIN
   DECLARE session_id INT UNSIGNED;

   IF @session_string_seed IS NULL THEN
      SIGNAL SQLSTATE 'ERROR' SET MESSAGE_TEXT = 'Failed to create session: missing session string seed.';
   END IF;
   
   SELECT id INTO session_id
     FROM SSYS_SESSION
    WHERE available
    LIMIT 1;

   IF session_id IS NOT NULL THEN
      UPDATE SSYS_SESSION
         SET hash = @session_string_seed,
             expires = ssys_calc_session_expires(),
             available = 0
       WHERE id = session_id;
   ELSE
      INSERT
        INTO SSYS_SESSION (hash, expires, available)
        VALUES (@session_string_seed, ssys_calc_session_expires(), false);
         
      SET session_id = LAST_INSERT_ID();
   END IF;
   
   SET @session_confirmed_id = session_id;

   RETURN session_id;
END $$

-- Use this function to clear the session record before it would have expired.
DROP PROCEDURE IF EXISTS ssys_session_abandon $$
CREATE PROCEDURE ssys_session_abandon(session_id INT UNSIGNED,
                                     session_string char(32))
BEGIN
   UPDATE SSYS_SESSION
      SET hash = NULL,
          expires = NULL,
          available = 1
    WHERE id = session_id
      AND hash = session_string;

    SELECT ROW_COUNT() AS session_abandoned;
END $$


-- Write error messages if either prerequisit stored procedure is missing:
SELECT COUNT(*) INTO @start_count
  FROM information_schema.ROUTINES
 WHERE ROUTINE_SCHEMA = DATABASE() AND SPECIFIC_NAME='App_Session_Start' $$

SELECT COUNT(*) INTO @abandon_count
  FROM information_schema.ROUTINES
 WHERE ROUTINE_SCHEMA = DATABASE() AND SPECIFIC_NAME='App_Session_Abandon';

SET @start_msg = IF(@start_count=0, 'ERROR: Missing App_Session_Start procedure', 'OK');
SET @abandon_msg = IF(@start_count=0, 'ERROR: Missing App_Session_Abandon procedure', 'OK');

SELECT CONCAT(@start_msg, ', ', @abandon_msg) ;




-- Call App_Session_Start when a new SSYS_SESSION record is created:
DROP TRIGGER IF EXISTS ssys_sync_session_insert $$
CREATE TRIGGER ssys_sync_session_insert
AFTER INSERT ON SSYS_SESSION FOR EACH ROW
BEGIN
   DECLARE tid INT UNSIGNED;
   SET tid := NEW.id;
   CALL App_Session_Start(tid);
END $$

-- Checking for a change in SSYS_SESSION.available, if it's true,
-- a new session is using the record, so call App_Session_Start().
-- Otherwise, SSYS_SESSION.available is false, a session has been
-- canceled, so call App_Session_Abandon so the application can
-- clear or delete the custom session data.
DROP TRIGGER IF EXISTS ssys_sync_session_abandon $$
CREATE TRIGGER ssys_sync_session_abandon
AFTER UPDATE ON SSYS_SESSION FOR EACH ROW
BEGIN
   IF NEW.available=0 AND OLD.available=1 THEN
      CALL App_Session_Start(NEW.id);
   ELSEIF NEW.available=1 AND OLD.available=0 THEN
      CALL App_Session_Abandon(NEW.id);
   END IF;
END $$



DELIMITER ;
