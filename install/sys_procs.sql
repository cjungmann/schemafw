DELIMITER $$

SELECT ' ' AS ' \n' $$

DROP PROCEDURE IF EXISTS MyBogusForceTestingProcedure $$
CREATE PROCEDURE MyBogusForceTestingProcedure()
BEGIN
END $$

SELECT ' ' AS 'If this script ends with a single ''already exists'' error,\nyou will need to run the script again.  Use the -f\nargument to bypass already exists errors, like this:\n\nmysql -f your_dbase_name < sys_procs.sql' $$

CREATE PROCEDURE MyBogusForceTestingProcedure()
BEGIN
END $$

DROP PROCEDURE IF EXISTS MyBogusForceTestingProcedure $$


SELECT ' ' AS '.\n.\nHere begins the real procedures and functions.\nIgnore \'already exists\' messages.\n' $$


/* Find documentation for these functions in sys_procs.hpp */

DROP PROCEDURE IF EXISTS ssys_get_procedure_params $$
CREATE PROCEDURE ssys_get_procedure_params(p_proc_name VARCHAR(64))
BEGIN
   -- Prevent silent misnamed procedure error:
   DECLARE proc_count INT;
   
   SELECT COUNT(*) INTO proc_count
     FROM information_schema.ROUTINES
    WHERE ROUTINE_SCHEMA = DATABASE()
      AND ROUTINE_NAME=p_proc_name;

   IF proc_count = 0 THEN
      SELECT -1 AS 'parameter_count';
   ELSE
      SELECT COUNT(*) as 'parameter_count'
        FROM information_schema.PARAMETERS
       WHERE SPECIFIC_SCHEMA = DATABASE()
         AND SPECIFIC_NAME = p_proc_name;

      SELECT PARAMETER_MODE as mode,
             PARAMETER_NAME as name,
             DATA_TYPE as dtype,
             CHARACTER_MAXIMUM_LENGTH as len,
             NUMERIC_PRECISION as num_prec,
             NUMERIC_SCALE as num_scale,
             DTD_IDENTIFIER as dtdid
        FROM information_schema.PARAMETERS
       WHERE SPECIFIC_SCHEMA = DATABASE()
         AND SPECIFIC_NAME = p_proc_name;
   END IF;
   
END $$

DROP PROCEDURE IF EXISTS ssys_get_column_dtds $$
CREATE PROCEDURE ssys_get_column_dtds(column_list TEXT)
BEGIN
   SELECT CONCAT(TABLE_NAME,':',COLUMN_NAME) AS name, COLUMN_TYPE
     FROM information_schema.COLUMNS
    WHERE TABLE_SCHEMA=DATABASE()
      AND LOCATE(CONCAT(TABLE_NAME,':',COLUMN_NAME), column_list);
END $$

DROP PROCEDURE IF EXISTS ssys_get_procedures $$
CREATE PROCEDURE ssys_get_procedures()
BEGIN
   SELECT ROUTINE_NAME
     FROM information_schema
    WHERE ROUTINE_SCHEMA=database();
END$$


DROP PROCEDURE IF EXISTS ssys_confirm_table $$
CREATE PROCEDURE ssys_confirm_table(p_table_name VARCHAR(64))
BEGIN
   SELECT COUNT(*)
     FROM information_schema.TABLES
    WHERE TABLE_SCHEMA = database()
      AND TABLE_NAME = p_table_name;
END $$

DROP PROCEDURE IF EXISTS ssys_default_import_removal $$
CREATE PROCEDURE ssys_default_import_removal(p_table_name VARCHAR(64))
BEGIN
   SET @dir_qry = CONCAT('DELETE FROM ', p_table_name, ' WHERE id_session=?');

   PREPARE qstmt FROM @dir_qry;
   EXECUTE qstmt USING @session_confirmed_id;
   DEALLOCATE PREPARE qstmt;

   SET @dir_qry = NULL;
END $$

DROP PROCEDURE IF EXISTS ssys_default_import_confirm $$
CREATE PROCEDURE ssys_default_import_confirm(p_table_name VARCHAR(64),
                                             p_limit INT)
