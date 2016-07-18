USE AllowanceDemo;

DELIMITER $$

DROP PROCEDURE IF EXISTS App_Person_List $$
CREATE PROCEDURE App_Person_List(id INT UNSIGNED)
BEGIN
   SELECT p.id,
          p.nom,
          p.birthday
     FROM Person p
    WHERE id IS NULL OR p.id=id;
END $$


DROP PROCEDURE IF EXISTS App_Person_Values $$
CREATE PROCEDURE App_Person_Values(id INT UNSIGNED)
BEGIN
   SELECT id, nom, birthday
     FROM Person p
    WHERE p.id = id;
END $$

DROP PROCEDURE IF EXISTS App_Person_Submit $$
CREATE PROCEDURE App_Person_Submit(id INT UNSIGNED,
                                   nom VARCHAR(20),
                                   birthday DATE)
BEGIN
   DECLARE rid INT UNSIGNED;
   
   IF id IS NULL THEN
      INSERT INTO Person(nom, birthday)
           VALUES (nom, birthday);
           
      SET rid = LAST_INSERT_ID();
   ELSE
      UPDATE Person p
         SET p.nom = nom,
             birthday = birthday
       WHERE p.id = id;
      SET rid = id;
   END IF;

   IF rid IS NOT NULL THEN
      CALL App_Person_List(rid);
   END IF;
END $$

DROP PROCEDURE IF EXISTS App_Person_Delete $$
CREATE PROCEDURE App_Person_Delete(id INT UNSIGNED)
BEGIN
   -- We will only delete a person who has not particiated in any transactions.
   -- If we allow it, we need to do something with the transaction details.  There
   -- are acceptable ways to do it, but here we demonstrate how to check.
   DECLARE tcount INT UNSIGNED;
   
   SELECT COUNT(*) INTO tcount
     FROM Transaction_Details t
          INNER JOIN Accounts a ON t.id_account = a.id
          INNER JOIN Person p ON a.id_person = p.id
    WHERE p.id = id;

   IF tcount=0 THEN
      DELETE FROM Person
       WHERE Person.id = id;
       
      SELECT ROW_COUNT() AS deleted;
   ELSE
      SELECT 0 AS deleted;
   END IF;
END $$


DROP PROCEDURE IF EXISTS App_Person_Accounts $$
CREATE PROCEDURE App_Person_Accounts(id_person INT UNSIGNED,
                                     id INT UNSIGNED)
BEGIN
   SELECT a.id,
          a.id_person,
          a.title,
          a.balance
     FROM Accounts a
    WHERE a.id_person = id_person
      AND (id IS NULL OR a.id=id);
END $$


DROP PROCEDURE IF EXISTS App_Person_Info $$
CREATE PROCEDURE App_Person_Info(id INT UNSIGNED)
BEGIN
   SELECT p.id,
          p.nom
     FROM Person p
    WHERE p.id = id;

    CALL App_Person_Accounts(id, NULL);
END $$

DROP PROCEDURE IF EXISTS App_Account_Values $$
DROP PROCEDURE IF EXISTS App_Account_Submit $$
DROP PROCEDURE IF EXISTS App_Account_Delete $$

CREATE PROCEDURE App_Account_Values(id INT UNSIGNED,
                                    id_person INT UNSIGNED)
BEGIN
   SELECT id, id_person;

   SELECT a.id, a.id_person, a.title, a.balance
     FROM Accounts a
    WHERE a.id = id
      AND a.id_person = id_person;
END $$


CREATE PROCEDURE App_Account_Submit(id INT UNSIGNED,
                                    id_person INT UNSIGNED,
                                    title VARCHAR(40),
                                    balance INT)
BEGIN
   DECLARE rid INT UNSIGNED;

   SELECT id IS NULL AS idisnull, id, id_person, title;
   
   IF id IS NULL THEN
      IF id_person > 0 THEN
         INSERT INTO Accounts (id_person, title)
               VALUES(id_person, title);
         SET rid = LAST_INSERT_ID();
      END IF;
   ELSE
      UPDATE Accounts
         SET Accounts.title = title,
             Accounts.balance = balance
       WHERE Accounts.id = id
         AND Accounts.id_person = id_person;

       SET rid = id;
   END IF;

   IF rid IS NOT NULL THEN
      CALL App_Person_Accounts(id_person, rid);
   END IF;
END $$

CREATE PROCEDURE App_Account_Delete(id INT UNSIGNED, id_person INT UNSIGNED)
BEGIN
   -- We will only delete an account without any transactions.  Otherwise
   -- the details will be orphaned.
   DECLARE tcount INT UNSIGNED;
   
   SELECT COUNT(*) INTO tcount
     FROM Transaction_Details t
          INNER JOIN Accounts a ON t.id_account = a.id
          INNER JOIN Person p ON a.id_person = p.id
    WHERE a.id = id
      AND p.id = id_person;

   IF tcount=0 THEN
      DELETE FROM Accounts
       WHERE Accounts.id = id
         AND Accounts.id_person = id_person;
       
      SELECT ROW_COUNT() AS deleted;
   ELSE
      SELECT 0 AS deleted;
   END IF;

END $$


DELIMITER ;
