#常见任务

## tar
   * tar.xz  外面是xz压缩方式，里层是tar打包方式
```bash
xz -d ***.tar.xz
tar -xvf  ***.tar
#或者
tar xvJf  ***.tar.xz
```
   * 查看tar文件下的所有文件（像ls一样）
```bash
 tar -tvzf stuff.tar.gz
 tar -tvf stuff.tar
```
   * 使用tar对文件加密
```bash
tar -zcvf - stuff|openssl des3 -salt -k secretpassword | dd of=stuff.des3
```
   * 使用tar对加密文件解压
```bash
dd if=stuff.des3 |openssl des3 -d -k secretpassword|tar zxf -
```

## date
   * date -d next-day +%Y%m%d #明天日期
   * date -d last-day +%Y%m%d #昨天日期
   * date -d @12345678 计算时间戳的日期
   * man date

## awk
指定多个分割符
命令行选项 -F"[:/]"  告诉awk  / 和 : 都是字段分隔符，即-F选项支持正则表达式


## rpm
   * rpm -q 
   查询系统已安装的软件(查看系统中所有已经安装的包，要加 -a 参数)
   * rpm -qf 
   查询一个文件属于哪个软件包
   * rpm -ql 
   查询已安装软件包都安装到何处
   * rpm -i 
   安装rpm包
   * rpm -qpl 
   列出RPM软件包内的文件信息[Query Package list]
   * rpm -qpi 
   查看一个软件包的用途、版本等信息
   * rpm -e 
   删除一个rpm包[erase]


## read
不用回车结束输入
   * read -n 10 var  输入10个字符
   * read -d “:”     以”:”结束输入
   * read -t 30     30s超时
   * read -s        关闭回显
   * 以上方式可以组合使用

## screen
   * 创建会话, 并attach:
$ screen -S abc
   * 会话列表:
$ screen -list
$ screen -ls
   * 连接/恢复会话:
$ screen -r abc
   * 暂时离开会话, 会话仍在后台运行:
CTRL+a+d
   * 停止会话(已连接的情况):
$ exit  或 ctrl+d

## script命令
script是将终端会话制成打印稿的命令。
想要script在登录时就生效，可以将其加在shell profile(~/.profile)。
`/usr/bin/script -qa /usr/local/script/log_record_script`
   * 选项-q是安静模式，用户登录时完全不会察觉到script命令的运行。
   * 选项-a是追加，不会覆盖以往的记录。

敲击ctrl+d或exit，可以停止记录。


## 均匀绑定cpu的shell
绑定指定程序均匀负载到各个CPU上
```bash
#!/bin/sh
pids=`/sbin/pidof $1`
cpunum=`cat /proc/cpuinfo | grep processor | wc -l`
cpuidx=0
for pid in $pids
do
    /usr/bin/taskset -cp ${cpuidx} ${pid}
    cpuidx=$(($cpuidx+1))
    cpuidx=$(($cpuidx%$cpunum))
    echo $cpuidx
done
```

## 检查命令执行是否成功
   * 常见写法
```bash
echo abcdee | grep -q abcd
 if [ $? -eq 0 ]; then
    echo "Found"
else
    echo "Not found"
fi
```
   * 简洁写法
```bash
if echo abcdee | grep -q abc; then
    echo "Found"
else
    echo "Not found"
fi
```
   * 不要if/else
```bash
echo abcdee | grep -q abc && echo "Found" || echo "Not found"
```


## 将man的内容干净的输出到一个文件里
`man tar | col -b > /tmp/tar.txt`

## 获取一个字符的ASCII码
* `man ascii`
附：man 报错的可能的原因
    * locale问题
    * 报错`conversion from utf8 unsupported`, 修改 /usr/bin/nroff脚本，把utf8 写成 utf-8（使用iconv -l检查支持的编码列表）

* `printf '%02x' "'+"`

* `echo -n '+' | od -tx1 -An | tr -d ' '`


## 显示进程树
pstree -p

## 根据进程名查找或kill进程
pgrep 和 pkill 来找到或是kill 某个名字的进程 (-f 选项很有用).

## 根据进程名找进程id
pidof _ProcessName_

## 让某个进程在后台运行
使用 nohup 或  disown 

## 日志监控
`tail -f /path/to/file.log | sed '/^Finished: SUCCESS$/ q'`
当file.log里出现Finished: SUCCESS时候就退出tail，这个命令用于实时监控并过滤log是否出现了某条记录。

不支持-f的tail命令可以使用watch命令。watch命令会过滤不可打印字符，可以考虑与cat –v连用。

## 获取登录的IP
`who -u am i 2>/dev/null| awk '{print $NF}'|sed -e 's/[()]//g'`

## 文本操作命令
   * sort uniq 
      * Stable sort (sort -s) 很有用
         * `sort -k1,1 | sort -s -k2,2` 将两例排序，先是以第二列，然后再以第一列
      * 按照tab分隔符进行字段排序
         * sort  -t $'\t' -k3,3n a.txt>a.sort
         * sort -t'<ctrl>v<tab>' -k3,3n a.txt>a.sort
         * <ctrl>v<tab>代表先同时按下Ctrl和v键，然后松开，按下tab键。
   * split 分隔一个大文件（split by size） csplit（split by a pattern）
   * 文件的集合操作
```bash
cat a b | sort| uniq > c   # c is a union b 并集
cat a b | sort| uniq -d > c   # c is a intersect b 交集
cat a b b | sort| uniq -u > c   # c is set difference a - b 差集
```
   * shuf    来打乱一个文件中的行或是选择文件中一个随机的行
   * shred

## AIX如何查询CPU个数
   * vmstat可以查看逻辑CPU个数
   * bindprocessor -q 
   * lsdev -Cc processor  可以查看物理CPU个数
   * prtconf或lsconf脚本可以查看物理CPU个数（查看脚本可以看到使用的方式
   * smtctl

## 删除文件的前N个字节
问题引出： libxml2不支持导入带有BOM的UTF-8格式的XML文件

`dd if=XXX.xml of=YYY.xml bs=1 skip=3`

## 删除BOM头
`sed -i 's/\xEF\xBB\xBF//' `

## 将printf格式化的结果赋值给变量
例如将数字转换成其十六进制形式
   * 常见的写法
`var=$(printf '%%%02x' 111)`
   * 简单的写法
`printf -v var '%%%02x' 111`

看看printf的help   
`help printf | grep -A 1 -B 1 -- -v`
```
printf: printf [-v var] format [arguments]
   Formats and prints ARGUMENTS under control of the FORMAT.
--
    Options:
      -v var    assign the output to shell variable VAR rather than
            display it on the standard output
```