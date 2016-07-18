USE ImportDemo;

CREATE TABLE IF NOT EXISTS Session_Import_People
(
   id_session INT UNSIGNED,
   id_family  VARCHAR(15),
   id_student VARCHAR(15),
   lname      VARCHAR(25),
   fname      VARCHAR(25),
   grade      VARCHAR(3),
   homeroom   VARCHAR(30),
   phone      VARCHAR(15),
   email1     VARCHAR(180),
   email2     VARCHAR(180)
);


DELIMITER $$

DROP PROCEDURE IF EXISTS App_Family_List $$
DROP PROCEDURE IF EXISTS App_Set_Warning_Level $$
DROP PROCEDURE IF EXISTS App_Create_Import_People $$
DROP PROCEDURE IF EXISTS App_Import_People $$


/**
 * @brief Minimal session-creation function for unconditional session.
 *
 * We need to have a session value to prepend to imported records
 * in case more than one person is attempting to import records
 * at the same time.
 */
DROP PROCEDURE IF EXISTS App_Session_Abandon $$
DROP PROCEDURE IF EXISTS App_Session_Start $$
CREATE PROCEDURE App_Session_Start(session_id INT UNSIGNED)
BEGIN
END $$

DROP PROCEDURE IF EXISTS App_Session_Abandon $$
CREATE PROCEDURE App_Session_Abandon(session_id INT UNSIGNED)
BEGIN
END $$

DROP PROCEDURE IF EXISTS App_Import_People_Preview $$
CREATE PROCEDURE App_Import_People_Preview()
BEGIN
   IF ssys_current_session_is_valid() THEN
      SELECT *
        FROM Session_Import_People
       WHERE id_session = @session_confirmed_id;
   END IF;
END $$


DROP PROCEDURE IF EXISTS App_Import_People_Abandon $$
DROP PROCEDURE IF EXISTS App_Import_People_Remove $$
CREATE PROCEDURE App_Import_People_Remove(p_table_name VARCHAR(64))
BEGIN
   IF p_table_name != 'Session_Import_People' THEN
      SIGNAL SQLSTATE 'ERROR' SET MESSAGE_TEXT =
       'Wrong table name for App_import_People_Remove.';
   END IF;

   IF ssys_current_session_is_valid() THEN
      DELETE
        FROM Session_Import_People
       WHERE id_session = @session_confirmed_id;
   END IF;
END $$

DROP PROCEDURE IF EXISTS App_Import_People_Accept $$
CREATE PROCEDURE App_Import_People_Accept()
BEGIN
   IF ssys_current_session_is_valid() THEN
      SET @one=1;
   END IF;
END $$



DELIMITER ;
