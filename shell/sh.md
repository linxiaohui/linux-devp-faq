#SHELL基础

## 查看当前系统支持哪些shell
   * `cat /etc/shells`
   * `echo $SHELL`   显示当前shell
   * `chsh -s`       更改登录shell
   * `echo $0`       显示当前shell名称
   * **注**: 若在bash中执行ksh，之后echo $SHELL与echo $0显示不一样

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
下面一段代码
```bash
grep -v 1 /tmp/test.txt | while read line; do
    let a++
    echo --$line--
done
echo a:$a
```
执行后有什么问题吗?
[Sun Nov 04 05:35 AM] [kodango@devops] ~/workspace 
$ sh test.sh 
--2--
--3--
a:
发现a这个变量没有被赋值, 为什么呢? 因为管道后面的代码是在在一个子shell中执行的, 所做的任何更改都不会对当前shell有影响, 自然a这个变量就不会有赋值了.
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
不过多了一个临时文件, 最后还要删除. 这里其实可以用到here document
```bash
while read line2; do
    let b++
    echo ??$line2??
done << EOF
`grep -v 1 /tmp/test.txt`
EOF
echo b: $b
```
here document往往用于需要输出一大段文本的地方, 例如脚本的help函数.

## 如何调试
在bash的脚本中，你可以使用 set -x 来debug输出。使用 set -e 来当有错误发生的时候abort执行。
考虑使用 set -o pipefail 来限制错误。还可以使用trap来截获信号（如截获ctrl+c）

## 函数返回值
函数的返回值默认是最后一行语句的返回值

## 在bash 脚本中，subshells (写在圆括号里的) 是一个很方便的方式来组合一些命令。
一个常用的例子是临时地到另一个目录中，例如：
```bash
# do something in current dir
(cd/some/other/dir; other-command)
# continue in original dir
```

## 通过 <(some command) 可以把某命令当成一个文件。
例如： 比较一个本地文件和远程文件 /etc/hosts： diff /etc/hosts <(ssh somehost cat /etc/hosts)

## Shell中的多进程
在命令行下, 我们会在命令行后面加上&符号来让该命令在后台执行, 在shell脚本中, 使用”(cmd)”可以让fork一个子shell来执行该命令. 利用这两点, 可以实现shell的多进程
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
bash中-后面的参数不会被当作选项解析.
   
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
   * `grep "abc" test.txt 2> /dev/null`   
**备注**: crontab中简洁写法可能无效  

## grep stderr
**因stderr是不参与pipe的**
使用  
   * `CMD 2>&1 1>/dev/null | grep XXX` 这样只grep stderr （注意顺序）
   * `CMD 2>&1 | grep XXX ` 同时grep stdout和stderr

