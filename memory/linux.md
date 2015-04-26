# Linux提供的内存相关的技术

## 查看进程内存使用情况
   * top命令
`top -d 1 -p pid [,pid ...]  //设置为delay 1s，默认是delay 3s`
如果想根据内存使用量进行排序，可以shift + m（Sort by memory usage）
   * pmap
   * ps
   * /proc/_pid_/status
      * cat /proc/_pid_/status | grep VmSize 任务虚拟地址空间的大小
      * cat /proc/_pid_/status | grep VmRSS  应用程序正在使用的物理内存的大小

## OOM killer
man malloc查看关于OOM的信息：

optimistic memory allocation stategy
* /proc/sys/vm/overcommit_memory
   * 0：启发式策略，比较严重的Overcommit将不能得逞
   * 1：永远允许overcommit
   * 2：永远禁止overcommit，在这个情况下，系统所能分配的内存不会超过swap+RAM*/proc/sys/vm/overcmmit_ratio%
* /proc/_pid_/oom_score
   * oom killer 会杀死oom_score较大的进程，当oom_score为0时禁止内核杀死该进程
* /proc/_pid_/oom_adj
   * oom_adj的值越大，该进程被系统选中终止的可能就越高，当oom_adj=-17时oom_score将变为0
   
Linux 提供了这样一个参数min_free_kbytes，用来确定系统开始回收内存的阈值，控制系统的空闲内存。值越高，内核越早开始回收内存，空闲内存越高。
`echo 963840 > /proc/sys/vm/min_free_kbytes` 

可以通过修改内核参数禁止OOM机制
`sysctl -w vm.panic_on_oom=1`
`vm.panic_on_oom = 1 //1表示关闭，默认为0表示开启OOM`
`sysctl -p`

## 关于free
free -m的结果可以用下图描述：
 
![free -m](./free.png "free -m")


## 清空cache:  
`sudo sysctl vm.drop_caches=3 释放buffer+cache`   
`echo 1 > /proc/sys/vm/drop_caches`   

## 内核空间的内存 
`slabtop -s c | head`

## 共享内存配置
   * /proc/sys/kernel/shmmax 参数定义共享内存段的最大尺寸
   * /proc/sys/kernel/shmmni 设置系统范围内共享内存段的最大数量
   * /proc/sys/kernel/shmall 系统一次可以使用的共享内存总量（以页为单位）
