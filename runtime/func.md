#函数调用关系

## 如何获取函数之间的调用情况
gcc编译选项`-finstrument-functions`可以记录运行时的函数调用和退出信息.
使用此选项时, 编译器会
   * 在所生成的汇编代码的每个函数的入口处增加对函数`__cyg_profile_func_enter()`的调用;
   * 在每个函数退出时增加对`__cyg_profile_func_exit()`的调用。

`man gcc`获取这些函数的详细使用方式。

## 栈调用关系跟踪
   * 使用glibc提供的backtrace函数和backtrace_symbols
   * 使用libunwind库
   * 根据函数调用规范, 如果有N个参数，将N个参数压栈自右向左顺序压栈，然后是将返回地址压栈，最后是将ebp压栈保存起来。
示例代码
```c
#include <dlfcn.h>
void **getEBP( int dummy )
{
	/*原理:参数的地址下面是返回地址，返回地址的下面是被保存的ebp的地址*/
	    void ** ebp = (void ** )&dummy -2 ;
	    return  * ebp;
	}
	void print_walk_backtrace( void )
	{
	    int dummy;
	    int frame = 0;
	    Dl_info dlip;
	    void ** ebp = getEBP( dummy );
	    void ** ret = NULL;
	    printf( "Stack backtrace:\n" );
	    while( ebp )
	    {
	        ret = ebp + 1;
	        dladdr( * ret, &dlip );
	        printf("Frame %d:[ebp=0x%08x][ret=0x%08x] %s\n",
	                frame++, * ebp, * ret, dlip.dli_sname );
	        ebp = (void**)(* ebp);
	        // get the next frame pointer
	    }
	    printf("-------------------------------------\n");
	}
```
   * \__builtin_return_address  （info gcc: C Extensions:Return Address）


## 关于调试与优化
gcc编译参数`-fomit-frame-pointer`在函数调用时可以减少几条切换栈基址的指令，
并且基址寄存器被用作通用寄存器，会有性能提升。
但是由于堆栈中没有存储rbp,那么从add函数并不能追溯整个函数的调用栈。
参数`-fno-omit-frame-pointer`禁用把EBP当作通用寄存器来使用。

可以不通过RBP,而是使用libunwind。

## abort不能backtrace
有版本的abort不能backtrace，因为gcc内置赋予了abort noreturn的属性，编译器会对其优化，
函数第一条指令不是push返回地址，因此backtrace时不知道abort返回地址。

## 递归导致栈空间满不能backtrace
使用sigaltstack 设置signal stack context
该函数函数原型如下：
```c
#include <signal.h>
int sigaltstack(const stack_t * ss, stack_t * oss);
typedef struct {
    void  * ss_sp;     // Base address of stack
    int    ss_flags;   // Flags
    size_t ss_size;    // Number of bytes in stack
} stack_t;
```
要想创建一个新的可替换信号栈，ss_flags必须设置为0，ss_sp和ss_size分别指明可替换信号栈的起始地址和栈大小。系统定义了一个常数`SIGSTKSZ`，该常数对极大多数可替换信号栈来说都可以满足需求，`MINSIGSTKSZ`规定了可替换信号栈的最小值。
如果想要禁用已存在的一个可替换信号栈，可将ss_flags设置为SS_DISABLE。
而sigaltstack第一个参数为创建的新的可替换信号栈，第二个参数可以设置为NULL，如果不为NULL的话，将会将旧的可替换信号栈的信息保存在里面。


## addr2line
backtrace函数的输出： XXX.so(func+offset)
```bash
nm XXX.so | grep func
addr2line -e XXX.so func_addr+offset
```
