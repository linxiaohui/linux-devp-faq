#bash历史记录表达式和参数符号

## 重复执行上次的命令
   * !! 
   * 如 sudo !!

## 执行上一个以XXX开头的命令
   * !XXX 
   
## history查看命令
   * !n 执行第n条命令
   * !-n 执行倒数第n条命令

##命令参数
   * !$ 上一条命令的最后一个参数
   * !^ 上一条命令的第一个参数
   * !:n上一条命令的第n个参数

## 历史和参数组合使用
参数的符号都是可以和!表达式任意组合使用的
   * cd !762:2 （表示762号历史命令的第2个参数）
   * ls !-3^ （表示倒数第3个命令的第一个参数）
   * dpkg -L !apt$ (表示搜索含apt的命令的最后一个参数）

## Magic-Space
虽然历史记录表达式和参数符号使用起来简易方便，但是在包含这些表达式和符号的命令回车执行之前，
并不知道这些表达式和符号到底代表的什么。为了解决这个问题，可以使用Magic-Space
在bash中执行
```bash
bind Space:magic-space
```
后，使用了历史表达式或参数符号后按空格，命令行即变为相应的内容。

## 搜索命令历史，但不执行
$ !whatever:p

## 命令置换
可以使用 ^texttosobstitute^substitution 来置换上一条命令中的texttosobstitute为substitution。 
也可以使用 !!:gs/old/new

## 命令历史隐藏
在命令前加空格，就可以避免改该命令计入命令历史

## 不执行并保留命令
若输入一条命令后不想立即执行但又想保留这条命令, 可以使用 `Ctrl+A, #`将这条命令作为注释后回车。
之后使用历史命令找回并编辑。或者使用 `Alt+#`(经测试在putty和GNOME Terminal中可以使用，在SecureCRT中不可以)。



## 浏览历史命令
在 bash 里如果系统的历史记录太多的话，可以通过 ctrl+r 来查找命令或者通过 history 命令来浏览历史命令。

## 统计最常用的十条命令
```bash
history | awk '{CMD[$2]++;count++;} END { for (a in CMD )print CMD[a] " " CMD[a]/count*100 "% " a }' \
| grep -v "./" | column -c3 -s " " -t | sort -nr | nl | head -n10
```
或者
```bash
history|awk ‘{print $2}’|awk ‘BEGIN {FS=”|”} {print $1}’|sort|uniq -c|sort -rn|head -10)
```

## man bash
   * HISTTIMEFORMAT
   * HISTFILE
   * HISTSIZE 
   * HISTIGNORE 
   * HISTCONTROL 

## Bash通过上下键快速查找历史命令
在用户$HOME目录下新建一个 .inputrc 文件
```bash
"\e[A": history-search-backward
"\e[B": history-search-forward
set show-all-if-ambiguous on
set completion-ignore-case on
```
退出 bash 后重新登陆，输入一个字母或者几个字母，然后 按"上下" 键，就会看到以这个字母搜索到的完整命令行。
如果搜索到几个类似命令，通过上下键来切换
