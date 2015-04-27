#环境变量

## LD_* 环境变量

* LD_PRELOAD：影响程序的运行时的链接，它允许定义在程序运行前优先加载的动态链接库。
这个功能主要就是用来有选择性的载入不同动态链接库中的相同函数。
   * AIX使用环境变量 LDR_PRELOAD   
   * AIX下对函数是否能被替代与是否在一个so中有关。  
* LD_DEBUG ：查看一个程序运行时与环境相关的信息（例如加载动态库）。
`export LD_DEBUG=help`后运行程序可以查看该环境变量的各个选项。

* LD_LIBRARY_PATH： 运行时搜索目录。在`-rpath` `/etc/ld.so.conf` 以及系统库之前 （参见dlopen的描述）。
   * 注意区别编译时搜索目录 LIBRARY_PATH

* LD_TRACE_LOADED_OBJECTS : 该环境变量不为空时运行可执行程序只是显示其依赖的so，而程序并不真正执行（参见ldd脚本）

更多环境变量，参见`man ld.so`或`man ld-linux.so`

## system函数与LD_PRELOAD变量
log里会出现 
```
ERROR: ld.so: object LD_PRELOAD cannot be preloaded: ignored,
```
由于代码里调用了`system(const char * )`函数,该函数会启动一个新的进程,并继承父进程所有的环境变量.
若是32位的程序,LD_PRELOAD库也是32位的,但是运行环境是64位的,所以system函数调用的程序也很可能是64位的,
而这些程序启动时LD_PRELOAD环境变量已经被设置去载入,结果发现不兼容,于是报错`cannot be preloaded: ignored`。

#资源限制

## listen socket总是可读而accept失败
产生这样的问题一个可能情况是连接数不够导致的，例如服务器接收的TCP连接超过`ulimit -n`允许的打开文件数，
这时候监听套接字每次select/epoll的时候都返回句柄可读，而accept调用会出现错误`EMFILE`. 
```
accept failed. errno:24, errmsg:Too many open files.
```
可见这种情况下服务器的实现方式有影响。
例如memcahced中如果accept的时候返回EMFILE，那么它会立即调用listen(sfd, 0) ，
也就是将监听套接字的等待accept队列的backlog设置为0，从而拒绝掉这部分请求，减轻系统负载。
并且为了让系统能够自动的恢复重新接受新连接，设置了一个maxconns_handler的定时器用来恢复功能。

## 批量程序一段时间后下载文件失败
一个批量程序，其逻辑是循环调用第三方函数从远程下载文件然后处理，其要处理大量的文件。
随着文件数量的增长，出现了正常执行一段时间后第三方函数报错无法下载文件，从错误码上看是连接服务器失败。

经排查，程序实现有bug，下载文件后fopen文件进行处理后没有fclose；结果导致进程打开的文件数量大于`ulimit -n`的设置。
从而socket失败。


