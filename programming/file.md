# 文件锁
根据底层的实现, Linux的文件锁主要有两种: `flock`和`lockf`(需要对lockf说明的是它只是`fcntl`系统调用的一个封装).
这两种文件锁是从历史上不同的标准中起源的, `flock`来自BSD而`lockf`来自POSIX, 因此`lockf`(或`fcntl`)实现的锁在类型上又叫做POSIX锁.

从使用角度讲, `lockf`(或`fcntl`)实现了更细粒度文件锁：记录锁。
   * 可以使用`lockf`(或`fcntl`)对文件的部分字节上锁
   * `flock`只能对整个文件加锁
   * `flock`的语义是针对文件的锁
   * `lockf`是针对文件描述符的锁
   * `fcntl`系统调用可以支持强制锁(Mandatory locking)

flock创建的锁是和文件打开表项(struct file)相关联的, 而不是fd.  因此复制文件fd(通过fork或者dup)后, 通过这两个fd都可以操作这把锁, 子进程继承父进程的锁.

示例程序
```c
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <wait.h>

#define PATH "/tmp/test-lock"

int main()
{
    int fd = 0;
    pid_t pid =0;
    fd = open(PATH, O_RDWR|O_CREAT|O_TRUNC, 0644);
    if (fd < 0) {
        perror("open()");
        exit(1);
    }

    if (flock(fd, LOCK_EX) < 0) {
        perror("flock()");
        exit(1);
    }
    printf("pid=<%d>: locked!\n", getpid());

    pid = fork();
    if (pid < 0) {
        perror("fork()");
        exit(1);
    }

    if (pid == 0) {
        /*若子进程中重新打开文件, 子进程会阻塞*/
        if (flock(fd, LOCK_EX) < 0) {
            perror("flock()");
            exit(1);
        }
        printf("pid=<%d>: locked!\n", getpid());
        exit(0);
    }
    wait(NULL);
    unlink(PATH);
    exit(0);
}
```
该程序编译运行后父子进程都会显示 `:locked`.

**备注**
上面代码中, 若使用`lockf(fd, F_LOCK, 0)`的方式加锁, 子进程将阻塞.

# 标准IO库文件锁
```c
#include <stdio.h>

void flockfile(FILE * filehandle);
int ftrylockfile(FILE * filehandle);
void funlockfile(FILE * filehandle);
```
**注意事项**

标准库实现的锁是在用户态的`FILE结构体`中实现的, 而非内核中的数据结构来实现.
因此只能处理 **一个进程中的多个线程之间共享的FILE的文件** 操作: 多个线程必须同时操作一个用`fopen`打开的`FILE`;
如果内部自己使用`fopen`重新打开文件, 返回的`FILE *`地址不同, 也起不到线程的互斥作用.

# 文件锁相关命令
   * `flock`
   * `lslocks`
   * `cat /proc/locks`
