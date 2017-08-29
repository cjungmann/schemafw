DELIMITER $$

-- Procedures in this file
-- MyBogusForceTestingProcedure
-- ssys_schemafw_version
-- ssys_drop_salt_string
-- ssys_hash_password_with_salt
-- ssys_confirm_salted_hash
-- ssys_get_procedure_params
-- ssys_get_column_dtds
-- ssys_get_procedures
-- ssys_confirm_table
-- ssys_default_import_removal
-- ssys_default_import_confirm
-- ssys_make_SFW_IntTable_from_list
-- ssys_month_info_result
-- ssys_month_get_first_and_last

-- App_Session_Cleanp
-- App_Session_Start
-- App_Session_Restore
-- App_Session_Abandon

-- ssys_clear_for_request
-- ssys_seed_session_string
-- ssys_calc_session_expires
-- ssys_current_session_is_valid
-- ssys_session_create
-- ssys_session_start
-- ssys_session_confirm
-- ssys_assert_session_id
-- ssys_session_abandon
-- ssys_session_cleanup

-- Also, an event_scheduler for checking session statuses
-- and a trigger (ssys_sync_session_update) to call a
-- session cleanup procedure.

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

-- ----------------------------------------------
DROP PROCEDURE IF EXISTS ssys_schemafw_version $$
CREATE PROCEDURE ssys_schemafw_version()
BEGIN
   SELECT 0.95;
END $$

-- ----------------------------------------------
DROP PROCEDURE IF EXISTS ssys_drop_salt_string $$
CREATE PROCEDURE ssys_drop_salt_string(salt_string VARCHAR(255))
BEGIN
   -- variable names that begin with @ are session variables,
   -- and persist as long as the connection is open.  For
   -- FASTCGI, we'll have to be careful to clear or at least
   -- not reuse the session string seed.
   SET @dropped_salt = salt_string;
END $$

-- ----------------------------------------------------
DROP FUNCTION IF EXISTS ssys_hash_password_with_salt $$
CREATE FUNCTION ssys_hash_password_with_salt(password VARCHAR(255),
                                             salt_string VARCHAR(255))
RETURNS BINARY(16)
BEGIN
   RETURN UNHEX(MD5(CONCAT(salt_string,password)));
END $$

-- ------------------------------------------------
DROP FUNCTION IF EXISTS ssys_confirm_salted_hash $$
CREATE FUNCTION ssys_confirm_salted_hash(saved_hash BINARY(16),
                                         saved_salt VARCHAR(255),
                                         password VARCHAR(255))
RETURNS BOOLEAN
BEGIN
   DECLARE diffs INT DEFAULT 0;
   DECLARE curdiff INT;
   DECLARE hpos INT;
   DECLARE pword_hash BINARY(16);
   SET pword_hash = ssys_hash_password_with_salt(password, saved_salt);

   SET hpos = 0;
   WHILE hpos < 16 DO
      SET hpos = hpos + 1;
      IF ASCII(SUBSTRING(saved_hash,hpos,1))=ASCII(SUBSTRING(pword_hash,hpos,1)) THEN
         SET curdiff = 0;
      ELSE
         SET curdiff = 1;
      END IF;
      SET diffs = diffs + curdiff;
   END WHILE;

   return diffs=0;
END $$



-- --------------------------------------------------
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
             CAST(NUMERIC_PRECISION AS SIGNED) as num_prec,
             NUMERIC_SCALE as num_scale,
             DTD_IDENTIFIER as dtdid
        FROM information_schema.PARAMETERS
       WHERE SPECIFIC_SCHEMA = DATABASE()
         AND SPECIFIC_NAME = p_proc_name;
   END IF;
   
END $$

-- ---------------------------------------------
DROP PROCEDURE IF EXISTS ssys_get_column_dtds $$
CREATE PROCEDURE ssys_get_column_dtds(column_list TEXT)
BEGIN
   SELECT CONCAT(TABLE_NAME,':',COLUMN_NAME) AS name, COLUMN_TYPE
     FROM information_schema.COLUMNS
    WHERE TABLE_SCHEMA=DATABASE()
      AND LOCATE(CONCAT(TABLE_NAME,':',COLUMN_NAME), column_list);
END $$

-- --------------------------------------------
DROP PROCEDURE IF EXISTS ssys_get_procedures $$
CREATE PROCEDURE ssys_get_procedures()
BEGIN
   SELECT ROUTINE_NAME
     FROM information_schema
    WHERE ROUTINE_SCHEMA=database();
END$$


-- -------------------------------------------
DROP PROCEDURE IF EXISTS ssys_confirm_table $$
CREATE PROCEDURE ssys_confirm_table(p_table_name VARCHAR(64))
BEGIN
   SELECT COUNT(*)
     FROM information_schema.TABLES
    WHERE TABLE_SCHEMA = database()
      AND TABLE_NAME = p_table_name;
END $$

-- ----------------------------------------------------
DROP PROCEDURE IF EXISTS ssys_default_import_removal $$
CREATE PROCEDURE ssys_default_import_removal(p_table_name VARCHAR(64))
BEGIN
   -- Confirm valid table name to avoid SQL Injection attack:
   DECLARE tcount INT DEFAULT 0;
   SELECT COUNT(*) INTO tcount
     FROM information_schema.TABLES
    WHERE TABLE_SCHEMA = database()
      AND TABLE_NAME = p_table_name;

   IF tcount = 1 THEN
      SET @dir_qry = CONCAT('DELETE FROM ', p_table_name, ' WHERE id_session=?');

      PREPARE qstmt FROM @dir_qry;
      EXECUTE qstmt USING @session_confirmed_id;
      DEALLOCATE PREPARE qstmt;

      SET @dir_qry = NULL;
   END IF;
