USE ImportDemo;

CREATE TABLE IF NOT EXISTS Session_Import_People
(
   id_session INT UNSIGNED,
   id_family  VARCHAR(15),
   id_student VARCHAR(15),
   lname      VARCHAR(25),
   fname      VARCHAR(25),
   grade      VARCHAR(3),
   homeroom   VARCHAR(30),
   phone      VARCHAR(15),
   email1     VARCHAR(180),
   email2     VARCHAR(180)
);

