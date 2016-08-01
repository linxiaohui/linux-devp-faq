#工具

## Clang Static Analyzer
代码静态检查工具，是 clang 编译器的一部分

## google-perftools
google-perftool是由google开发的用来分析C/C++程序性能的一套工具，主要包括内存和CPU 两个方面
   * 内存分析使用google-perftool所提供的tcmalloc
   * CPU分析使用它所提供的profiler

tcmalloc(thread cache malloc) 的主要优点有
   * 内存allocate/deallocate的速度，通常情况下它的速度比glibc所提供的 malloc要快
   * 小内存(< =32K)的管理，它的小内存是在thread cache里面管理的
      * 减少了加锁的开销，
      * 用来表示小内存所用的额外的空间也比较小，比较节省空间。

除了在allocate/deallocate内存时的优化外，tcmalloc还提供了heapcheck和heapprofile的功能。
heapcheck主要被用来检查程序中是否有内存泄露。

关于google-perftool的使用，总体上来讲有以下三种方式：
   * 直接调用提供的api：这种方式比较适用于对于程序的某个局部来做分析的情况，直接在要做分析的局部调用相关的api即可
   * 链接静态库
   * 链接动态库

* Heap Checker:
```shell
gcc [...] -o myprogram -ltcmalloc
env HEAPCHECK=normal ./myprogram
```
* Heap Profiler:
```shell
gcc [...] -o myprogram -ltcmalloc
env HEAPPROFILE=./myprogram.hprof ./myprogram
```
* Cpu Profiler:
```shell
gcc [...] -o myprogram -lprofiler
env CPUPROFILE=./myprogram.prof ./myprogram
```
通过上面的方法，可以生成google-perftool原始的输出结果。
google-perftoolg还提供了一个叫 pprof的工具，它是一个perl的脚本.
通过这个工具，可以将google-perftool的输出结果分析得更为直观，输出为图片、pdf等格式。

**注意**
   * 要用google-perftool来分析程序，必须保证程序能正常退出。
   * 64位操作系统需要安装 libunwind库，libunwind库为基于64位CPU和操作系统的程序提供了基本的堆栈辗转开解功能，其中包括用于输出堆栈跟踪的API、用于以编程方式辗转开解堆栈的API以及支持C++异常处理机制的API。
   * 安装完成后，将/path/to/libtcmalloc.so 加入LD_LIBRARY_PATH，在需要启动程序<cmd>时使用 `LD_PRELOAD=/path/to/libtcmalloc.so  <cmd>`
   * 可以使用lsof查看tcmalloc是否生效

## gprof
GNU 编译器工具包所提供了一种剖析工具 GNU profiler（gprof）。
gprof 可以为 Linux平台上的程序精确分析性能瓶颈。gprof精确地给出函数被调用的时间和次数，给出函数调用关系。
   1. 可以显示"flat profile"，包括每个函数的调用次数，每个函数消耗的处理器时间，
   2. 可以显示"Call graph"，包括函数的调用关系，每个函数调用花费了多少时间。
   3. 可以显示"注释的源代码"－－是程序源代码的一个复本，标记有程序中每行代码的执行次数。

* 原理：  
通过在编译和链接程序的时候（使用 -pg 编译和链接选项），
gcc 在应用程序的每个函数中都加入了一个名为mcount的函数调用，而mcount会在内存中保存一张函数调用图，
并通过函数调用堆栈的形式查找子函数和父函数的地址。这张调用图也保存了所有与函数相关的调用时间，调用次数等等的所有信息。
运行程序后在程序运行目录下生成 gmon.out文件（如果原来有gmon.out 文件，将会被重写）。

用 gprof 工具分析 gmon.out 文件。   
一般用法：
   `gprof –b 二进制程序 gmon.out >report.txt`    
注：`export GMON_OUT_PREFIX=x.out` 生成的文件名形如x.out.<进程的pid>   
Gprof 的具体参数 `man gprof`

* 注意
程序的累积执行时间只是包括gprof能够监控到的函数。
工作在内核态的函数和没有加-pg编译的第三方库函数是无法被gprof能够监控到的

