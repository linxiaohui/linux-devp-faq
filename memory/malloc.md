## ptmalloc

glibc使用`ptmalloc`库进行分配.  它通过两个系统调用向kernel申请内存:
   * brk() sets the end of the process's data segment.
   * mmap() creates a new VMA and passes it to the allocator.

`ptmalloc`根据申请的内存大小决定使用`brk`还是`mmap`: 如果申请的内存大于`M_MMAP_THRESHOLD`, 使用`mmap`, 否则调用`brk`.
默认`M_MMAP_THRESHOLD`为128KB, 可以通过`mallopt`更改.

## deferred page allocation
对比一下两个程序
```c
#include <stdio.h>
#include <stdlib.h>
#define MEGABYTE 1024*1024
int main(int argc, char * argv[])
{
  void * myblock = NULL;
  int count = 0;
  while (1)
  {
    myblock = (void * ) malloc(MEGABYTE);
    if (!myblock) break;
    printf("Currently allocating %d MB\n", ++count);
  }
  exit(0);
}
```
与
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MEGABYTE 1024*1024
int main(int argc, char * argv[])
{
  void * myblock = NULL;
  int count = 0;
  while(1)
  {
    myblock = (void * ) malloc(MEGABYTE);
    if (!myblock) break;
    memset(myblock,1, MEGABYTE);
    printf("Currently allocating %d MB\n",++count);
  }
  exit(0);
}
```
在64位Linux上使用`gcc -m32`编译两个程序并运行, 第一个程序比第二个程序能申请到更多的内存,
并且程序A因malloc申请失败而退出, 程序B被kill.
因为Linux采用`deferred page allocation`, 即`optimistic memory allocation`, 只有在使用时才内存才真正分配.
程序A因地址空间用尽malloc失败.

可以在程序中加`sleep`, 运行时使用`watch -n 1 free`观察free的值的变化的方式来验证.

试验的时候先`swapoff -a`关闭swap.


## free的内存不归还给操作系统
测试程序
```c
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define SIZE 1024
#define PAGE 10240
int main()
{
    char * a[SIZE];
    char * b[SIZE];
    printf("before malloc\n");
    sleep(10);
    for(int i=0; i<SIZE; i++)
    {
        a[i] = malloc(PAGE);
        memset(a[i], 1, PAGE);
        b[i] = malloc(1);
        memset(b[i], 1, 1);
    }
    printf("malloc OK\n");
    sleep(10);
    for(int j=0;j<SIZE;j++)
    {
        free(a[j]);
        free(b[j]);
    }
    printf("free OK\n");
    /*char * ptr=malloc(PAGE);
    free(ptr);
    printf("All free\n");*/
    sleep(30);
}
```

通过top -p $(pidof _progname_) 可以看到malloc阶段占用内存不断变大；但free完成后内存占用没有减少，
应用程序已经通过调用free将内存释放，但glic没有将内存归还给操作系统。

如果增加上例中注释的部分，编译执行后glibc将内存归还给操作系统。

## 多线程中的malloc
OpenSUSE 13.2（Glib 2.19）中发现：多线程的程序中malloc内存时，有如下现象：
   1. 线程创建时，进程占用的虚拟内存增加大约`ulimit -s`，这是线程堆栈的大小。
   2. 创建一个线程并申请1K内存时，进程占用的内存增加64M，第二次申请时内存不增加。

示例代码
```c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

volatile int start = 0;

void * run( void * p)
{
    while(1)
    {
        if(start%3==0)
        {
            printf("Thread malloc\n");
            char * buf = malloc(1024);
        }
        sleep(1);
    }
}

int main()
{
    pthread_t th;
    sleep(10);
    pthread_create(&th, 0, run, 0);
    printf("thread created\n");
    sleep(10);
    while(start<10)
    {
        start++;
        sleep(5);
    }
    printf("Creating Another thread\n");
    pthread_create(&th, 0, run, 0);
    while(start<20)
    {
        start++;
        sleep(5);
    }
    return(0);
}
```

`gcc mem.c -lpthread`

原因为glibc在比较高的版本中参考了tcmalloc等内存分配库的思想；即为每个线程分配自己的arena，
这样就可以尽可能避免在内存分配过程中的锁。

默认情况下
   * 32位机器arena数量 2*core数量
   * 64位机器arena数量 8*core数量，每个大小为64M

相关的行为可以由环境变量控制
   * `MALLOC_ARENA_TEST_`: memory pools达到此值时检查CPU核心数量
   * `MALLOC_ARENA_MAX_`: 最大memory pools数量（不论几核）。实践中有建议设其为4

程序中可以使用mallopt(M_ARENA_MAX, 4)修改
