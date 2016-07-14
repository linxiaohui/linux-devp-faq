# Linux信号处理相关

## 信号种类
   * 可靠信号与不可靠信号
      * 从UNIX系统继承过来的信号都是非可靠信号
         * 信号值小于`SIGRTMIN`的都是非可靠信号.
         * 信号不支持排队
         * 信号可能会丢失, 比如发送多次相同的信号, 进程只能收到一次
      * Linux改进了信号机制, 增加了32种新的信号, 这些信号都是可靠信号
         * 信号值位于 `[SIGRTMIN, SIGRTMAX]` 区间的都是可靠信号.
         * 信号支持排队
         * 不会丢失, 发多少次, 就可以收到多少次.
         * SIGRTMIN在不同的系统可能有不同的值, 必须用 `SIGRTMIN+n`的方式使用
   * 实时信号与非实时信号
      * 可靠信号就是实时信号
      * 非可靠信号就是非实时信号

使用命令 `kill -l` 查看所有信号
程序中可以使用
```c
char *strsignal(int sig);
```
获取信号的名字（或sys_siglist数组. 详情 man strsignal）

## signal调用
早期的Linux使用系统调用 signal 来安装信号

```c
#include <signal.h>
void (* signal(int signum, void (* handler))(int)))(int);
```
该函数有两个参数, signum指定要安装的信号, handler指定信号的处理函数. 该函数的返回值是一个函数指针, 指向上次安装的handler

经典安装方式:
```c
if (signal(SIGINT, SIG_IGN) != SIG_IGN) {
    signal(SIGINT, sig_handler);
}
```
先获得上次的handler, 如果不是忽略信号, 就安装此信号的handler

由于信号被交付后, 系统自动的重置handler为默认动作, 为了使信号在handler
处理期间, 仍能对后继信号做出反应, 往往在handler的第一条语句再次调用 signal
```c
sig_handler(ing signum)
{
    /*重新安装信号*/
    signal(signum, sig_handler);
    ......
}
```
我们知道在程序的任意执行点上, 信号随时可能发生, 如果信号在sig_handler重新安装
信号之前产生, 这次信号就会执行默认动作, 而不是sig_handler. 这种问题是不可预料的.


## sigaction
POSIX 信号安装方式, 使用
sigaction安装信号的动作后, 该动作就一直保持, 直到另一次调用 sigaction建立另一个
动作为止. 这就克服了古老的 signal 调用存在的问题

```c
#include <signal.h>
int sigaction(int signum,const struct sigaction * act,struct sigaction * oldact));
/*使用*/
struct sigaction action, old_action;
/* 设置SIGINT */
action.sa_handler = sig_handler;
sigemptyset(&action.sa_mask);
sigaddset(&action.sa_mask, SIGTERM);
action.sa_flags = 0;

/* 获取上次的handler, 如果不是忽略动作, 则安装信号 */
sigaction(SIGINT, NULL, &old_action);
if (old_action.sa_handler != SIG_IGN) {
    sigaction(SIGINT, &action, NULL);
}
```
目前linux中的signal()是通过sigation()函数
实现的，因此，即使通过signal（）安装的信号，在信号处理函数的结尾也不必
再调用一次信号安装函数。

```c
struct sigaction {
    void     (* sa_handler)(int);
    void     (* sa_sigaction)(int, siginfo_t * , void * );
    sigset_t   sa_mask;
    int        sa_flags;
    void     (* sa_restorer)(void);
};
```
其中，`sa_mask`字段定义了一个信号集，其中的信号在信号处理函数执行期间被屏蔽，在信号处理函数被调用时，内核同时会屏蔽该信号，因此保证了在处理一个给定信号时，如果该信号再次发生，那么它会被阻塞到对前一个信号的处理结束为止（可靠信号，对不可靠信号而言信号可能丢失）。

sa_flags字段指定对信号处理的一些选项，常用的选项及其含义说明`man sigaction`

`man sigqueue`

## 屏蔽信号
每一个进程都有一个信号屏蔽字(参见`/proc/self/status`): 当前要阻塞递送到该进程的信号集。
对于每种可能的信号，该屏蔽字中都有一位与之对应。对于某种信号，若其对应为已设置，则它当前是被阻塞的。
进程可以调用`sigprocmask`来检测和更改当前信号的屏蔽字。

