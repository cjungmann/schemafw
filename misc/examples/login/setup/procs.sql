DELIMITER $$

DROP PROCEDURE IF EXISTS App_Home $$
CREATE PROCEDURE App_Home()
BEGIN
   SELECT @session_handle_id AS session_id, @session_handle_name AS session_handle;
END $$
