--DROP DATABASE IF EXISTS SchemaDemo;
CREATE DATABASE IF NOT EXISTS SchemaDemo;
USE SchemaDemo;
SET storage_engine=InnoDB;

CREATE TABLE IF NOT EXISTS Person
(
   id           INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
   handle       VARCHAR(20) UNIQUE KEY NOT NULL UNIQUE,
   fname        VARCHAR(20) NOT NULL,
   lname        VARCHAR(30) NULL,
   
-- Think about using MySQL function SHA2() to encrypt passwords
-- in order to avoid having to deal with encryption in C++
-- password     VARCHAR(99) NULL,   

   index(handle)
);

CREATE TABLE IF NOT EXISTS Items
(
   id           INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
   id_owner     INT UNSIGNED NULL,
   id_possessor INT UNSIGNED NULL,
   description  VARCHAR(80),

   index(id_owner),
   index(id_possessor)
);