所谓屏蔽, 并不是禁止递送信号, 而是暂时阻塞信号的递送, 解除屏蔽后, 信号将被递送, 不会丢失。

## 信号集
信号种类数目超过一个整型所包含的位数，因此POSIX.1定义了数据结构`sigset_t`表示信号集，并且定义了以下信号集处理函数：
```c
#include <signal.h>
int sigemptyset(sigset_t * set);
int sigfillset(sigset_t * set);
int sigaddset(sigset_t * set, int signum);
int sigdelset(sigset_t * set, int signum);
int sigismember(const sigset_t * set, int signum);
int sigsuspend(const sigset_t * mask);
int sigpending(sigset_t * set);
int sigprocmask(int  how,  const  sigset_t * set, sigset_t * oldset))；
```

```
#include <signal.h>  
#include <stdio.h>  
#include <stdlib.h>  
#include <error.h>  
#include <string.h>  

void sig_handler(int signum)  
{  
    printf("catch SIGINT\n");  
}  

int main(int argc, char ** argv)  
{  
    sigset_t block;  
    struct sigaction action, old_action;  

    /*安装信号*/  
    action.sa_handler = sig_handler;  
    sigemptyset(&action.sa_mask);  
    action.sa_flags = 0;  

    sigaction(SIGINT, NULL, &old_action);  
    if (old_action.sa_handler != SIG_IGN) {  
        sigaction(SIGINT, &action, NULL);  
    }  

    /*屏蔽信号*/  
    sigemptyset(&block);  
    sigaddset(&block, SIGINT);  

    printf("block SIGINT\n");  
    sigprocmask(SIG_BLOCK, &block, NULL);  

    printf("--> send SIGINT -->\n");  
    kill(getpid(), SIGINT);  
    printf("--> send SIGINT -->\n");  
    kill(getpid(), SIGINT);  
    sleep(1);  

    /*解除信号后, 之前触发的信号将被递送,
     但SIGINT是非可靠信号, 只会递送一次*/  
    printf("unblock SIGINT\n");  
    sigprocmask(SIG_UNBLOCK, &block, NULL);  

    sleep(2);  

    return 0;  
}  
```
这里发送了两次SIGINT信号 可以看到, 屏蔽掉SIGINT后,
信号无法递送, 解除屏蔽后, 才递送信号, 但只被递送一次,
因为SIGINT是非可靠信号, 不支持排队.

```c
#include <signal.h>  
#include <stdio.h>  
#include <stdlib.h>  
#include <error.h>  
#include <string.h>  

void sig_handler(int signum)  
{  
    printf("in handle, SIGTERM is blocked\n");  
    /*在此handler内将屏蔽掉SIGTERM, 直到此handler返回*/  
    printf("--> send SIGTERM -->\n");  
    kill(getpid(), SIGTERM);  
    sleep(5);  
    printf("handle done\n");  
}  

void handle_term(int signum)  
{  
    printf("catch sigterm and exit..\n");  
    exit(0);  
}  

int main(int argc, char ** argv)  
{  
    struct sigaction action, old_action;  

    /*设置SIGINT*/  
    action.sa_handler = sig_handler;  
    sigemptyset(&action.sa_mask);  
    /*安装handler的时候, 设置在handler
      执行期间, 屏蔽掉SIGTERM信号*/
    sigaddset(&action.sa_mask, SIGTERM);  
    action.sa_flags = 0;  

    sigaction(SIGINT, NULL, &old_action);  
    if (old_action.sa_handler != SIG_IGN) {  
        sigaction(SIGINT, &action, NULL);  
    }  

    /*设置SIGTERM*/  
    action.sa_handler = handle_term;  
    sigemptyset(&action.sa_mask);  
    action.sa_flags = 0;  

    sigaction(SIGTERM, NULL, &old_action);  
    if (old_action.sa_handler != SIG_IGN) {  
        sigaction(SIGTERM, &action, NULL);  
    }  

    printf("--> send SIGINT -->\n");  
    kill(getpid(), SIGINT);  

    while (1) {  
        sleep(1);  
    }  

    return 0;  
}  
```
收到SIGINT后, 进入sig_handler,此时发送SIGTERM信号将被屏蔽,
等sig_handler返回后, 才收到SIGTERM信号, 然后退出程序

