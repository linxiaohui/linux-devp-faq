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


查找相关资料[1]解释如下:
```
ptmalloc使用chunk结构来实现内存管理。用户free掉的内存并不是都会马上归还给系统，ptmalloc会统一管理heap和mmap映射区域中的空闲chunk。当用户进行下一次分配请求时，ptmalloc会首先试图在空闲的chunk中挑选一块给用户，这样就避免了频繁的系统调用，降低了内存分配的开销。对于空闲的chunk，ptmalloc采用分箱式内存管理方式，根据空闲chunk的大小和处于的状态将其放在三个不同的容器中。

    bins：ptmalloc将相似大小的chunk用双向链表链接起来，这样的一个链表被称为一个bin。bins有128个队列，前64个队列是定长的（small bins），每隔8个字节大小的块分配在一个队列，后面的64个队列是不定长的（large bins），就是在一个范围长度的都分配在一个队列中。所有长度小于512字节的都分配在定长的队列中，后面的64个队列是变长的队列，每个队列中的chunk都是从大到小排列的。

     unsort队列（只有一个队列），它是一个cache，所有free下来的如果要进入bins队列中都要经过unsort队列，分配内存时会查看unsorted bin中是否有合适的chunk，如果找到满足条件的chunk，则直接返回给用户，否则将unsorted bin中所有chunk放入bins中。

     fastbins，大约有10个定长队列，它是一个高速缓冲，所有free下来的并且长度是小于max_fast（默认80B）的chunk就会进入这种队列中。进入此队列的chunk在free的时候并不修改使用位，目的是为了避免被相邻的块合并掉。

malloc的步骤：
     1.  先在fastbins中找，如果能找到，从队列中取下后（不需要再置使用位为1）立刻返回；
     2.  判断需求的块是否在small bins（bins的前64个bin）范围，如果在，并且刚好有满足需求的块，则直接返回内存地址；
     3.  到了这一步，说明需要分配的是一块大内存，或者小箱子里找不到合适的chunk；这个时候，会触发consolidate，ptmalloc首先会遍历fastbins中的chunk，将相邻的chunk合并，并链接到unsorted bin中（因为在大箱子找一般都要切割，所以要优先合并，避免过多碎片）；
     4.  在unsort bin中取出一个chunk，如果能找到刚好和想要的chunk相同大小的chunk，立刻返回，如果不是想要的chunk大小的chunk，就把它插入到bins对应的队列中去，转到2。
     5.  到了这一步，说明需要分配的是一块大的内存，或者small bins和unsorted bin中都找不到合适的chunk，并且fastbins和unsorted bin中所有的chunk都清除干净了。在large bins中找，找到一个最小的能符合需求的chunk从队列中取下，如果剩下的大小还能建一个chunk，就把chunk分成两个部分，把剩下的chunk插入到unsort队列中取，把chunk的内存地址返回；
     6.  如果搜索fastbins和bins都没有找到合适的chunk，那么就需要操作topchunk（是堆顶的一个chunk，不会放在任何一个队列里）来进行分配了。在topchunk找，如果能切出符合要求的，把剩下的一部分当作topchunk，然后返回内存地址；
     7.  到了这一步说明topchunk也不能满足分配要求，就只能调用sysalloc，其实就是增长堆了，然后返回内存地址。

free的步骤：
     1.  判断所需释放的chunk是否为mmaped chunk，如果是，则调用munmap释放mmaped chunk，解除内存空间映射，该空间不再有效，然后立刻返回；
     2.  如果和topchunk相邻，直接和topchunk合并，不会放到其他的空闲队列中取，然后立刻返回；
     3.  如果释放的大小小于max_fast(80字节)，就把它挂到fastbins中去返回，使用位仍然为1，当然更不会去合并相邻块，然后立刻返回；
     4.  如果释放块得大小介于80—128K，把chunk的使用位置为0，判断前一个chunk是否处于使用中，如果前一块也是空闲块，则合并，并转入下一步；
     5.  判断当前释放chunk的下一个块是否为top chunk，如果是，则转到第7步，否则转下一步；
     6.  判断下一个chunk是否处在使用中，如果也是空闲的，则合并，并将合并后的chunk挂到unsort队列中去；
     7.  如果执行到了这一步，说明释放了一个与top chunk相邻的chunk；则无论它有多大，都将它与top chunk合并，并更新top chunk的大小等信息，转下一步；
     8.  如果合并后的大小大于FASTBIN_CONSOLIDATION_THRESHOLD(64K)，也会触发consolidate，即fastbins的合并操作，合并后的chunk会被放到unsorted bin中，fastbins将变为空，操作完成之后转下一步；
     9.   试图收缩堆。（判断topchunk的大小是否大于mmap的收缩阈值，默认为128KB）。

只要堆顶的空间没释放，堆是一直不会收缩的。因为ptmalloc的内存收缩是从top chunk开始，如果与top chunk（堆顶的一个chunk）相邻的那个chunk在内存池中没有释放，top chunk以下的空闲内存都无法返回给系统，即使这些空闲内存有几十个G也不行。

按照这个测试程序分配后，内存变成由小块和大块交替出现，释放小块的时候，直接把小块放在fastbins中取，而且他的使用位还是1，释放大块的时候，它试图合并相邻的块，但是和它相邻的块的使用位还是1，所以它不能把相邻的块合并起来，而且释放的块的大小小于64K，也不会触发consolidate，即不会把fastbins清空，所以当所有的块都被释放完后，所有的小块都在fastbins里面，使用位都还是1，大块都挂在unsort队列里面。全部都无法合并。所以使用的堆更加无法压缩。如果在循环后面再分配2000字节然后释放的话，所有内存将全部被清空，这是因为再申请2000字节的时候，由malloc的第二步，程序会先调用consolidate，即把所有的fastbins清空，同时把相邻的块合并起来，等到所有的fastbins清空的时候，所有的块也被合并起来了，然后调用free(2000)的时候，这块将被合并起来，成为topchunk，并且大小远小于64K，所有堆将会压缩，内存归还给系统。
```


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


## 参考资料
   1. [glibc（ptmalloc）内存暴增问题解决 ](http://blog.chinaunix.net/uid-18770639-id-3385860.html)
   2. [ptmalloc,tcmalloc和jemalloc内存分配策略研究](https://www.owent.net/2013/07/ptmalloctcmalloc%E5%92%8Cjemalloc%E5%86%85%E5%AD%98%E5%88%86%E9%85%8D%E7%AD%96%E7%95%A5%E7%A0%94%E7%A9%B6.html)
