CREATE DATABASE IF NOT EXISTS AllowanceDemo;
USE AllowanceDemo;
SET storage_engine=InnoDB;

CREATE TABLE IF NOT EXISTS Person
(
   id          INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
   nom         VARCHAR(20) NOT NULL,
   birthday    DATE,

   index(nom)
);

CREATE TABLE IF NOT EXISTS Accounts
(
   id          INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
   id_person   INT UNSIGNED NOT NULL,
   title       VARCHAR(40) NOT NULL,
   balance     INT NOT NULL DEFAULT 0
);

CREATE TABLE IF NOT EXISTS Transactions
(
   id          INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
   description VARCHAR(80) NOT NULL,
   tdate       DATE,

   index(tdate)
);

CREATE TABLE IF NOT EXISTS Transaction_Details
(
   id_transaction INT UNSIGNED NOT NULL,
   id_account     INT UNSIGNED NOT NULL,
   amount         INT NOT NULL DEFAULT 0,

   index(id_transaction),
   index(id_account)
);
