#SHELL基础

## 查看当前系统支持哪些shell
   * `cat /etc/shells`
   * `echo $SHELL`   显示当前shell
   * `chsh -s`       更改登录shell
   * `echo $0`       显示当前shell名称
   * **注**: 若在bash中执行ksh，之后echo $SHELL与echo $0显示不一样

## bash执行顺序
   1. 别名：alias
   2. 函数：function
   3. 内建命令：built in
   4. 外部命令：command
`\COMMAND` 不把COMMAND作为alias。 `type COMMAND`可以检查COMMAND的类型。

详请`man bash`查看其中`COMMAND EXECUTION`章节


**备注**: 实际上bash遇到名字解析的顺序是: 别名-关键字-函数-內建命令-hash-外部命令
关于hash `man bash`搜索hash


## 各种方式执行shell脚本的区别
   1. .  _script_ / source _script_
   Shell內建命令，将_script_在当前Shell进程中执行
   2. bash  _script_
   将_script_传给bash执行，其中bash是当前shell的子进程
   3. ./_script_
   调用exec执行 _script_，内核在检查文件以`#!`开始后将启动其定义的interpreter, 参数为 _script_。
   对interpreter的要求
   ```
   The interpreter must be a valid pathname for an executable which is not itself a script.
   ```
   即任一可执行的二进制程序都可以作为 interpreter, 不限于bash, python等。

   `man execve`

## 查看bash内置命令
   * `help`

## bash组合键
   * `man readline`

## shopt
在bash4.0以上版本中，如果bash环境开启了globstar设置，那么两个连续的**可以用来递归匹配某目录下所有的文件名。
shopt -s expand_aliases

## sudo
   * `/etc/sudoers`
   * 使用`visudo`, 修改上述文件
   * 注意`su `与`su - `的区别: 前者只切换用户, 不改变环境变量
   * 不需要输入密码 `user_name ALL=(ALL) NOPASSWD:ALL`
   * 定制密码过期时间 `Defaults    env_reset,timestamp_timeout=180`

## 返回值
如果bash脚本没有调用`exit 返回值`的话，其最后执行命令的返回值将成为bash脚本的返回值。
   * 0 正确

## shell超时
   * A.
```python
def command_run(command,timeout=10):
    proc = subprocess.Popen(command,bufsize=0,stdout=subprocess.PIPE,
    stderr=subprocess.PIPE,shell=True)
    poll_seconds = .250
    deadline = time.time() + timeout
    while time.time() < deadline and proc.poll() == None:
        time.sleep(poll_seconds)
    if proc.poll() == None:
        if float(sys.version[:3]) >= 2.6:
            proc.terminate()
    stdout,stderr = proc.communicate()
return stdout,stderr,proc.returncode
```
   * B.
```bash
function timeout() {
waitsec=5
( $* ) & pid=$!
( sleep $waitsec && kill -HUP $pid ) 2>/dev/null & watchdog=$!
# if command finished
if wait $pid 2>/dev/null; then
    pkill -HUP -P $watchdog
    wait $watchdog
fi
# else: command interrupted
}
timeout sleep 10
```
   * C. SUSE coreutils包中有`timeout`命令

## here document
here document往往用于需要输出一大段文本的地方, 例如脚本的help函数.

下面一段代码
```bash
grep -v 1 /tmp/test.txt | while read line; do
    let a++
    echo --$line--
done
echo a:$a
```
执行后可以发现a这个变量没有被赋值。

**因为管道后面的代码是在在一个子shell中执行的, 所做的任何更改都不会对当前shell有影响**，自然a这个变量就不会有赋值了。
换一种思路, 可以这样做
```bash
grep -v 1 /tmp/test.txt > /tmp/test.tmp
while read line; do
    let a++
    echo --$line--
done < /tmp/test.tmp

echo a:$a
rm -f /tmp/test.tmp
```
不过多了一个临时文件, 最后还要删除

其实可以用到here document
```bash
while read line2; do
    let b++
    echo --$line2--
done << EOF
`grep -v 1 /tmp/test.txt`
EOF
echo b: $b
```
上面的`EOF`被称为limit string
   * limit string前加`-`号表示suppress tab(不会suppress space)
   * limit string用`'`,`"`或"\\"转义可以禁用here document中变量替换

## Here String
`cmd <<<$var`,$var将被展开并作为cmd的标准输入.

