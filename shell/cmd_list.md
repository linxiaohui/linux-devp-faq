#常用命令列表

## less
   * Shift+f

## head/tail
   * `head -n -10 tmp` tmp中除了后10行的部分
   * `tail -n +10 tmp` tmp中除了前9行的部分

## xargs & parallel
   * 从标准输入中读取信息作为参数，初始化可执行命令，然后一次或多次运行该指令。可并行执行。

## yes
   * 将参数的字符串重复输出
   * 如果没有参数则打印y
   * 可以用于自动输入交互式命令的选择

## cal
   * 日历
   * `cal 9 1752`

## env
   * 显示当前用户的环境变量；

## look
   * 查找出以参数字符串开头的英语单词

## cut
   * 用来分割文件中的字符串，并且根据要求进行显示

## paste
   * 将几个文件的相应行用制表符连接起来
   * 输出到标准输出

## join
   * 根据公共字段（关键字）来合并两个文件的数据行

## fmt
   * 从指定的文件里读取内容，将其依照指定格式重新编排后
   * 输出到标准输出设备

## pr
   * 将一连串文本编排成合适打印的格式
   * 可以将较大的文件分割成多个页面,并为每个页面添加标题

## fold
   * 从指定的文件里读取内容，将超过限定列宽的列加入`\n`字符后，输出到标准输出设备;

## column
   * 将文本根据空白符格式化成表格形式

## expand
   * 将文件中的制表符（TAB）转换为空白字符（Space）
   * 显示到标准输出设备

## unexpand
   * 和expand相反
   * 将文件中空白字符（Space）的转换为制表符（TAB）
   * 输出到标准输出设备

## nl
   * 在每行前面加上行号

## seq
   * 显示从1到指定数字的数字序列，如果参数为负数
   ```bash
   for i in `seq 10`
   do
   echo $i
   done
   ```

## bc
   * 计算器

## factor
   * 将一个数分解成多个数的乘积；

## nc
   * netcat

## tac
   * 反向输出文件内容

## comm
   * 按行比较两个已排过序的文件

## hexdump
   * `hexdump -C _filename_` 显示16进制

## xxd
   * 将一个文件以十六进制的形式显示出来
   * `xxd -g 1 _filename_`

## tr
   * 实现字符的简单转换
   * 可以删除字符串中的指定子串
   * 合并字符串中重复串

## split & csplit
   * 可将文件切成较小的文件

## host & dig
   * DNS解析查询工具

## last & lastb
   * 查系统的登陆日志

## users
   * 当前登录的用户名

## w
   * 显示目前登入系统的用户信息，比who功能更加强大

## id
   * 查询用户的用户ID和组ID信息

## ss
   * 用来查看socket信息的命令，netstat的升级版

## dmesg
   * 用于显示内核缓冲区系统控制信息的工具

## hdparm
   * 用于检测、显示与设定IDE或SCSI硬盘的参数；

## sdiff
   * 以并排方式合并文件之间的差异

## base64
   * 将数据以Base64编码方式输出

## nice
   * 以指定的优先级运行命令
   * 如果不指定优先级，程序会显示当前的优先级

## setfacl
   * 设定文件的访问控制列表

## tee
   * 将标准输入重定向到文件或者标准输出中

## beep & `echo -e '\a' >/dev/console`
   * 计算机发出嘟嘟的声音，用于向计算机用户告警
   * echo 加 -e 参数，参数才按"Escap String"解释

## mesg
   * 控制是否允许其他发送信息到自己的终端机界面

## write
   * 向该计算机的其他用户发送信息
   * Ctrl+D结束

## wall
   * 向该计算机的所有用户发送一条信息

## finger
   * 查询用户的信息
   * 通常会显示系统中某个用户的用户名、主目录、停滞时间、登录时间、登录shell等信息

## at
   * 定时调度，一定的时间间隔运行作业
   * 需要只运行作业一次而不是定期运行时，使用 at 命令

## taskset
   * 限制进程所占用的CPU数量

## apropos
   * 搜索其它命令的描述信息。
   * 对比`man -k`

## watch
   * 重复执行命令，并输出到屏幕

## dmidecode
   * 以一种可读的方式dump出机器的DMI(Desktop Management Interface)信息,包括了硬件以及BIOS，既可以得到当前的配置，也可以得到系统支持的最大配置；
   * 查看内存的插槽数,已经使用多少插槽.每条内存多大，已使用内存多大
      `sudo dmidecode|grep -P -A5 "Memory\s+Device"|grep Size|grep -v Range`
   * 查看内存支持的最大内存容量
      `sudo dmidecode|grep -P 'Maximum\s+Capacity'`
   * 查看内存的频率
      `sudo dmidecode|grep -A 16 "Memory Device"|grep 'Speed'`

## mapfile && readarray
   * bash内建命令
   * Read lines from the standard input into the indexed array variable array
   * 需要注意的是如果使用 `cat /etc/passwd|mapfile passwd`，passwd变量不存在。因为内建命令放在管道中，bash会在subshell中进行处理。
   * 使用 -u 参数

## eval
Bash內建命令，eval: eval [参数 ...]
    将参数作为 shell 命令执行。

    将 ARGs 合成一个字符串，用结果作为 shell 的输入，
    并且执行得到的命令。

    退出状态：
    以命令的状态退出，或者在命令为空的情况下返回成功。

## coproc
coproc: coproc [名称] 命令 [重定向]
    创建一个以 NAME 为名的副进程。

    异步执行 COMMANDS 命令，在执行 shell 中的数组变量 NAME
    的 0 号和 1 号元素作为文件描述符，以一个管道连接命令
    分别作为命令的标准输出和输入设备。
    默认的 NAME 是 "COPROC"。

    退出状态：
    返回 COMMAND 命令的退出状态。


#其它工具

## fortune
   * 每次运行这个指令，都会给出一句有趣的话，或者一对有意思的问答。

## sl
   * 蒸汽机车

## cowsay/cowthink
   * 这个命令以ASCII字符显示牛说话/思考
   * `fortune | cowsay`

## bvi
   * 二进制文件编辑器；

## mtr
   * 网络工具

## dstat
   * 收集系统的运行数据，还可以收集指定的性能资源

## lshw
   * list hardware，可以查看硬件信息的工具

## nload
   * 实时查看linux服务器网络流量的工具

## iptraf
   * 监控网络流量

## nethogs
   * 按进程实时显示网络流量

## collectl
   * 可以被用来收集描述当前系统状态的数据，并且它支持如下两种模式：
      * 记录模式
         * 允许从一个正在运行的系统中读取数据，然后将这些数据要么显示在终端中，要么写入一个或多个文件或一个套接字中。
      * 回放模式
         * 根据 man 手册，在这种模式下，数据从一个或多个由记录模式生成的数据文件中读取。

## ntop
   * 用于展示网络使用情况
   * 在一定程度上它与 top 针对进程所做的工作类似
   * 基于libpcap

## nmap
   * 获取指定IP的操作系统信息
   * `nmap -sS -O _ip_address_`

## flock
   * flock是"劝告性"锁, 文件可以被其它进程操作
