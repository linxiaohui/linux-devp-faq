# SYBASE

## sybase报错Client password encryption fails
在进行一个应用项目从AIX到linux的移植时，开发环境中的sybase使用 isql时，报错
```
ct__string_extended_encryption: user api layer: internal common library error: 
Client password encryption fails.
```
问题是在 OCS-15_0的lib3p，将其 LD_LIBRARY_PATH下问题解决

## 关于sybase的locale
linux下 locale设置为 zh_CN.gbk后，sybase的 isql 报错
```
Neither language name in login record 'chinese' nor language name in syslogins'<NULL>;' 
is an official language name on this SQL Server
```
如果将locale设置为C则相关的信息不能显示为中文。
解决方法之一是： 编辑 $SYBASE/locale/locale.dat 将  [linux] 下 locale为zh_CN.gbk的条目修改（参考C的条目）

**附**： 
```
Locale name
Open Client uses the values of the following POSIX environment variables as locale names 
(does not apply to DB-Library):
   * LC_ALL
   * LANG, if LC_ALL is not defined
Open Client later uses this value to obtain localization information from the locales.dat file. 
If LC_ALL, LANG, and sLanguage are not defined, Open Client uses "default" as the locale name.
```

## 在64位环境编译运行 SYBASE ESQL程序
编译Sybase ESQL 程序，运行时报错
```
cs_objects: cslib user api layer: external error： 
An illlegal value of 0 was given for parameter objname->lnlen.
```
因为编译的时候没有加参数 `-DSYB_LP64`

## sybase数据库报错-25018
 父进程打开了数据库子进程再打开数据库会报错.sqlcode=[-25018],errmsg=[Connection name in use.]

## 存储过程
   * sp_configure 检查数据库的配置
   * sp_lock 查看数据库中锁的情况


## 隔离级别
可以使用
```sql
select @@isolation
```
语句查看数据库的隔离级别
   0. Read Uncommitted
   事务在这种隔离级别下，允许读取/访问（看到）其他事务中尚未提交的数据修改。
   读取未提交的数据，也被称之为脏读（Dirty Read）。通常情况下，很少使用这种隔离级别。
   1. Read Committed
   在这种隔离级别下，事务只能访问其他事务已经提交的数据修改。
   "读提交"隔离级别虽然解决了“脏读”问题，但是可能会遇到"不可重复读"问题：
   "即一个事务在先后两次执行相同查询语句所读取到的数据值发生了不一致。
   这是由于在前后两次执行相同语句期间，其它的事务对数据进行了update或delete操作并且已经提交了事务而导致的"。例如：
  事务TA执行如下操作：
```
    begin transaction
      .....
      select balance from accounts where actno=1234567   --第一次读取
      .....
      select balance from accounts where actno=1234567  --第二次读取
      ....
    commit 
```
   事务TA在第1次执行查询时得到账号123567的余额是1000；当他第2次执行相同查询语句之前，事务TB执行了如下操作序列并提交：
```
   begin transaction
      ......
      update accounts set balance=balance+100 where actno=1234567
    commit
```
   这样一来，事务TA第2次执行相同查询时得到账号1234567的余额就是1100了，两次查询结果不一致，即遇到了"不可重复读"问题！
   此外，如果事务TB执行了delete accounts where actno=1234567，那么事务TA在第2次执行查询时就会找不到对应记录了。

   2. Repeatable Read
   在这种隔离级别下，事务可以重复多次执行同一查询，并且读到的结果集中的任何一行记录都不会被其他事务更新或删除。
   这种隔离级别虽然解决了"不可重复读"问题，但是可能会遇到"幻像读 （Phantom Read）"问题：
   "即当事务读取满足某一范围条件的数据行时，另一个事务又在该范围内插入了新行，
   当事务再次读取该范围的数据行时，会发现有新的"幻像" 行"。例如：
   事务TA执行如下操作：
```
     begin transaction
       .....
       --第一次读取
       select count(*) from accounts where balance>=1000 and balance <=3000  
       .....
       select count(*) from accounts where balance>=1000 and balance <=3000
       ....
     commit 
```
   事务TA在第1次执行查询时得到账户余额在1000和3000之间的账户数，假设为500；
   当他第2次执行相同查询语句之前，事务TB执行那个了如下事务并提交：
```
   begin transaction
     ......
     insert into accounts(actno, balance) values (2222222,2000)
   commit
```
   这样一来，事务TA第2次执行相同查询时得到账户余额在1000和3000之间的账户数就变为501个了，这样两次查询结果不一致！
   这种不一致是由于insert语句扎入了满足范围条件的新记录行导致的（注意与"不可重复读"的区别）。
   3. Serializable
   "可串行化"是ANSI SQL中定义的最高事务隔离级别。在这种隔离级别下，事务可以重复多次执行相同查询，
   并且每次都能够得到完全相同的结果。在这种隔离级别下，其他事务不能插入任何将出现在结果集中的记录行，
   从而可以解决"幻像读"问题（参见前面有关"幻像读"的例子）。