## 如何调试
在bash的脚本中
   * 以使用 set -x 来debug输出
   * 使用 set -e 来当有错误发生的时候abort执行
   * 使用 set -o pipefail 来限制错误
   * 可以使用trap来截获信号（如截获ctrl+c）
可以直接bash -x _script_ 来执行并打印调试信息

## Shell等待
   * suspend
   bash还提供了一种让bash执行暂停并等待信号的功能，就是suspend命令。它等待的是18号SIGCONT信号，这个信号本身的含义就是让一个处在T（stop）状态的进程恢复运行。使用方法：

```bash
   #!/bin/bash
   pid=$$
   echo "echo $pid"
   #打开jobs control功能，在没有这个功能suspend无法使用，脚本中默认此功能关闭。
   #我们并不推荐在脚本中开启此功能。
   set -m
   echo "Begin!"
   echo $-
   echo "Enter suspend stat:"
   #让一个进程十秒后给本进程发送一个SIGCONT信号
   ( sleep 10 ; kill -18 $pid ) &
   #本进程进入等待
   suspend
   echo "Get SIGCONT and continue running."
   echo "End!"
```

## 函数返回值
函数的返回值默认是最后一行语句的返回值

## subshell
在bash 脚本中，subshells (写在圆括号里的) 是一个很方便的方式来组合一些命令。
一个常用的例子是临时地到另一个目录中，例如：
```bash
# do something in current dir
(cd/some/other/dir; other-command)
# continue in original dir
```

## 把某命令的输出当成一个文件
通过 <(_commandline_) 可以把某命令当成一个文件。
例如： 比较一个本地文件和远程文件 /etc/hosts： `diff /etc/hosts <(ssh somehost cat /etc/hosts)`

## Shell中的多进程
   * 在命令行下, 在命令行后面加上&符号来让该命令在后台执行
   * 在shell脚本中, 使用`(_cmd_)`可以fork一个子shell来执行_cmd_
利用这两点, 可以实现shell的多进程
```bash
job_num=10
 function do_work()
{
    echo "Do work.."
}
 for ((i=0; i < job_num ;i++)); do
    echo "Fork job $i"
    (do_work) &
done
 wait   # wait for all job done
echo "All job have been done!"
```
注意最后的wait命令, 作用是等待所有子进程结束.

## bash参数
bash中--后面的参数不会被当作选项解析.

# 重定向
## `1> /dev/null 2>&1`解释
   * 1代表标准输出，2代表错误信息输出.
   * `1>/dev/null` 就是指将标准输出定向到空设备，
   * `2>&1`的意思是将错误输出定向到和1一样的输出设备，也同样是空.

## `cmd >a 2>a`和`cmd >a 2>&1`为什么不同？
   * `cmd >a 2>a` stdout和stderr都直接送往文件 a, a文件会被打开两遍，由此导致stdout和stderr互相覆盖。
   * `cmd >a 2>&1` stdout直接送往文件a ，stderr是继承了FD1的管道之后，再被送往文件a 。a文件只被打开一遍，就是FD1将其打开
他们的不同点在于：
   * `cmd >a 2>a` 相当于使用了FD1、FD2两个互相竞争使用文件 a 的管道；
   * 而`cmd >a 2>&1` 只使用了一个管道FD1，但已经包括了stdout和stderr。
   * 从IO效率上来讲，`cmd >a 2>&1`的效率更高。

## 简洁写法
   * `grep "abc" test.txt &> /dev/null`  

**备注**: crontab中简洁写法可能无效  

## 管道与标准错误
**因stderr是不参与pipe的** (仍然会被输出)
使用  
   * `CMD 2>&1 1>/dev/null | grep XXX` 这样只grep stderr （注意顺序）
   * `CMD 2>&1 | grep XXX ` 同时grep stdout和stderr
   * `CMD |& grep XXX`  将CMD的标准输出和标准报错都与grep的标准输入连接.
   * `|&`的写法BASH-3.2不支持; BASH-4.2支持
   * **注意**, stderr与stdout的输出顺序, 测试发现stderr总是在stdout之前

## Shell 文件读写

复制输入文件描述符：`[n]<&fd`
   * 如果n没有指定数字, 则默认为0
   * fd: 已经打开的文件描述符
   * 如果fd是`-`则表示关闭这个文件描述符

复制输出文件描述符：`[n]>&fd`
   * 同上

