USE Allowances;

DELIMITER $$

-- Create the procedure with every MYSQL type for parameters:
DROP PROCEDURE IF EXISTS All_Types_Procedure $$
CREATE PROCEDURE All_Types_Procedure(
  p_tiny      TINYINT,
  p_utine     TINYINT UNSIGNED,
  p_short     SMALLINT,
  p_ushort    SMALLINT UNSIGNED,
  p_int       INTEGER,
  p_uint      INTEGER UNSIGNED,
  p_longlong  BIGINT,
  p_ulonglong BIGINT UNSIGNED,

  p_float     FLOAT,
  p_double    DOUBLE,

  p_decimal   DECIMAL(10,4),
  p_numeric   NUMERIC(10,4),

  p_datetime  DATETIME,
  p_date      DATE,
  p_time      TIME,
  p_timestamp TIMESTAMP,
  p_year      YEAR,

  p_varchar   VARCHAR(80),
  p_char      CHAR(50),

  p_enum      ENUM('one','two','three'),
  p_set       SET('one','two','three'),
  p_binary    BINARY(40),
  p_vbinary   VARBINARY(80),

  p_blob       BLOB,
  p_tinyblob   TINYBLOB,
  b_mediumblob MEDIUMBLOB,
  b_longblob   LONGBLOB,

  b_text       TEXT,
  b_tinytext   TINYTEXT,
  b_mediumtext MEDIUMTEXT,
  b_longtext   LONGTEXT
  
  )
BEGIN
END $$


-- Query to get a list of the types in the procedure:
SELECT PARAMETER_NAME,
       DATA_TYPE,
       DTD_IDENTIFIER
  FROM information_schema.PARAMETERS
 WHERE SPECIFIC_SCHEMA = 'Allowances'
       AND SPECIFIC_NAME = 'All_Types_Procedure';


-- Clean up (leave no trace):
DROP PROCEDURE IF EXISTS All_Types_Procedure $$


DELIMITER ;



