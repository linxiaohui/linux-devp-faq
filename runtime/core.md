# 关于coredump

## 允许的文件大小
`ulimit -c unlimited`

## coredump文件路径
daemon 方式运行的程序，其与 CLI 启动的程序的主要区别是进程的运行环境，其中就包括程序的当前路径 cwd。
一般来说，daemon 文件的 cwd 是/（可以通过/proc/_pid_/cwd来查看）。
而如果用户对这个目录没有写权限，那么就不会生成coredump文件。
   * 查看coredump路径
      * 方法1：`cat /proc/sys/kernel/core_pattern`
      * 方法2：`/sbin/sysctl kernel.core_pattern`
   * 修改core dump文件路径
      * 临时修改：`echo '/var/log/%e.core.%p' > /proc/sys/kernel/core_pattern`   
         但/proc目录本身是动态加载的，每次系统重启都会重新加载。
      * 永久修改：使用sysctl -w name=value命令。  
`/sbin/sysctl -w kernel.core_pattern=/var/log/%e.core.%p`

## systemd与coredump
 [/usr/src/linux/Documentation/sysctl/kernel.txt](https://www.kernel.org/doc/Documentation/sysctl/kernel.txt)中关于 core_pattern的描述
```
If the first character of the pattern is a '|', the kernel will treat
  the rest of the pattern as a command to run.  The core dump will be
  written to the standard input of that program instead of to a file.
```
在systemd 下，是一个以'|'开头的字符串，如：| /usr/lib/systemd/systemd-coredump ...
Coredumps are stored in the systemd journal，需要使用 `systemd-coredumpctl` 来查看

## coredump_filter
[/usr/src/linux/Documentation/filesystems/proc.txt](https://www.kernel.org/doc/Documentation/filesystems/proc.txt)中关于通过coredump_filter指定要转储的内存段。

## coredum文件模式
为了更详尽的记录core dump当时的系统状态，可通过以下参数来丰富core文件的命名：
   * %% 单个%字符
   * %p 所dump进程的进程ID
   * %u 所dump进程的实际用户ID
   * %g 所dump进程的实际组ID
   * %s 导致本次core dump的信号
   * %t core dump的时间 (由1970年1月1日计起的秒数)
   * %h 主机名
   * %e 程序文件名

## core_uses_pid
/proc/sys/kernel/core_pattern中未定义%p时，/proc/sys/kernel/core_uses_pid文件中定义是否在core文件名后追加进程PID。
  * `echo 1> /proc/sys/kernel/core_uses_pid` 使得core文件名后包含 .PID
  * `echo 0> /proc/sys/kernel/core_uses_pid` 使得core文件名后不包含 .PID

## suid_dumpable
若程序调用了`seteuid()`/`setegid()`改变了进程的有效用户或组，则在默认情况下系统不会为这些进程生成Coredump。为了能够让这些进程生成core dump，需要将`/proc/sys/fs/suid_dumpable`文件的内容改为1。