移动输入描述符：`[n]<&digit-`
   * 将原有描述符在新的描述符编号上打开，并且关闭原有描述符。

移动输出描述符：`[n]>&digit-`
   * 同上

新建输入描述符：`[n]<path`

新建输出描述符：`[n]>path`

新建输入和输出的描述符：`[n]<>path`
   * `path`应该写一个文件路径，用来表示这个文件描述符的关联文件是谁。

示例
```bash
#!/bin/bash
# example 1
#打开3号fd用来输入，关联文件为/etc/passwd
exec 3< /etc/passwd
#让3号描述符成为标准输入
exec 0<&3
#此时cat的输入将是/etc/passwd，会在屏幕上显示出/etc/passwd的内容。
cat
#关闭3号描述符。
exec 3>&-
# example 2
#打开3号和4号描述符作为输出，并且分别关联文件。
exec 3> /tmp/stdout
exec 4> /tmp/stderr
#将标准输入关联到3号描述符，关闭原来的1号fd。
exec 1>&3-
#将标准报错关联到4号描述符，关闭原来的2号fd。
exec 2>&4-
#这个find命令的所有正常输出都会写到/tmp/stdout文件中，错误输出都会写到/tmp/stderr文件中。
find /etc/ -name "passwd"
#关闭两个描述符。
exec 3>&-
exec 4>&-
```

**注意**
一般输入输出重定向都是放到命令后面作为后缀使用，所以如果改变脚本的描述符，需要在前面加exec命令。
这种用法也叫做描述符魔术。

## 特殊用法
`> /tmp/out` 表示清空文件，也可以写成`:> /tmp/out`
`:`是一个内建命令, 跟true是同样的功能, 没有任何输出, 所以这个命令清空文件的作用.

示例:
```bash
echo 1234567890 > tmp
exec 3<> tmp
read -n 4 <&3
echo -n . >&3
exec 3>&-
cat tmp
```
输出为 1234.67890

**注意**: `3<>`, `<&3`, `&>3`,`3>&` 均不能有空格


## 受限的shell
set -r

## 后台作业
Shell执行命令后，Ctrl+Z挂起程序执行(注意不是到后台执行)，并会提示作业号。此时可以使用`jobs`查看，使用`bg _作业号_`使其后台执行，`fg _作业号_`使其继续前台执行

后台作业有两个特点：
   1. 继承当前session的标准输出和标准错误
   2. 不再继承当前session的标准输入；如果其试图读取标准输入，就会等待

退出session，系统向该session发出SIGHUP信号，session将SIGHUP信号发给所有子进程，子进程收到SIGHUP信号后，自动退出。
后台作业是否收到SIGHUP信号取决于`shopt | grep huponexit`参数。这个参数为off时，session退出的时候不会把SIGHUP信号发给后台任务。但如果如果后台进程与标准I/O有交互，其会退出。使用`disown`命令也会有该问题。解决办法是对后台任务的标准输入输出进行重定向.

*备注: 实验时发现关于huponexit的描述只对登录交互session适用*

一般情况下应该使用`nohup`命令. 通过`strace nohup 命令`可以发现。nohup将标准输入重定向到`/dev/null`, 将标准输出/标准错误重定向到 nohup.out，忽略了SIGHUP信号。
需要注意的是，nohup命令不会自动把进程变为后台作业，必须使用`&`。
在当shell中提示了nohup成功后，还需要按终端上键盘任意键退回到shell输入命令窗口，然后通过在shell中输入exit来退出终端


# Shell与网络
Linux有类比较特殊的文件`/dev/[tcp|upd]/host/port` 读取或者写入这类文件, 相当尝试连接`host:port`. 如果主机以及端口存在, 就建立一个socket 连接.

示例
```bash
#从时间服务器读取时间
cat</dev/tcp/time-b.nist.gov/13
```

```bash
#通过重定向读取远程web服务器头信息
# $1为HOST
# $2位PORT

#打开host的port 可读写的socket连接，与文件描述符6连接
exec 6<>/dev/tcp/$1/$2

if(($?!=0));then
    echo "open $1 $2 error!";
    exit 1;
fi

echo -e "HEAD / HTTP/1.1\n\n\n\n\n">&6;

cat<&6;
#从socket读取返回信息，显示为标准输出
exec 6<&-;
exec 6>&-;
#关闭socket的输入，输出
exit 0;
```

## 如何在Shell中输入 TAB
Ctrl+V,Tab
