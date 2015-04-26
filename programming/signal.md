# Linux信号种类

## 信号种类
   * 可靠信号与不可靠信号
      * 那些从UNIX系统继承过来的信号都是非可靠信号, 表现在信号不支持排队,信号可能会丢失, 比如发送多次相同的信号, 进程只能收到一次
      * 信号值小于SIGRTMIN的都是非可靠信号.
      * Linux改进了信号机制, 增加了32种新的信号, 这些信号都是可靠信号, 表现在信号支持排队, 不会丢失, 发多少次, 就可以收到多少次.
      * 信号值位于 [SIGRTMIN, SIGRTMAX] 区间的都是可靠信号.
   * 实时信号与非实时信号
   * 可靠信号就是实时信号, 非可靠信号就是非实时信号

使用命令 `kill -l` 查看所有信号

## 屏蔽信号
所谓屏蔽, 并不是禁止递送信号, 而是暂时阻塞信号的递送, 解除屏蔽后, 信号将被递送, 不会丢失。

## 非局部控制转移
setjmp 和 sigsetjmp 的区别是: setjmp 不一定会恢复信号集合,而sigsetjmp可以保证恢复信号集合

## RST与SIGPIPE
   1. socket通讯中，若对方close了socket，继续向其中写数据，第一次会收到RST包
   2. 向一个收到了RST包的socket继续写入数据会收到SIGPIPE信号
   3. socket选项SO_LINGER决定了close socket后是否立即向对方发送RST包
   
## SIGBUS与SIGSEGV
  * 根据man mmap：若mmap后访问的内存没有文件对应（例如被另外的进程truncate）会产生SIGBUS信号
  * 据说一些平台下访问未对其的内存会发生SIGBUS； x86平台不会

## system函数与SIGCHLD信号
   * system函数的实现中使用了waitpid，忽略SIGCHLD信号会导致waitpid错误.
   * system函数执行过程中 SIGCHLD 被 blocked, SIGINT 和 SIGQUIT被忽略（参考glibc中system的实现）。
   * 关于system函数的返回值 man wait
   * system的代替方案 popen和pclose

## 关于daemon进程的创建
创建daemon进程的一般步骤是
```c
pid=fork();
if(pid!=0) exit();
setsid();
pid=fork();
if(pid!=0) exit();
chdir(“/”);
close(0);close(1);close(2);
stdfd = open ("/dev/null", O_RDWR);
dup2(stdfd, STDOUT_FILENO);
dup2(stdfd, STDERR_FILENO);
```
* 这样做的原因主要是进程的控制终端由于某些原因（如断开终端链接）会发送一些信号给进程，
而接收进程处理这些信号缺省动作会让进程退出。   
*例如*
   * 按CTRL-C ,CTRL-\ CTRL-Z会向前台进程组发送下面这些信号
       * signal(SIGINT,  SIG_IGN );
       * signal(SIGQUIT, SIG_IGN );
       * signal(SIGTSTP, SIG_IGN );
   * 终端断开，会给会话组长或孤儿进程组所有成员发送下面信号
       * signal(SIGHUP,  SIG_IGN );

* 第一次fork的作用是让shell认为本条命令已经终止，不用挂在终端输入上。
还有一个作用是为确保调用setsid时进程不是进程组组长。
setsid()使进程没有控制终端
* 第二次fork主要是为了防止进程再次打开一个控制终端。（因为只有会话组长才能打开控制终端）。

