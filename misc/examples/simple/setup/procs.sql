-- This script file includes four procedures that provide the
-- basic needs of managing a record (create new, edit or delete
-- existing) and showing a set of the records.
-- 
-- Use these four procedures as a guide and example for setting
-- up your own application.
-- 
-- App_Person_Collection : renders a table,
-- App_Person_Values     : retrieves the field values of a form,
-- App_Person_Submit     : saves the form values of new or edit record,
-- App_Person_Delete     : deletes the item.

DELIMITER $$

-- This 'table' procedure serves two purposes.  It will return the
-- entire set of data if id==NULL, and it will return a single item
-- as identified by id if id!=NULL.  The second return type allows
-- the application to replace or add a table line after an item edit
-- or addition.
DROP PROCEDURE IF EXISTS App_Person_Collection $$
CREATE PROCEDURE App_Person_Collection(id INT UNSIGNED)
BEGIN
   SELECT p.id,
          p.handle,
          p.fname,
          p.lname
     FROM Person p
    WHERE id IS NULL
          OR p.id = id;
END $$


-- This procedure returns the fields necessary to populate a
-- form from an existing item.  For new items, simple use the
-- App_Person_Submit procedure to get a form schema.
DROP PROCEDURE IF EXISTS App_Person_Values $$
CREATE PROCEDURE App_Person_Values(id INT UNSIGNED)
BEGIN
   SELECT id,
          handle,
          fname,
          lname
     FROM Person p
    WHERE p.id = id;
END $$

-- This procedure UPDATEs or INSERTs the record according
-- to the value of id.  If id==NULL, a new record is created,
-- otherwise the record identified by id will updated with the
-- procedure parameters.
DROP PROCEDURE IF EXISTS App_Person_Submit $$
CREATE PROCEDURE App_Person_Submit(id INT UNSIGNED,
                                   handle VARCHAR(20),
                                   fname VARCHAR(20),
                                   lname VARCHAR(30))
BEGIN
   DECLARE rid INT UNSIGNED;
   
   IF id IS NULL THEN
      INSERT INTO Person (handle, fname, lname)
      VALUES (handle, fname, lname);
      
      SET rid = LAST_INSERT_ID();
   ELSE
      UPDATE Person p
         SET p.handle = handle,
             p.fname = fname,
             p.lname = lname
       WHERE p.id = id;

       SET rid = id;
   END IF;

   CALL App_Person_Collection(rid);
END $$


-- Deletes a single record as identified by id.
-- For the default, built-in delete, the framework client
-- code looks for results that look like the results of
-- 'SELECT ROW_COUNT() AS deleted' so the client can know
-- if the procedure had any effect.
--
-- It is not required to follow this example, but if you
-- don't, you will have to make corresponding changes to
-- the specs file mode that calls this procedure.
DROP PROCEDURE IF EXISTS App_Person_Delete $$
CREATE PROCEDURE App_Person_Delete(id INT UNSIGNED)
BEGIN
   DELETE FROM Person
    WHERE Person.id = id;

   SELECT ROW_COUNT() AS deleted;
END $$

-- This procedure is a root-procedure that provides extra
-- attributes for the document element.
--
-- It will be called, if specified, without any parameters.
-- It should return a single record, whose values should be
-- either session variables, function results, or values that
-- can be accessed if a session is active by using the
-- @session_confirmed_id value to use in a query (within a
-- stored procedure, of course).
DROP PROCEDURE IF EXISTS App_Person_Root $$
CREATE PROCEDURE App_Person_Root()
BEGIN
   SELECT NOW() AS `date`,
          SCHEMA() AS `dbase`,
          CURRENT_USER() AS `suser`,
          'Test & ''no-test''' AS `escape`,
          USER() AS `cuser`;
END $$



DELIMITER ;