BEGIN
   SET @dic_qry = CONCAT('SELECT \* FROM ', p_table_name, ' WHERE id_session=?');
   IF p_limit>0 THEN
      SET @dic_qry = CONCAT(@dic_qry, ' LIMIT ', p_limit);
   END IF;

   PREPARE qstmt FROM @dic_qry;
   EXECUTE qstmt USING @session_confirmed_id;
   DEALLOCATE PREPARE qstmt;

   SET @dic_qry = NULL;
END $$



-- ----------------------------
-- Session Table and Procedures
-- ----------------------------


CREATE TABLE IF NOT EXISTS SSYS_SESSION
(
   id         INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
   hash       CHAR(32) NULL,
   expires    DATETIME NULL,
   available  BOOLEAN,

   INDEX (available)
) $$

-- The next several procedures are dummy functions that should be replaced
-- for each application.  See doxygen topic SchemaFW_MySQL_Procedures for
-- information on when each procedure will be called and what each is
-- expected to do.
--
-- These CREATE procedures will cause an error if:
-- 1. They have already been replaced by application-specific versions, and
-- 2. The '-f' directive has been omitted when call this script.
--

-- NOTE: The following App_Session_ functions should not
--       call DROP PROCEDURE in sys_procs.sql.  Reloading sys_procs.sql
--       as part of an update must not overwrite app-specific versions
--       of these procedures.
--
--       Uncomment the DROP PROCEDURE lines only after copying to an
--       application-specific script file.

-- Called when a session starts to ensure important session variables 
-- DROP PROCEDURE IF EXISTS App_Session_Cleanup $$
CREATE PROCEDURE App_Session_Cleanup()
BEGIN
END $$

-- Called to set session variables in application data tables
-- DROP PROCEDURE IF EXISTS App_Session_Start $$
CREATE PROCEDURE App_Session_Start(id INT UNSIGNED)
BEGIN
END $$

-- Called to prepare session, especially to set session variables
-- DROP PROCEDURE IF EXISTS App_Session_Restore $$
CREATE PROCEDURE App_Session_Restore(id INT UNSIGNED)
BEGIN
END $$

-- Called when session ends as opportunity to clean up data tables
-- DROP PROCEDURE IF EXISTS App_Session_Abandon $$
CREATE PROCEDURE App_Session_Abandon(id INT UNSIGNED)
BEGIN
END $$


DROP PROCEDURE IF EXISTS ssys_clear_for_request $$
CREATE PROCEDURE ssys_clear_for_request()
BEGIN
   SET @session_confirmed_id = NULL;
   CALL App_Session_Cleanup();
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

DROP FUNCTION IF EXISTS ssys_calc_session_expires $$
CREATE FUNCTION ssys_calc_session_expires()
RETURNS DATETIME
BEGIN
   RETURN DATE_ADD(NOW(), INTERVAL 20 MINUTE);
END $$

DROP FUNCTION IF EXISTS ssys_current_session_is_valid $$
CREATE FUNCTION ssys_current_session_is_valid()
RETURNS BOOLEAN
BEGIN
   DECLARE VCount INT;
   SELECT COUNT(*) INTO VCount
     FROM SSYS_SESSION
    WHERE id = @session_confirmed_id
      AND expires >= NOW();

   RETURN VCount=1;
END $$

DROP FUNCTION IF EXISTS ssys_session_create $$
CREATE FUNCTION ssys_session_create()
   RETURNS INT
BEGIN
   DECLARE session_id INT UNSIGNED;

   IF @session_string_seed IS NULL THEN
      SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'Failed to create session: missing session string seed.';
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
   
   IF session_id IS NOT NULL THEN
      CALL App_Session_Start(session_id);
   END IF;

   RETURN session_id;
End $$

DROP PROCEDURE IF EXISTS ssys_session_start $$
CREATE PROCEDURE ssys_session_start()
BEGIN
   DECLARE newid INT;
   SET newid = ssys_session_create();

   IF newid IS NOT NULL THEN
      SELECT id, hash
        FROM SSYS_SESSION
       WHERE id = newid;
       
      SET @session_confirmed_id = newid;
   END IF;
END $$