## 未决信号
未决信号, 是指被阻塞的信号, 等待被递送的信号.
int sigsuspend(const sigset_t * mask)；
int sigpending(sigset_t * set)
获得当前已递送到进程， 却被阻塞的所有信号，在set指向的信号集中返回结果。
```c
#include <signal.h>  
#include <stdio.h>  
#include <stdlib.h>  
#include <error.h>  
#include <string.h>  

/* 版本1, 可靠信号将被递送多次 */  
//#define MYSIGNAL SIGRTMIN+5  
/* 版本2, 不可靠信号只被递送一次 */  
#define MYSIGNAL SIGTERM  

void sig_handler(int signum)  
{  
    psignal(signum, "catch a signal");  
}  

int main(int argc, char ** argv)  
{  
    sigset_t block, pending;  
    int sig, flag;  

    /*设置信号的handler*/  
    signal(MYSIGNAL, sig_handler);  

    /*屏蔽此信号*/  
    sigemptyset(&block);  
    sigaddset(&block, MYSIGNAL);  
    printf("block signal\n");  
    sigprocmask(SIG_BLOCK, &block, NULL);  

    /*发两次信号, 看信号将会被触发多少次*/  
    printf("---> send a signal --->\n");  
    kill(getpid(), MYSIGNAL);  
    printf("---> send a signal --->\n");  
    kill(getpid(), MYSIGNAL);  

    /*检查当前的未决信号*/  
    flag = 0;  
    sigpending(&pending);  
    for (sig = 1; sig < NSIG; sig++) {  
        if (sigismember(&pending, sig)) {  
            flag = 1;  
            psignal(sig, "this signal is pending");  
        }   
    }  
    if (flag == 0) {  
        printf("no pending signal\n");  
    }  

    /*解除此信号的屏蔽, 未决信号将被递送*/  
    printf("unblock signal\n");  
    sigprocmask(SIG_UNBLOCK, &block, NULL);  

    /*再次检查未决信号*/  
    flag = 0;  
    sigpending(&pending);  
    for (sig = 1; sig < NSIG; sig++) {  
        if (sigismember(&pending, sig)) {  
            flag = 1;  
            psignal(sig, "a pending signal");  
        }   
    }  
    if (flag == 0) {  
        printf("no pending signal\n");  
    }  
    return 0;  
}  
```
发送两次可靠信号, 最终收到两次信号
发送两次非可靠信号, 最终只收到一次

## 被中断了的系统调用
一些IO系统调用执行时, 如 read 等待输入期间, 如果收到一个信号,
系统将中断read, 转而执行信号处理函数. 当信号处理返回后, 系统
遇到了一个问题: 是重新开始这个系统调用, 还是让系统调用失败?

早期UNIX系统的做法是, 中断系统调用, 并让系统调用失败, 比如read
返回 -1, 同时设置 errno 为 EINTR

中断了的系统调用是没有完成的调用, 它的失败是临时性的, 如果再次调用
则可能成功, 这并不是真正的失败, 所以要对这种情况进行处理, 典型的方式为:
```c
while (1) {
    n = read(fd, buf, BUFSIZ);
    if (n == -1 && errno != EINTR) {
        printf("read error\n");
        break;
    }
    if (n == 0) {
        printf("read done\n");
        break;
    }
}
```
安装信号的时候, 设置 SA_RESTART
属性, 那么当信号处理函数返回后, 被该信号中断的系统
调用将自动恢复.
```c
struct sigaction action, old_action;  

action.sa_handler = sig_handler;  
sigemptyset(&action.sa_mask);  
action.sa_flags = 0;  
/* 版本1:不设置SA_RESTART属性
 * 版本2:设置SA_RESTART属性 */  
action.sa_flags |= SA_RESTART;  

sigaction(SIGINT, NULL, &old_action);  
if (old_action.sa_handler != SIG_IGN) {  
    sigaction(SIGINT, &action, NULL);  
}  
```




