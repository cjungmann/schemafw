SET storage_engine=InnoDB;

/** Default session support tables. */
CREATE TABLE IF NOT EXISTS Session_Info
(
   id        INT UNSIGNED NOT NULL PRIMARY KEY,
   id_handle INT UNSIGNED NULL
);


CREATE TABLE IF NOT EXISTS Handle
(
   id       INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
   handle   VARCHAR(128) NOT NULL UNIQUE,
   password CHAR(32)   -- length of MD5 hash output
);


/** Session support procedures. */
DELIMITER $$

/** Custom procedures supporting logged-in sessions **/
DROP PROCEDURE IF EXISTS App_Login_Init $$
CREATE PROCEDURE App_Login_Init(handle_id INT UNSIGNED)
BEGIN
   INSERT INTO Session_Info (id, id_handle)
          VALUES (@session_confirmed_id, handle_id);
   
END $$


DROP PROCEDURE IF EXISTS App_Request_Cleanup $$
CREATE PROCEDURE App_Request_Cleanup()
BEGIN
   SET @session_handle_id = NULL,
       @session_handle_name = NULL;
END $$

DROP PROCEDURE IF EXISTS App_Session_Start $$
CREATE PROCEDURE App_Session_Start(session_id INT UNSIGNED)
BEGIN
   INSERT INTO Session_Info (id,id_handle) VALUES (session_id,NULL);
END $$

DROP PROCEDURE IF EXISTS App_Session_Abandon $$
CREATE PROCEDURE App_Session_Abandon(session_id INT UNSIGNED)
BEGIN
   DELETE
     FROM Session_Info
    WHERE id = session_id;
END $$

DROP PROCEDURE IF EXISTS App_Session_Restore $$
CREATE PROCEDURE App_Session_Restore(session_id INT UNSIGNED)
BEGIN
   SET @session_handle_id =  @session_handle_name = NULL;

   SELECT h.id, h.handle INTO @session_handle_id, @session_handle_name
     FROM Session_Info i
          INNER JOIN Handle h ON i.id_handle=h.id
    WHERE i.id = session_id;
END $$

DROP PROCEDURE IF EXISTS App_Session_Confirm_Authorization $$
CREATE PROCEDURE App_Session_Confirm_Authorization()
BEGIN
   SELECT CASE
             WHEN @session_handle_id IS NULL THEN 0
             ELSE 1
          END;
END $$



DROP PROCEDURE IF EXISTS App_Login_Create $$
CREATE PROCEDURE App_Login_Create(handle VARCHAR(128),
                                  password1 VARCHAR(20),
                                  password2 VARCHAR(20))
BEGIN
   DECLARE handle_count INT;
   DECLARE signal_message VARCHAR(250);
   SELECT COUNT(*) INTO handle_count
     FROM Handle
    WHERE Handle.handle = handle;

   IF handle_count>0 THEN
      SET signal_message = CONCAT('''', handle, ''' is not available.''');
      SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = signal_message;
    ELSEIF NOT (password1=password2) THEN
      SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'Mismatched password confirmation.';
   ELSE
      INSERT INTO Handle (handle, password) VALUES (handle, MD5(password1));
   END IF;
   
END $$

/** LoginDemo application procedures */
DROP PROCEDURE IF EXISTS App_Login_Submit $$
CREATE PROCEDURE App_Login_Submit(p_handle VARCHAR(128), p_password VARCHAR(20))
BEGIN
   DECLARE handle_id INT UNSIGNED;

   IF @session_confirmed_id IS NULL THEN
      SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'No session to log in to.';
   END IF;

   SELECT id INTO handle_id
     FROM Handle
    WHERE handle = p_handle
      AND password = MD5(p_password);

   -- App_Session_Start should have already created
   -- the record to which we are adding this information:
   IF handle_id IS NOT NULL THEN
      UPDATE Session_Info
         SET id_handle = handle_id
       WHERE id = @session_confirmed_id;
   END IF;

END $$


