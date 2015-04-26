#运行时相关环境变量和SHELL环境资源限制

## LD_* 环境变量

* LD_PRELOAD
环境变量LD_PRELOAD可以影响程序的运行时的链接，它允许定义在程序运行前优先加载的动态链接库。
这个功能主要就是用来有选择性的载入不同动态链接库中的相同函数。
   * 备注： AIX使用环境变量 LDR_PRELOAD。另外，AIX下对函数是否能被替代与是否在一个so中有关。
man ld.so
man ld-linux.so
* LD_DEBUG
环境变量LD_DEBUG查看一个程序搜索其各个动态库
export LD_DEBUG=help后运行程序可以查看该环境变量的各个选项。

* LD_LIBRARY_PATH
环境变量 LD_LIBRARY_PATH 运行时搜索目录。在 -rpath /etc/ld.so.conf 系统库之前 （参见dlopen的描述）
区别编译时搜索目录 LIBRARY_PATH

* LD_TRACE_LOADED_OBJECTS

LD_TRACE_LOADED_OBJECTS环境变量不为空时运行可执行程序只是显示其依赖的so，而程序并不真正执行（参见ldd脚本）

## system函数与LD_PRELOAD变量
log里会出现 
```
ERROR: ld.so: object LD_PRELOAD cannot be preloaded: ignored,
```
由于代码里调用了system(const char * )函数,该函数会启动一个新的进程,并继承父进程所有的环境变量.
若是32位的程序,LD_PRELOAD库也是32位的,但是运行环境是64位的,所以system函数调用的程序也很可能是64位的,
而这些程序启动时LD_PRELOAD环境变量已经被设置去载入,结果发现不兼容,于是报错cannot be preloaded: ignored。

## 连接数不够导致listen socket总是可读而accept失败
例如：`ulimit -n 20 `修改当前会话的打开文件数，然后启动某个服务器程序，
然后给其发送超过限制的TCP连接，这时候监听套接字一定会每次select/epoll的时候，都返回句柄可读。
从而不断的accept调用，然后accept立即出现如下错误,也就是EMFILE：
```
accept failed. errno:24, errmsg:Too many open files.
```
可见这种情况下服务器的实现方式有影响。
例如memcahced中如果accept 的时候返回EMFILE，那么它会立即调用listen(sfd, 0) ，
也就是将监听套接字的等待accept队列的backlog设置为0，从而拒绝掉这部分请求，减轻系统负载。
并且为了让系统能够自动的恢复重新接受新连接，设置了一个maxconns_handler的定时器用来恢复功能。



