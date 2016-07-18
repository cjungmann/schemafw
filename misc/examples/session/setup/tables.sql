CREATE DATABASE IF NOT EXISTS SessionDemo;
USE SessionDemo;
SET storage_engine=InnoDB;

CREATE TABLE IF NOT EXISTS Session_Info
(
   id         INT UNSIGNED NOT NULL PRIMARY KEY,
   id_account INT UNSIGNED NULL
);

CREATE TABLE IF NOT EXISTS Account
(
   id        INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
   handle    VARCHAR(20) NOT NULL UNIQUE,
   password  CHAR(32) NULL
);

CREATE TABLE IF NOT EXISTS Person
(
   id         INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
   id_account INT UNSIGNED NOT NULL,
   fname      VARCHAR(20),
   lname      VARCHAR(20),

   index(id_account)
);





