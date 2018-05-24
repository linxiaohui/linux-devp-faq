#终端


## login shell与non-login shell
   * 登录shell 是通过输入用户名密码登录得到的shell。如ssh或telnet登录进入的shell。
   * 非登陆shell如在图形系统下打开控制台。或直接在命令行里输入/bin/csh 之类切换得到shell。属于非登录shell。
   * 如果在命令行里输入 /bin/bash --login. 那么这时候得到的就是登录shell

读取的配置文件数据并不一样。

* login shell会读配置文件
   * /etc/profile
   * ~/.bash_profile或~/.bash_login或~/.profile
      * 如果~/.bash_profile存在，那么其他两个文件不论有无存在，都不会被读取
      * 如果~/.bash_profile不存在才会去读取 ~/.bash_login
      * 而前两者都不存在才会读取~/.profile
* non-login shell会读配置文件：
   * ~/.bashrc

## 交互式Shell和非交互式Shell
使用shell输入命令得到结果的方式是交互式的方式，而shell脚本使用的是非交互式方式
在交互式模式下，shell的alias扩展功能是打开的，在非交互式模式下alias扩展功能默认是关闭的，



## 如何查看当前是终端还是伪终端
   * 使用ps命令
       * 终端类型TTY
       * 在ssh或telnet登录的控制台使用ps查看为PTS，p=pseudo意思

## 终端设备当文件使用

例如可以使用`echo 1 > /dev/tty1 ` 或 `echo 2 > /dev/pts/1 `直接显示消息到终端上去，可以将消息输出到别的控制台上去。
```bash
for cons in `w | grep $LOGNAME | awk '{printf("/dev/%s\n", $2)}'`;
do echo "TEST" > $cons;
done
```
## Server refused to allocate pty
```bash
mknod -m 666 /dev/ptmx c 5 2
#chmod 666 /dev/ptmx
mkdir /dev/pts
#编辑/etc/fstab，加入:
none /dev/pts devpts gid=5,mode=620 0 0
mount /dev/pts
```

## ANSI颜色汇总
要在终端输出带颜色的内容时，可以使用ANSI颜色设定。
   * `\033[#m` 为样式
   * `\033[3#m` 为前景色
   * `\033[4#m` 为背景色
   * `\033[1;3#m` 为粗体前景色。

详细配色如下：
```
\033[0m关
\033[1m粗体\033[0m
\033[2m无\033[0m
\033[3m无\033[0m
\033[4m下划线\033[0m
\033[5m闪烁\033[0m
\033[6m无\033[0m
\033[7m反显\033[0m
\033[8m消隐\033[0m
\033[9m无\033[0m

\033[30m黑\033[0m
\033[31m酱红\033[0m
\033[32m浅绿\033[0m
\033[33m黄褐\033[0m
\033[34m浅蓝\033[0m
\033[35m紫\033[0m
\033[36m天蓝\033[0m
\033[37m灰白\033[0m

\033[1;30m浅黑\033[0m
\033[1;31m红\033[0m
\033[1;32m绿\033[0m
\033[1;33m黄\033[0m
\033[1;34m蓝\033[0m
\033[1;35m粉红/洋红\033[0m
\033[1;36m青/蓝绿\033[0m
\033[1;37m白\033[0m
```

## tput改变echo的颜色

示例
```bash
#tput Color Capabilities:
#tput setab [1-7] :  Set a background color using ANSI escape
#tput setb [1-7] :  Set a background color
#tput setaf [1-7] :  Set a foreground color using ANSI escape
#tput setf [1-7] :  Set a foreground color
#tput Text Mode Capabilities:
#tput bold :  Set bold mode
#tput dim :  turn on half-bright mode
#tput smul :  begin underline mode
#tput rmul :  exit underline mode
#tput rev :  Turn on reverse mode
#tput smso :  Enter standout mode (bold on rxvt)
#tput rmso :  Exit standout mode
#tput sgr0 :  Turn off all attributes
#Color Code for tput:
#0 :  Black
#1 :  Red
#2 :  Green
#3 :  Yellow
#4 :  Blue
#5 :  Magenta
#6 :  Cyan
#7 :  White

NORMAL=$(tput sgr0)
GREEN=$(tput setaf 2; tput bold)
YELLOW=$(tput setaf 3)
RED=$(tput setaf 1)
function red() {
echo -e "$RED$*$NORMAL"
}
function green() {
echo -e "$GREEN$*$NORMAL"
}
function yellow() {
echo -e "$YELLOW$*$NORMAL"
}
# To print success
green "Task has been completed"
# To print error
red "The configuration file does not exist"
# To print warning
yellow "You have to use higher version."
```

**说明**
   * man tput   
   * man 5 terminfo   