## 非局部控制转移
setjmp 和 sigsetjmp 的区别是: setjmp 不一定会恢复信号集合,而sigsetjmp可以保证恢复信号集合
```c
#include <signal.h>  
#include <stdio.h>  
#include <stdlib.h>  
#include <errno.h>  
#include <string.h>  
#include <setjmp.h>  

void sig_alrm(int signum);  
void sig_usr1(int signum);  
void print_mask(const char * str);  

static sigjmp_buf jmpbuf;  
static volatile sig_atomic_t canjmp;  
static int sigalrm_appear;  

int main(int argc, char ** argv)  
{  
    struct sigaction action, old_action;  

    /*设置SIGUSR1*/  
    action.sa_handler = sig_usr1;  
    sigemptyset(&action.sa_mask);  
    action.sa_flags = 0;  

    sigaction(SIGUSR1, NULL, &old_action);  
    if (old_action.sa_handler != SIG_IGN) {  
        sigaction(SIGUSR1, &action, NULL);  
    }  

    /*设置SIGALRM*/  
    action.sa_handler = sig_alrm;  
    sigemptyset(&action.sa_mask);  
    action.sa_flags = 0;  

    sigaction(SIGALRM, NULL, &old_action);  
    if (old_action.sa_handler != SIG_IGN) {  
        sigaction(SIGALRM, &action, NULL);  
    }  

    print_mask("starting main:");  

    if (sigsetjmp(jmpbuf, 1) != 0) {  
        print_mask("exiting main:");  
    } else {  
        printf("sigsetjmp return directly\n");  
        canjmp = 1;  
        while (1) {  
            sleep(1);  
        }  
    }  
    return 0;  
}  

void sig_usr1(int signum)  
{  
    time_t starttime;  
    if (canjmp == 0) {  
        printf("please set jmp first\n");  
        return;  
    }  

    print_mask("in sig_usr1:");  

    alarm(1);  
    while (!sigalrm_appear);  
    canjmp = 0;  
    siglongjmp(jmpbuf, 1);  
}  

void sig_alrm(int signum)  
{  
    print_mask("in sig_alrm:");  
    sigalrm_appear = 1;  

    return;  
}  

void print_mask(const char * str)   
{  
    sigset_t sigset;  
    int i, errno_save, flag = 0;  

    errno_save = errno;  

    if (sigprocmask(0, NULL, &sigset) < 0) {  
        printf("sigprocmask error\n");  
        exit(0);  
    }  

    printf("%s\n", str);  
    fflush(stdout);  

    for (i = 1; i < NSIG; i++) {  
        if (sigismember(&sigset, i)) {  
            flag = 1;  
            psignal(i, "a blocked signal");  
        }  
    }  

    if (!flag) {  
        printf("no blocked signal\n");  
    }  

    printf("\n");  
    errno = errno_save;  
}  
```

## RST与SIGPIPE
   1. socket通讯中，若对方close了socket，继续向其中写数据，第一次发送如果发送缓冲没问题, 会返回正确发送，但会收到RST包;
   2. 向一个收到了RST包的socket继续写入数据会收到SIGPIPE信号
   3. socket选项SO_LINGER决定了close socket后是否立即向对方发送RST包

## SIGBUS
  * 根据man mmap：若mmap后访问的内存没有文件对应（例如被另外的进程truncate）会产生SIGBUS信号
  * 据说一些平台下访问未对其的内存会发生SIGBUS； x86平台不会

## system函数与SIGCHLD信号
   * system函数的实现中使用了waitpid，忽略SIGCHLD信号会导致waitpid错误.
   * system函数执行过程中 SIGCHLD 被 blocked, SIGINT 和 SIGQUIT被忽略（参考glibc中system的实现）。
   * 关于system函数的返回值：`man wait`
   * system无法获取调用命令的屏幕输出，代替方案 `popen`和`pclose`可以

## 关于daemon进程的创建
创建daemon进程的一般步骤是
```c
pid=fork();
if(pid!=0) exit();
setsid();
pid=fork();
if(pid!=0) exit();
chdir("/");
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