DROP PROCEDURE IF EXISTS ssys_session_confirm $$
CREATE PROCEDURE ssys_session_confirm(session_id INT UNSIGNED,
                                     session_string CHAR(32))
BEGIN
   DECLARE still_good INT;

   IF session_id IS NULL OR session_string IS NULL THEN
      SIGNAL SQLSTATE 'ERROR' SET MESSAGE_TEXT = 'passed one or both session_id or session_string NULL values.';
   END IF;

   SELECT COUNT(*) INTO still_good
     FROM SSYS_SESSION
    WHERE id = session_id
      AND hash = session_string
      AND expires >= NOW();

   IF still_good=1 THEN
      UPDATE SSYS_SESSION
         SET expires = ssys_calc_session_expires()
       WHERE id = session_id;
       
      SET @session_confirmed_id = session_id;

      CALL App_Session_Restore(session_id);
   ELSE
      SET @session_confirmed_id = NULL;
   END IF;

   SELECT still_good;
END $$

DROP PROCEDURE IF EXISTS ssys_assert_session_id $$
CREATE PROCEDURE ssys_assert_session_id(id INT UNSIGNED)
BEGIN
  IF id IS NULL OR NOT(id = @session_confirmed_id) THEN
     SIGNAL SQLSTATE 'ERROR' SET MESSAGE_TEXT = 'id doesn''t match session confirmed id.';
  END IF;
END $$

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



/**
 * @fn ssys_setup_confirm_trigger_targets
 * @brief Checks if trigger target procedures are defined.
 *
 * It isn't possible to conditionlly create procedures only if they
 * don't exist, so the best I can do is create a message that indicates
 * why the triggers ssys_sync_session_insert and ssys_sync_session_abandon
 * might have failed,  if they indeed did fail.
 */
DROP PROCEDURE IF EXISTS ssys_setup_confirm_trigger_targets $$
-- CREATE PROCEDURE ssys_setup_confirm_trigger_targets()
-- BEGIN
--    SELECT COUNT(*) INTO @start_count
--      FROM information_schema.ROUTINES
--     WHERE ROUTINE_SCHEMA = DATABASE() AND SPECIFIC_NAME='App_Session_Start';

--    SELECT COUNT(*) INTO @abandon_count
--      FROM information_schema.ROUTINES
--     WHERE ROUTINE_SCHEMA = DATABASE() AND SPECIFIC_NAME='App_Session_Abandon';

--    SET @start_msg=IF(@start_count=0,'Missing App_Session_Start procedure', 'OK');
--    SET @abandon_msg=IF(@start_count=0,'Missing App_Session_Abandon procedure', 'OK');

--    SELECT CONCAT(@start_msg, ', ', @abandon_msg) ;
-- END $$

-- Not using this for testing: use --force instead
-- CALL ssys_setup_confirm_trigger_targets() $$


-- delete obsolete triggers
DROP TRIGGER IF EXISTS ssys_sync_session_insert $$
DROP TRIGGER IF EXISTS ssys_sync_session_abandon $$

/**
 * @fn ssys_sync_session_update
 * @brief Call appropriate procedure according to whether a session
 * is beginning or ending.
 *
 * The trigger is called whenever a SSYS_SESSION record changes, but
 * this trigger procedure checks further for a change to the field
 * SSYS_SESSION.available before acting.
 *
 * If SSYS_SESSION.available changes from 1 to 0, a session has been
 * established, so the trigger calls App_Session_Start to prepare the
 * session.
 *
 * If SSYS_SESSION.available changes from 0 to 1, a session has been
 * abandoned and the trigger calls App_Session_Abandon to allow for
 * session cleanup.
 */
DROP TRIGGER IF EXISTS ssys_sync_session_update $$
CREATE TRIGGER ssys_sync_session_update
AFTER UPDATE ON SSYS_SESSION FOR EACH ROW
BEGIN
   IF NEW.available=1 AND OLD.available=0 THEN
      CALL App_Session_Abandon(NEW.id);
   END IF;
END $$

SELECT ' ' AS '\n';
SELECT ' ' AS 'The sys_procs.sql script finished successfully.\n' $$








DELIMITER ;

