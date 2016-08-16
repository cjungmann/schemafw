# Advanced Case Study Data Definition

The data environment described on this page is used by several advanced topics pages.

## The Tables

~~~sql
CREATE TABLE IF NOT EXISTS Person
(
   id    INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
   fname VARCHAR(32),
   lname VARCHAR(32)
);

CREATE TABLE IF NOT EXISTS Phone
(
   id      INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
   pnumber VARCHAR(11),
);

CREATE TABLE IF NOT EXISTS Phone2Person
(
   id_person INT UNSIGNED NOT NULL,
   id_phone  INT UNSIGNED NOT NULL,
   INDEX(id_person),
   INDEX(id_phone)
);
~~~

## Procedures

~~~sql
-- Standard List procedure that includes a line-update path:
CREATE PROCEDURE App_Person_Table(person_id INT UNSIGNED)
BEGIN
   SELECT id, fname, lname
     FROM Person
    WHERE person_id IS NULL
       OR id = person_id;
END $$

-- Standard Create procedure, returning new record if successful
CREATE PROCEDURE App_Person_Create(fname VARCHAR(32), lname VARCHAR(32))
BEGIN
   INSERT
     INTO Person(fname, lname)
     VALUES(fname, lname);

   IF ROW_COUNT() = 1 THEN
      CALL App_Person_Table(LAST_INSERT_ID());
   END IF;
END $$

-- Standard Update procedure, returning updated recorded if successful
CREATE PROCEDURE App_Person_Update(id INT UNSIGNED,
                                   fname VARCHAR(32),
                                   lname VARCHAR(32))
BEGIN
   UPDATE Person p
      SET p.fname = fname,
          p.lname = lname
    WHERE p.id = id;

   IF ROW_COUNT() = 1 THEN
      CALL App_Person_Table(id);
   END IF;
END $$

-- Standard Delete procedure, returning flag to indicate success if appropriate.
-- Includes several parameters to hinder mistaken or malicious deletion calls
CREATE PROCEduRE App_Person_Delete(id INT UNSIGNED,
                                   fname VARCHAR(32),
                                   lname VARCHAR(32))
BEGIN
   -- Use alternate alias form to disambiguate the field names:
   DELETE
     FROM p USING Person AS p
    WHERE p.id = id
      AND p.fname = fname
      AND p.lname = lname;

   SELECT ROW_COUNT() AS deleted;      
END $$
~~~

                                   
