#系统性能分析工具

![Linux Performance Observability Tools](./linuxperftools.png "Linux Performance Observability Tools")

from [Linux Performance Observability Tools by Brendan Gregg](http://www.brendangregg.com/linuxperf.html)

## uptime
显示系统已启动时间以及系统负载。
   * 系统负载可理解为当前可运行的程序+正在运行的程序+等待 IO 的程序
   * 三个数分别是 1min， 5min 和 15min 的负载平均值
   * 一般情况下这个值越大就说明等待 CPU 的进程越多，如果大于核数就说明有进程在等待 CPU

## 如何查看一个进程的运行时间
使用 ps 命令来查看关于一组正在运行的进程的信息
   * `etime`  显示自从该进程启动以来，经历过的时间，格式为 [[DD-]hh:]mm:ss
   * `etimes` 显示自该进程启动以来，经历过的时间，以秒的形式
   * `ps -p {PID} -o etime`/`ps -p {PID} -o etimes`
   * `ps -p {PID} -o pid,cmd,etime,uid,gid`

## top & htop
   * wa(io wait)
   * st(time stolen from this vm by the hypervisor)虚拟机在等待真实物理机的 CPU 资源的时间
   * htop 是 top 的改进版，带着各种颜色表示和百分比进度条，

## mpstat
   * 显示出每个 CPU 核心的工作情况
   * 可以在 top 里输入 1 看到
   * 可以观察是不是存在负载不均的现象

## iostat
   * 加上 -x 参数后可以看到几乎全部的 io 指标
      * tps
      * 请求 queue 的平均长度
      * 平均处理时间
      * 磁盘带宽利用率等等
   * man iostat

## vmstat & free
   * vmstat 展示内存整体使用情况的命令
      * swpd 和 swap 的 in/out
      * 如果这一部分的数值过大，会频繁的 IO 造成性能下降
      * memory 里的 buffer 指的是写磁盘缓冲区
      * cache 可以当成是读文件的缓冲区。
   * free展示内存部分的内容

## ping
   * 反映了主机间的延迟和连通性
   * hping 有着指定端口，高级 tracerout 的功能

## nicstat
   * 针对网卡的与iostat 类似的命令

## dstat
   * 综合了cpu、 memory、 IO、 network 的工具
   * 可以实时展示当前的系统资源利用情况

## sar
   * System Activity Reporter
   * cpu、 mem、 disk、 net等基本都可以覆盖
   * 可以周期性的执行统计
   * 示例 `sar -A 1 1`
   * 执行`sar`时报错`please check if data collecting is enabled`, 执行
```
systemctl enable sysstat
systemctl start sysstat
```

## netstat
   * 查看 socket 的连接信息，状态等

## pidstat
   * 与 ps 类似
   * pidstat 可以指定进程提供定时多次的统计信息

## strace
   * 可以统计出进程进行了哪些系统调用，处理了哪些信号
   * 是一个分析程序实现的一个很好的工具，

## ltrace
   * strace跟踪系统调用，ltrace跟踪函数调用

## tcpdump
   * 网络探嗅工具，将经过网卡的数据包保存下来
   * 开启网卡混杂模式也可以捕获同一网络上其他的数据包

## blktrace
   * strace 的 io 版
   * 可以实时的展示每次磁盘 IO 请求的内容，耗时，发生位置等等很多东西

## iotop
   * top 的 io 版

## iftop
   * top的实时流量监控版

## slabtop
   * 显示内核slab缓存信息
   * slab 是对象缓冲池，将一些常用的小的对象结构再释放后缓存起来，而不是直接交给系统回收，可以避免频繁的小对象找系统申请内存造成性能下降，

## sysctl
   * 内核参数的显示和设置
   * sysctl -a

## /proc
   * 诸多的工具都是从/proc中相应的文件中读取信息
   * 可以strace之前的命令查看其从何处读取信息
