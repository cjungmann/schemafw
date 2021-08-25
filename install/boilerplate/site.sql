SET default_storage_engine=InnoDB;

-- ------------------------------
CREATE TABLE IF NOT EXISTS Person
(
   id       INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
   lname    VARCHAR(30),
   fname    VARCHAR(30),
   mname    VARCHAR(30),
   birthday DATE
);

DELIMITER $$

-- ----------------------------------------
DROP PROCEDURE IF EXISTS App_Person_List $$
CREATE PROCEDURE App_Person_List(id INT UNSIGNED)
BEGIN
   SELECT p.id, p.lname, p.fname, p.mname, p.birthday
     FROM Person p
    WHERE id IS NULL OR id = p.id;
END $$

-- ---------------------------------------
DROP PROCEDURE IF EXISTS App_Person_Add $$
CREATE PROCEDURE App_Person_Add(lname VARCHAR(30),
                                fname VARCHAR(30),
                                mname VARCHAR(30),
                                birthday DATE)
BEGIN
   INSERT INTO Person (`lname`, `fname`, `mname`, `birthday`)
   VALUES (lname, fname, mname, birthday);

   IF ROW_COUNT() > 0 THEN
      CALL App_Person_List(LAST_INSERT_ID());
   END IF;
END $$

-- ----------------------------------------
DROP PROCEDURE IF EXISTS App_Person_Read $$
CREATE PROCEDURE App_Person_Read(id INT UNSIGNED)
BEGIN
   SELECT p.id, p.lname, p.fname, p.mname, p.birthday
     FROM Person p
    WHERE id = p.id;
END $$

-- ------------------------------------------
DROP PROCEDURE IF EXISTS App_Person_Update $$
CREATE PROCEDURE App_Person_Update(id INT UNSIGNED,
                                   lname VARCHAR(30),
                                   fname VARCHAR(30),
                                   mname VARCHAR(30),
                                   birthday DATE)
BEGIN
   UPDATE Person p
      SET p.lname = lname,
          p.fname = fname,
          p.mname = mname,
          p.birthday = birthday
    WHERE p.id = id;

   IF ROW_COUNT() > 0 THEN
      CALL App_Person_List(id);
   END IF;
END $$

-- ------------------------------------------
DROP PROCEDURE IF EXISTS App_Person_Delete $$
CREATE PROCEDURE App_Person_Delete(id INT UNSIGNED,
                                   lname VARCHAR(30))
BEGIN
   DELETE FROM p USING Person AS p
    WHERE p.id = id
      AND p.lname = lname;

   SELECT ROW_COUNT() AS deleted;
END $$

DELIMITER ;
