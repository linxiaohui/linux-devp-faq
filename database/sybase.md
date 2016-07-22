# SYBASE

## sybase报错Client password encryption fails
在进行一个应用项目从AIX到linux的移植时，开发环境中的sybase使用 isql时，报错
```
ct__string_extended_encryption: user api layer: internal common library error:
Client password encryption fails.
```
问题是在 OCS-15_0的lib3p，将其配置到LD_LIBRARY_PATH中问题解决

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

sybase数据库通常隔离界别设置为1; 值得注意的是通过jdbc连接数据库上或ESQL编译默认隔离界别可能与之不一样.

   0. Read Uncommitted
   事务在这种隔离级别下，允许读取/访问（看到）其他事务中尚未提交的数据修改。
   读取未提交的数据，也被称之为脏读（Dirty Read）。排他锁在对数据库进行写操作后立即释放，不会持有到事务提交或回滚。
   1. Read Committed
   在这种隔离级别下，事务只能访问其他事务已经提交的数据修改。排他锁持有到事务提交或回滚，但共享锁在加载数据到内存后立即释放。
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
   在这种隔离级别下，事务可以重复多次执行同一查询，并且读到的结果集中的任何一行记录都不会被其他事务更新或删除。排它锁和共享锁都会持有到事务结束，查询结果集不可以删除和修改，但是可以插入。
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
   从而可以解决"幻像读"问题（参见前面有关"幻像读"的例子）。排它锁和共享锁都会持有到事务结束，查询结果集不可以增删改。


## 隔离级别对死锁的影响
隔离级别同样会对锁定有很大的影响，例如，

### 情形一

| 执行顺序	| T1	| T2 |
|----------| ----| ----|
| 1	       | 排他锁A |  |
| 2	 |   | 	排他锁B |
|3	| 共享锁B |  |	 
| 4	|  	  |  共享锁A |

当隔离级别为0时，不会出现死锁。当隔离界别为1,2,3时，则会发生死锁。

### 情形二

| 执行顺序	| T1	| T2 |
|----------| ----| ----|
| 1	       | 共享锁A |  |
| 2	 |   | 	共享锁B |
|3	| 排他锁B |  |	 
| 4	|  	  |  排他锁A |

当隔离级别为0,1时，不会出现死锁。当隔离界别为2,3时，会发生死锁。

### 情形三

该情况是最近在系统中发现的一个死锁问题。程序从文件导入数据到数据库中，每次导入一条记录时，首先尝试以update的方式导入一条记录，当找到记录为空时，则将该条记录更改为以insert的方式导入到数据库中。
同时，导入过程是由多个进程共同完成的，每个进程导入一个文件，多个进程同时工作，然而当程序运行时，多个进程同时导入出现死锁。
通过监控sybase日志，发现死锁都是发生在insert时，出现next-key lock。sybase日志保存在安装目录下，例如安装目录为/sybase/ASE-12_5，日志文件为/sybase/ASE-12_5/install/db_name.log。
通过检查数据库的隔离级别，为1，没有发现异常，百思不得其解。
后在程序中添加查询数据库隔离级别语句，以检查在程序运行中到底隔离级别是多少？
经检查，隔离级别为3，也就是说在事务中，不能插入任何将出现在结果集中的行，下面分析一下出现死锁的原因。
当两个进程同时插入记录到同一个间隙中时，每个事务可能由两个操作组成
1.update
2.insert，当update结果集为空时，则转为insert。
其执行过程中，两个进程可能出现以下运行情况

| 执行顺序	| T1	| T2 |
|----------| ----| ----|
| 1	       | 共享锁A |  |
| 2	 |   | 	共享锁B |
|3	| 排他锁B |  |	 
| 4	|  	  |  排他锁A |

例如，目前数据库只有一条记录，主键为5，此时T1，T2分别插入主键为3,4的数据，由于两个事务都在运行之中，因此T1，T2都会尝试在5之前插入数据，首先其在update时，会产生共享锁，由于隔离级别为3，此时两个事务尝试插入时都会失败，要解决这种死锁，可以在程序中显式设置隔离级别为1。

## sybase锁升级
sybase同时提供锁升级的功能，例如将行锁升级为页锁，将页锁升级为行锁。具体参数可以进行设置。
例如当某一页中90%的行都被锁定，那么此时sybase可能将这些多个行锁升级为一个页锁，锁定整个页。这也是造成死锁一个重要的原因。
有时，根据判断，不会产生死锁。

| 执行顺序	| T1	| T2 |
| ---------| -----| ----|
|1	| 行级排他锁A |  |
|2	|  	| 行级排他锁B |
|3	|行级排他锁C	|  |
|4	|  |	行级排他锁D |

在上述情况中，如果没有锁升级机制，是无论如何也不会产生死锁的。但是当有了锁升级机制之后，可能T1在将行级锁A升级为页锁Pa，T2将行锁B升级为页锁Pb，而T1需要访问的行C在页Pb中，T2需要访问的D在也Pa中，这时就会构成一个锁定环，构成死锁。
