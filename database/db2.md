#DB2

## 
```bat
rem /* EXEC SQL CONNECT TO :DBNAME USER :DBUSER USING :DBPASS; */
rem EXEC SQL CONNECT TO SAMPLE;
LIB=C:\PROGRA~1\IBM\SQLLIB\LIB
db2 prep example.sqc bindfile
cl example.c /linkdb2api.lib
db2 bind example.bnd
```