END $$

-- ----------------------------------------------------
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

-- ---------------------------------------------------------
DROP PROCEDURE IF EXISTS ssys_make_SFW_IntTable_from_list $$
CREATE PROCEDURE ssys_make_SFW_IntTable_from_list(intlist TEXT)
BEGIN
   DECLARE valstr VARCHAR(10);
   DECLARE vallen INT UNSIGNED;
   DECLARE tlist TEXT DEFAULT intlist;

   DROP TABLE IF EXISTS SFW_IntTable;
   CREATE TEMPORARY TABLE SFW_IntTable
   ( val INT UNSIGNED )
   ENGINE=MEMORY;

   int_loop : LOOP
      SET valstr = SUBSTRING_INDEX(tlist,',',1);
      SET vallen = LENGTH(valstr);

      IF vallen=0 THEN
         LEAVE int_loop;
      ELSE
         INSERT INTO SFW_IntTable (val) VALUES(valstr);

         IF LENGTH(tlist) > vallen+1 THEN
            SET tlist = SUBSTRING(tlist,vallen+2);
         ELSE
            LEAVE int_loop;
         END IF;
      END IF;
   END LOOP int_loop;
END $$



-- --------------------------
-- Calendar Helper Procedures
-- --------------------------
-- -----------------------------------------------
DROP PROCEDURE IF EXISTS ssys_month_info_result $$
CREATE PROCEDURE ssys_month_info_result(ddate DATE)
BEGIN
   DECLARE s_first     DATE;
   DECLARE month       CHAR(7);
   DECLARE initialDay  INT UNSIGNED;
   DECLARE countOfDays INT UNSIGNED;

   SET s_first = DATE_SUB(ddate, INTERVAL DAYOFMONTH(ddate)-1 DAY);
   
   SET initialDay = DAYOFWEEK(s_first) - 1;  -- a javascript month is 0-based
   SET countOfDays = DAYOFMONTH(DATE_SUB(DATE_ADD(s_first, INTERVAL 1 MONTH),INTERVAL 1 DAY));

   SET month = CONCAT(YEAR(ddate),'-', SUBSTRING(CONCAT(MONTH(ddate)+100),2));

   SELECT month, initialDay, countOfDays, DATE_FORMAT(NOW(),'%Y-%m-%d') AS 'today';
END $$

-- ------------------------------------------------------
DROP PROCEDURE IF EXISTS ssys_month_get_first_and_last $$
CREATE PROCEDURE ssys_month_get_first_and_last(tdate DATE,
                                               OUT first_day DATE,
                                               OUT last_day DATE)
BEGIN
   DECLARE wmonth INT DEFAULT MONTH(tdate);
   DECLARE wday INT DEFAULT DAY(tdate);
   DECLARE wyear INT DEFAULT YEAR(tdate);

   DECLARE next_month INT DEFAULT MOD(wmonth,12)+1;
   DECLARE lyear INT;

   SET lyear = CASE WHEN next_month>wmonth THEN wyear ELSE wyear+1 END;

   SELECT CONCAT_WS('-',wyear,SUBSTR(CONCAT(wmonth+100),2),'01'),
          CONCAT_WS('-',lyear,SUBSTR(CONCAT(next_month+100),2),'01') - INTERVAL 1 DAY
     INTO first_day, last_day;
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
CREATE PROCEDURE App_Session_Start(session_id INT UNSIGNED)
BEGIN
END $$

-- Called to prepare session, especially to set session variables
-- DROP PROCEDURE IF EXISTS App_Session_Restore $$
CREATE PROCEDURE App_Session_Restore(session_id INT UNSIGNED)
BEGIN
END $$

-- Called when session ends as opportunity to clean up data tables
-- DROP PROCEDURE IF EXISTS App_Session_Abandon $$
CREATE PROCEDURE App_Session_Abandon(session_id INT UNSIGNED)
BEGIN
END $$


-- -----------------------------------------------
DROP PROCEDURE IF EXISTS ssys_clear_for_request $$
CREATE PROCEDURE ssys_clear_for_request()
BEGIN
   SET @session_confirmed_id = NULL;
   CALL App_Session_Cleanup();
END $$

-- -------------------------------------------------
DROP PROCEDURE IF EXISTS ssys_seed_session_string $$
CREATE PROCEDURE ssys_seed_session_string(session_string CHAR(32))
BEGIN
   -- variable names that begin with @ are session variables,
   -- and persist as long as the connection is open.  For
   -- FASTCGI, we'll have to be careful to clear or at least
   -- not reuse the session string seed.
   SET @session_string_seed = session_string;
END $$

-- -------------------------------------------------
DROP FUNCTION IF EXISTS ssys_calc_session_expires $$
CREATE FUNCTION ssys_calc_session_expires()
RETURNS DATETIME
BEGIN
   RETURN DATE_ADD(NOW(), INTERVAL 20 MINUTE);
END $$

-- -----------------------------------------------------
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

-- -------------------------------------------
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

-- -------------------------------------------
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

-- ---------------------------------------------
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

-- -----------------------------------------------
DROP PROCEDURE IF EXISTS ssys_assert_session_id $$
CREATE PROCEDURE ssys_assert_session_id(session_id INT UNSIGNED)
BEGIN
  IF id IS NULL OR NOT(session_id = @session_confirmed_id) THEN
     SIGNAL SQLSTATE 'ERROR' SET MESSAGE_TEXT = 'id doesn''t match session confirmed id.';
  END IF;
END $$

-- ---------------------------------------------
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

-- ---------------------------------------------
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

