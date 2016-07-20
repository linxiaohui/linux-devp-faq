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


另一种方式(摘自网络)
```c
/*
如果源代码编译时使用了-O1或-O2优化选项
可执行代码会把ebp/rbp/rsp寄存器当作普通寄存器使用，导致backtrace失败。
为了防止这种情况发生，可以在编译时使用-O2  -fno-omit-frame-pointer  或-Og 来避免优化中使用上述寄存器。
 */
#define STACKCALL __attribute__((regparm(1),noinline))  
void ** STACKCALL getEBP(void){  
        void ** ebp=NULL;  
        __asm__ __volatile__ ("mov %%rbp, %0;\n\t"  
                    :"=m"(ebp)      /*输出*/  
                    :      /*输入*/  
                    :"memory");     /*不受影响的寄存器*/  
        return (void ** )(* ebp);  
}  
int my_backtrace(void ** buffer,int size){  

    int frame=0;  
    void ** ebp;  
    void ** ret=NULL;  
    unsigned long long func_frame_distance=0;  
    if(buffer!=NULL && size >0)  
    {  
        ebp=getEBP();  
        func_frame_distance=(unsigned long long)(* ebp) - (unsigned long long)ebp;  
        while(ebp&& frame<size  
            &&(func_frame_distance< (1ULL<<24)) /*assume function ebp more than 16M*/
            &&(func_frame_distance>0))  
        {  
            ret=ebp+1;  
            buffer[frame++]= * ret;  
            ebp=(void**)(* ebp);  
            func_frame_distance=(unsigned long long)(* ebp) - (unsigned long long)ebp;  
        }  
    }  
    return frame;  
}  
```

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

一个检查错误所在代码行的脚本:
```bash
#/bin/sh
#脚本接受一个参数，格式为 /path/to/booking.so(initfunc+0xea)
loc=$1
#获取动态库的名称
lib=$(echo $loc | awk -F[+\(\)] '{print $1}')
#获取函数名
func=$(echo $loc | awk -F[+\(\)] '{print $2}')
#获取偏移地址
offset=$(echo $loc | awk -F[+\(\)] '{print $3}')
#获取函数的地址
base=$(nm $lib | grep -w $func | awk '{print $1}')
echo "lib=$lib"
echo "func=$func"
echo "offset=$offset"
echo "FIND FUNC $(nm $lib | grep -w $func)"
echo "base=$base"
#计算函数地址+偏移的16进制值(发生coredump的地方)
let b=0x$base
let o=offset
let x=b+o
addr=$(echo "obase=16;$x" | bc)
echo "addr=$addr"
#使用addr2line获取所在代码及其行号
addr2line -e $lib 0x$addr
#可以加上-f参数现在所在的函数名
```
