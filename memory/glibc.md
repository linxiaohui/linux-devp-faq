#Glibc中关于内存的函数

## __malloc_hook
glibc中定义了函数指针变量__malloc_hook等。
根据glibc中malloc的实现(malloc/malloc.c)，如果这个变量不为空则执行其指向的函数，否则进行内存分配。
因此使用这种技术可以实现内存分配的跟踪等功能。例如，设置全局变量记录程序使用的动态内存的大小，代码如下：
```c
#include <malloc.h>
#include <stdlib.h>

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
size_t malloc_usage;
size_t const malloc_align=8;

void* malloc_hook(size_t s, void const* x)
{
    __malloc_hook = 0;
    size_t* p = (size_t*)malloc(s + malloc_align);
    __malloc_hook = malloc_hook;
    if(likely(p))
    {
        *p = s;
        malloc_usage += s;
        return (char*)p + malloc_align;
    }
    return 0;
}

void free_hook(void* p, void const* x)
{
    if(likely(p))
    {
        p = (char*)p - malloc_align;
        malloc_usage -= *(size_t*)p;
    }
    __free_hook = 0;
    free(p);
    __free_hook = free_hook;
}
void* realloc_hook(void* q, size_t t, void const* x)
{
    if(q)
    {
        q = (char*)q - malloc_align;
        malloc_usage -= *(size_t*)q;
    }
    size_t s = t;
    if(s)
        s += malloc_align;
    __realloc_hook = 0;
    __malloc_hook = 0;
    __free_hook = 0;
    size_t* p = (size_t*)realloc(q, s);
    __free_hook = free_hook;
    __malloc_hook = malloc_hook;
    __realloc_hook = realloc_hook;

    if(likely(p))
    {
        *p = t;
        malloc_usage += t;
        return (char*)p + malloc_align;
    }
    return 0;
}

void* memalign_hook(size_t y, size_t z, void const* x)
{
    abort(); // not implemented
}
void init_malloc_hook()
{
    __malloc_hook = malloc_hook;
    __realloc_hook = realloc_hook;
    __memalign_hook = memalign_hook;
    __free_hook = free_hook;
}

void(* __MALLOC_HOOK_VOLATILE __malloc_initialize_hook)(void) = init_malloc_hook;
int main()
{
        void * ptr= malloc(100);
        int i=0;
        for(i=0;i<10;i++)
        {
                ptr=malloc(123);
        }
        free(ptr);
        printf("%d\n", malloc_usage );
}
```
上面的代码编译会报错`warning: '__malloc_hook' is deprecated (declared at /usr/include/malloc.h:153) [-Wdeprecated-declarations]`
因为其不适用与多线程的环境。若果在单线程环境使用该技术，可以仿照malloc的实现，
利用`__builtin_return_address(0)`与`__libc_malloc`实现`__malloc_hook`的功能

## __libc_malloc等函数
Glibc中malloc等函数实际上是调用了`__libc_malloc`等函数;
后者可以用来实现自己的malloc以"重载"标准库中的malloc
可以测试时使用，结合LD_PRELOAD等。
需要注意的是free必须支持free(NULL)。否则程序可能尚未执行即coredump。
使用`__libc_malloc`函数实现malloc，若其中需要读写文件不能使用C标准库函数，需要使用系统调用。

## mallopt函数
`MMAP_THRESHOLD`决定malloc时使用sbrt还是mmap
设置内存分配参数，控制内存分配函数的行为

## mcheck、mtrace
使用`mtrace`简单的进行内存泄漏检测，它hook malloc(), realloc(), memalign(), calloc() 和 free() ，
对分配和释放内存的操作进行配对检测，如果发现有内存泄漏的情况， 会记录导致内存泄漏的分配函数调用所在的位置，
并将记录保存到环境变量 `MALLOC_TRACE` 指定的文件中，然后就可以使用 mtrace 命令来查看日志了。
man mtrace
info mtrace
查阅`glic manual （info libc）: Memory(Virtual Memory Allocation And Paging):Memory Allocation: Unconstrained Allocation: Heap Consistency Checking`
注意使用mtrace函数必须在malloc等函数调用之前调用，需引用头文件`#include <mcheck.h>`
另，可以`-lmcheck`连接`mcheck`库。
查阅环境变量`MALLOC_CHECK_ `


## libmemusage 统计内存使用
glibc 自带了一个 libmemusage 的库，用于收集应用程序运行时的内存使用情况。使用起来很简单，只要在编译的时候添加 -lmemusage 即可。
它使用 api hook 技术对 malloc， realloc，calloc和free 的调用进行监视，统计相应大小内存块的使用比率，并可给出简单的内存申请与释放的统计信息，可以用于简单的判断是否有内存泄漏。
