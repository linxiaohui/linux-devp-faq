#SSH
## 在远程机器上运行一段脚本
`ssh user@server bash < /path/to/local/script.sh`
好处是不用把脚本拷到远程机器上。
## 比较一个远程文件和一个本地文件
`ssh user@host cat /path/to/remotefile | diff /path/to/localfile -`
## vim一个远程文件
`vim scp://username@host//path/to/somefile`
## stty : : invalid argument
ssh执行远程命令时显示 "stty : : invalid argument."

这是因为在bashrc中有stty命令。使用SSH执行远程命令使用的是非交互式SHELL(non-interactive shell)，stty会报错。

* sh
   * execute ~/.profile and ~/.bash_profile once at login
   * ~/.kshrc and ~/.bashrc when ever a new shell starts. 

* csh
   *  ~/.login on login
   *  ~/.cshrc when ever a new shell starts
   
要解决这个问题，在初始化脚本中寻找stty的地方，修改
```bash
# -t fd - True if file descriptor fd is open and refers to a terminal
if [ -t 0 ]; then
   echo interactive
   stty erase ^H
else
   #echo non-interactive
fi
```
或者直接忽略错误信息
```bash
stty erase ^H 2>/dev/null
```

**另：参考**

```bash
#check for interactivity:
case $- in
   *i* )   INTERACTIVE=1 ;;
esac
```
或者
```bash
if [ -t 255 ]; then INTERACTIVE=1 ; fi
```

## cat file | while read 在执行ssh远程命令的时只能执行一行

* 分析   
```bash
cat hostlist | while read i ;do sshpass -p 'anything' ssh -l root $i 'date';done
```
while read读取文件里面的内容, 然后执行ssh远程命令时会碰到while read只能对文件里面的第一行执行操作, 之后退出。

* 解决方案
   * 不使用cat, 利用`while read i;do ;done< filename.txt`代替。例如：
```bash
while read i;do sshpass -p 'anything' ssh -e '|' -l root $i 'date';done < hostlist
```
   * 使用for i in $();do;done代替cat | while read。例如：
```bash
for i in `cat hostlist`;do sshpass -p 'anything' ssh -l root $i 'date';done
```

如果没有文件只有echo返回的值呢？

* 根本原因:
   * 管道后相当于一个子程序,这里是while子程序。
   * SSH执行的远程命令则是while子程序的子程序。
   * `while read i;do<COMMANDS>;done<filename.txt`则只是在当前SHELL下开一个SSH子程序

* 解决办法
   * 子程序的子程序SHELL依旧支持,只是在子程序的子程序还没有获得结果的时候,**上一级子程序就已经退出了**.所以只能看到一行的返回结果.
   * 子程序包在()&里,并且后面跟`wait`,那么父程序要等到子程序完成后才退出.
   * 如果()& wait和do...done循环合用的话 &和wait之间不能加分号';'。
例如：
```bash
$ echo '
hostname1
hostname2
……
hostnamen' |while read i;do(sshpass -p 'anything' ssh -l root $i 'date')& wait ;done
```
这样，while子程序要等到()&里的所有程序结束才能退出
