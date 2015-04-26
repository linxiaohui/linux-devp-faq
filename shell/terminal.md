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
```shell
GREEN=$(tput setaf 2; tput bold)
NORMAL=$(tput sgr0)
function green() { 
echo -e "$GREEN$*$NORMAL" 
}
green "Task has been completed"
```
   * man tput   
   * man 5 terminfo   