* 共享库的支持
对于代码剖析的支持是由编译器增加的，因此如果希望从共享库中获得剖析信息，就需要使用 -pg 来编译这些库。
如果需要分析系统函数（如libc库），用 –lc_p替换-lc，这样程序会链接libc_p.so或libc_p.a。   
用`ldd ./example | grep libc`来查看程序链接的是libc.so还是libc_p.so

**注意事项** 
   1. 在编译和链接两个过程，都要使用-pg选项。
   2. 只能使用静态连接libc库，否则在初始化*.so之前就调用profile代码会引起段错误，
      * 编译时加上`-static-libgcc`或`-static`。
   3. 如果不用g++而使用ld直接链接程序，要加上链接文件/lib/gcrt0.o
      * `ld -o myprog /lib/gcrt0.o myprog.o utils.o -lc_p` 
      * 也可能是gcrt1.o
   4. 要监控到第三方库函数的执行时间，第三方库也必须是添加 –pg 选项编译的。
   5. gprof只能分析应用程序所消耗掉的用户时间.
   6. 程序不能以demon方式运行。否则采集不到时间。（可采集到调用次数）
   7. 首先使用 time 来运行程序从而判断 gprof 是否能产生有用信息是个好方法。
   8. 如果 gprof 不适合您的剖析需要，那么还有其他一些工具可以克服 gprof 部分缺陷，包括 OProfile 和 Sysprof。
   9. gprof对于代码大部分是用户空间的CPU密集型的程序用处明显。对于大部分时间运行在内核空间或者由于外部因素（例如操作系统的 I/O 子系统过载）而运行得非常慢的程序难以进行优化。
   10. gprof 不支持多线程应用，多线程下只能采集主线程性能数据。原因是gprof采用ITIMER_PROF信号，在多线程内只有主线程才能响应该信号。但是有一个简单的方法可以解决这一问题：http://sam.zoy.org/writings/programming/gprof.html
采用什么方法才能够分析所有线程呢？关键是能够让各个线程都响应ITIMER_PROF信号。可以通过桩子函数来实现，重写pthread_create函数。
   11. gprof只能在程序正常结束退出之后才能生成报告（gmon.out）。
        * 原因： gprof通过在atexit()里注册了一个函数来产生结果信息，任何非正常退出都不会执行atexit()的动作，所以不会产生gmon.out文件。
        * 程序可从main函数中正常退出，或者通过系统调用exit()函数退出。
        * 可以使用捕捉信号，在信号处理函数中调用exit的方式来解决部分问题。
        * 不能捕获、忽略SIGPROF信号。man手册对SIGPROF的解释是：profiling timer expired. 如果忽略这个信号，gprof的输出则是：Each sample counts as 0.01 seconds. no time accumulated.

## Coan
coan可以用来净化C语言中的预编译指令；可以用于阅读源代码时简化无用的代码。    
例如学习libevent在Linux下实现时可以使用如下命令
```bash
coan source -D_EVENT_HAVE_SYS_IOCTL_H -D_EVENT_HAVE_NETDB_H -D_EVENT_HAVE_STDDEF_H \ 
-D_EVENT_HAVE_STDINT_H  -D_EVENT_HAVE_SYS_TIME_H -D_EVENT_HAVE_SYS_TYPES_H \
-D_EVENT_HAVE_INTTYPES_H \
-D_EVENT_HAVE_SYS_SOCKET_H -D_EVENT_HAVE_UNISTD_H -D_EVENT_HAVE_SYS_EVENTFD_H -D_EVENT_HAVE_SELECT \
-D_EVENT_HAVE_POLL -D_EVENT_HAVE_EPOLL -U__cplusplus -UWIN32 -U_EVENT_HAVE_EVENT_PORTS \
-U_EVENT_HAVE_WORKING_KQUEUE -U_EVENT_HAVE_DEVPOLL -U_EVENT_IN_DOXYGEN -U_MSC_VER \
-U_EVENT_HAVE_SYS_SENDFILE_H --filter c,h --recurse .
```

