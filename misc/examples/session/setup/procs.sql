USE SessionDemo;

DELIMITER $$

-- Overriding the next two items from sys_proc.sql
-- to make shorter sessions for testing.
DROP FUNCTION IF EXISTS ssys_calc_session_expires $$
CREATE FUNCTION ssys_calc_session_expires()
RETURNS DATETIME
BEGIN
   RETURN DATE_ADD(NOW(), INTERVAL 2 MINUTE);
END $$


DROP EVENT IF EXISTS ssys_session_event $$
CREATE EVENT ssys_session_event
    ON SCHEDULE EVERY 2 MINUTE
    DO CALL ssys_session_cleanup() $$


-- The following procedures will be called by SchemaFW.  Redefining
-- these procedures is optional because `sys_procs.sql` created
-- do-nothing versions of these functions to ensure calling the
-- named procedure will not create an error.
--
-- See the SchemaFW_MySQL_Procedures page in the documentation.

DROP PROCEDURE IF EXISTS App_Request_Cleanup $$
CREATE PROCEDURE App_Request_Cleanup()
BEGIN
   SET @session_id_account = NULL;
END $$

-- Makes an empty record in Session_Info table that will be filled
-- by App_Login_Submit or App_Account_Create.
DROP PROCEDURE IF EXISTS App_Session_Start $$
CREATE PROCEDURE App_Session_Start(session_id INT UNSIGNED)
BEGIN
   INSERT INTO Session_Info(id, id_account) VALUES (session_id, NULL);
END $$

-- Delete the record when the associated SSYS_SESSION record is deleted.
DROP PROCEDURE IF EXISTS App_Session_Abandon $$
CREATE PROCEDURE App_Session_Abandon(session_id INT UNSIGNED)
BEGIN
   DELETE FROM Session_Info
     WHERE id = session_id;
END $$

DROP PROCEDURE IF EXISTS App_Session_Restore $$
CREATE PROCEDURE App_Session_Restore(session_id INT UNSIGNED)
BEGIN
   SELECT id_account INTO @session_id_account
     FROM Session_Info
    WHERE id = session_id;

   IF @session_id_account IS NULL THEN
      SET @msg := CONCAT('session id ', @session_confirmed_id, ': no Session_Info record.');
      SIGNAL SQLSTATE 'ERROR' SET MESSAGE_TEXT = @msg;
   END IF;
END $$


-- Create session variables to hold session record field values:
DROP PROCEDURE IF EXISTS App_Session_Prepare $$


DROP PROCEDURE IF EXISTS App_Auth_Test $$
CREATE PROCEDURE App_Auth_Test()
BEGIN
   DECLARE id_acct INT;

   SELECT id_account INTO id_acct
     FROM Session_Info
    WHERE id = @session_confirmed_id;

   SELECT CASE WHEN id_acct IS NULL THEN 0 ELSE 1 END;
END $$

-- App_Login_Submit: open existing account, establish session:
-- Session should already exist, use @session_confirmed_id get
-- appropriate record.
DROP PROCEDURE IF EXISTS App_Login_Submit $$
CREATE PROCEDURE App_Login_Submit(handle VARCHAR(20),
                                  password VARCHAR(20))
BEGIN
   DECLARE account_id INT UNSIGNED;
   DECLARE session_id INT UNSIGNED;

   SELECT a.id INTO account_id
     FROM Account a
    WHERE a.handle=handle
      AND a.password=MD5(password);

   -- This may have to change.  For now, a result is only
   -- returned if acct_id is not null, and ssys_session_create
   -- will return the necessary fields:
   IF account_id IS NOT NULL THEN
      UPDATE Session_Info
         SET id_account = account_id
       WHERE id = @session_confirmed_id;
   END IF;
END $$

-- App_Account_Create: create a new account, establish session:
DROP PROCEDURE IF EXISTS App_Account_Create $$
CREATE PROCEDURE App_Account_Create(handle VARCHAR(20),
                                    password VARCHAR(20),
                                    password2 VARCHAR(20))
BEGIN
   DECLARE account_id INT UNSIGNED;
   DECLARE session_id INT UNSIGNED;
   
   IF password = password2 THEN
      INSERT INTO Account
         (handle, password) VALUES (handle, MD5(password));
      SET account_id = LAST_INSERT_ID();
   END IF;

   IF account_id IS NOT NULL THEN
      UPDATE Session_Info
         SET id_account = account_id
       WHERE id = @session_confirmed_id;

      CALL ssys_session_announce(session_id);
   END IF;
END $$


-- App_Account_Set_Password: account editing function
DROP PROCEDURE IF EXISTS App_Account_Set_Password $$
CREATE PROCEDURE App_Account_Set_Password(id INT UNSIGNED,
                                          old_password VARCHAR(20),
                                          new_password VARCHAR(20),
                                          new_password2 VARCHAR(20))
BEGIN
   -- Emits error signal if mismatched:
   CALL ssys_confirm_session_id(id);

   UPDATE Account a
      SET a.password = new_password
    WHERE a.id = id
      AND a.password = old_password;

    SELECT ROW_COUNT() AS recs_changed;
END $$


-- App_Person_List: get a list or a single row
DROP PROCEDURE IF EXISTS App_Person_List $$
CREATE PROCEDURE App_Person_List(id INT UNSIGNED)
BEGIN
   SELECT p.id, p.fname, p.lname
     FROM Person p
    WHERE p.id_account = @session_id_account
      AND p.id=id OR id IS NULL;
END $$

-- App_Person_Values: To fill a form with values from a person record:
DROP PROCEDURE IF EXISTS App_Person_Values $$
CREATE PROCEDURE App_Person_Values(id INT UNSIGNED)
BEGIN
   SELECT p.id, p.fname, p.lname
     FROM Person p
    WHERE p.id_account = @session_id_account
      AND p.id = id;
END $$

-- App_Person_Submit: Add or change a person record, depending on value of id_person:
DROP PROCEDURE IF EXISTS App_Person_Submit $$
CREATE PROCEDURE App_Person_Submit(id INT UNSIGNED,
                                   fname VARCHAR(20),
                                   lname VARCHAR(20))
BEGIN
   IF id IS NULL THEN
      BEGIN
         DECLARE new_id INT UNSIGNED;
         INSERT INTO Person (id, id_account, fname, lname)
                     VALUES(id, @session_id_account, fname, lname);
                     
         SET new_id = LAST_INSERT_ID();
         IF new_id IS NOT NULL THEN
            CALL App_Person_List(new_id);
         END IF;
      END;
   ELSE
      BEGIN
         UPDATE Person p
            SET p.fname = fname,
                p.lname = lname
          WHERE p.id_account = @session_id_account
            AND p.id = id;

          CALL App_Person_List(id);
       END;
   END IF;
END $$


DELIMITER ;
