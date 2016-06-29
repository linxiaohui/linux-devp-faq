# AppArmor
AppArmor(Application Armor)提供了强制访问控制机制，可以指定程序可以读、写或运行哪些文件，是否可以打开网络端口等，从而限制程序的功能。

## 配置文件
AppArmor与程序绑定，而不是用户。假设有一个可执行文件的路径为`/home/linux/apparmordemo`，如果要用Apparmor对其进行访问控制的话，就要在`/etc/apparmor.d`中对应文件名为`home.linux.apparmordemo`的配置文件。因每一个可执行文件都是与一个配置文件对应的的，因此如果修改文件名的话，配置文件将失效。

## 工作模式
Apparmor有两种工作模式：`enforcement`、`complain`/learning
   1. enforcement：配置文件里列出的限制条件都会得到执行，并且对于违反这些限制条件的程序会进行日志记录。
   2. complain ：  配置文件里的限制条件不会得到执行，Apparmor只是对程序的行为进行记录。

## 资源限制
### 文件系统
Apparmor可以对某一个文件，或者某一个目录下的文件进行访问控制，包括以下几种访问模式：

   | 模式        |  说明  |
   | --------- - |:------:|
   | r |    Read mode |
   | w |    Write mode (mutually exclusive to a) |
   | a |    Append mode (mutually exclusive to w) |
   | k |    File locking mode |
   | l |    Link mode |
   |   linkfile->target |  Link pair rule (cannot be combined with other access modes)|

例如：
```
/tmp r, #表示可对/tmp目录下的文件进行读取
```
**注意**:没在配置文件中列出的文件，程序是不能访问的

### 资源限制
Apparmor可以提供类似系统调用setrlimit一样的方式来限制程序可以使用的资源。
```
set rlimit [resource] <= [value],
```
例如:
```
set rlimit as<=1M, #可以使用的虚拟内存最大为1M
```

### 网络
Apparmor可以对程序是否可以访问网络进行限制：
```
network [ [domain] [type] [protocol] ]
```
例如:
```
network, #可以进行所有的网络操作
network inet tcp,  #允许程序使用在IPv4下使用TCP协议
```

### capability
AppArmor可以设置进程可以使用的`capabilities`(`man 7 capablities`).
`man apparmor.d` 可以查到:
```
ACCESS TYPE = ( 'allow' | 'deny' )
QUALIFIERS = [ 'audit' ] [ ACCESS TYPE ]
CAPABILITY RULE = [ QUALIFIERS ] 'capability' [ CAPABILITY LIST ]
CAPABILITY LIST = ( CAPABILITY )+
CAPABILITY = (lowercase capability name without 'CAP_' prefix; see capabilities(7))
```

## 配置文件编写
`aa-genprof`


## 常用命令
   1. `sudo apparmor_status`: 查询当前Apparmor的状态
   2. `sudo apparmor_parser`: loads AppArmor profiles into the kernel

## 手册
   1. `man apparmor`
   2. `man apparmor.d`
