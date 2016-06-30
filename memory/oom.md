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

Linux 提供了这样一个参数min_free_kbytes，用来确定系统开始回收内存的阈值，控制系统的空闲内存。
值越高，内核越早开始回收内存，空闲内存越高。   
`echo 963840 > /proc/sys/vm/min_free_kbytes`

可以通过修改内核参数禁止OOM机制   
`sysctl -w vm.panic_on_oom=1`
`vm.panic_on_oom = 1` //1表示关闭，默认为0表示开启OOM
`sysctl -p`
