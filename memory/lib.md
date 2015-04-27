#检查内存的第三方库和工具

## Electric Fence检测内存越界操作
Electric Fence 使用虚拟内存技术，在 malloc() 分配到的内存块的前后分别放置一个不能访问的内存页，
当应用程序去读写这两个内存页时硬件会引发段错误。
* 安装（OpenSUSE中）
```bash
zypper install ElectricFence
rpm -ql ElectricFence
```
* 使用
   * 添加链接选项 -lefence 链接 libefence.so，它会在程序运行时预先加载 libefence.so Hook 对 malloc等的调用。
   * 不用编译程序而通过设置 PRE_LOAD=/usr/lib64/libefence.so 环境变量
   * 直接链接 /usr/lib64/libefence.a
   * 默认情况下 Electric Fence 只会检测读写越下边界和已释放的内存块的违规操作
   * Electric Fence 不能与其他内存检测工具或者内存分配器使用
* 环境变量
   * EF_ALIGNMENT 变量控制分配到的内存块的对齐大小，值越小越严格。
   * EF_PROTECT_BELOW=1 时在内存块的之前也添加一个无法访问的内存页。检查访问越过内存块上边界的情况。
   * EF_PROTECT_FREE: Electric Fence 通常会将释放的内存放到一个内存池中，以后这块内存块可以被 realloc 用来重新分配。若怀疑程序中有访问内存池中的 空闲内存块的嫌疑，可以将 EF_PROTECT_FREE 设置为 1 来检测这种情况。当将 EF_PROTECT_FREE 设置为 1, 则 Electric Fence 不会将释放的内存重新分配，这样就可以检测到错误。
   * EF_ALLOW_MALLOC_0 ，避免将malloc(0)作为错误。
   * EF_FILL,设置指定的值使 Electric Fence 在分配内存之后 将内存块初始化为此值。

## DUMA 检测内存违规访问和内存泄漏
D.U.M.A(Detect Unintended Memory Access)是 Electric Fence 的加强版, 能定位C++程序内存非法访问的位置与内存泄露。  
为了收集内存分配的语句所在的位置，DUMA 需要对 malloc() 等函数进行“Hook”，这是用宏实现的.  
* 需要包含头文件`#include <duma.h>`（对C++程序`#include <dumapp.h>`）
* 需要链接 libduma.a 和 pthread 库。   
`g++ -g -O0 heap-corruption.cc -o heapC -Wl,-Bstatic,-lduma -Wl,-Bdynamic –pthread`
* 环境变量   
DUMA和Electric Fence一样，同样支持通过变量来控制其行为，只不过比Electric Fence要多。常用如下：
   * DUMA_ALIGNMENT ：对应 EF_ALIGNMENT。
   * DUMA_PROTECT_BELOW：对应 EF_PROTECT_BELOW。
   * DUMA_FILL：对应 EL_FILE，默认为 0xFF。
   * DUMA_ALLOW_MALLOC_0：对应 EF_ALLOW_MALLOC_0。
   * DUMA_SLACKFILL：DUMA内部分配内存时以页为单位，如果申请的内存大小小于页大小，则未使用的空间会填充 DUMA_SLACKFILL 指定的值。默认为 0xAA。
   * DUMA_CHECK_FREQ：对于分配页中未使用的空间，这个值指定检查这个区域的频率，n 代表多少次内存分配或释放时就去检查；如果为1则表示每次内存分配和释放都检查；默认为 0 ，表示只有在内存释放的时候才去检查。频率越高性能损耗越大。
   * DUMA_MALLOC_0_STRATEGY：当 malloc(0) 的参数为0 时的返回值策略:
      * 0 – 和 DUMA_ALLOW_MALLOC_0 = 0时的行为一样，会终止应用程序。 
      * 1 – 返回 NULL 
      * 2 – 总是返回指向某一个受保护的页(Page)的指针，这样在使用时和 0 一样。 
      * 3 – 返回一个受保护的页中间地址。
   * DUMA_NEW_0_STRATEGY：当 new 的大小参数为 0 时的返回值策略: 
      * 2 – 和 DUMA_MALLOC_0_STRATEGY 为2时一样。
      * 3 – 和 DUMA_MALLOC_0_STRATEGY 为3时一样。
   * DUMA_PROTECT_FREE：其作用和 EF_PROJECT_FREE一样。默认为 -1，表示已经开启。如果为 n(n > 0)， 则表示允许空闲内存的总大小为 n(KB)。
   * DUMA_REPORT_ALL_LEAKS：DUMA 通常只会报告那些能查找对应文件名和行号的导致内存泄漏的语句。如果此值为 1 则会报告所有的内存泄漏错误。默认为 0。
   * DUMA_MALLOC_FAILEXIT：很多程序并没有检查内存分配失败的情况，这会导致在失败的情况下会延后才会体现出来。 设置为正数表示失败时就终止程序。
   * DUMA_MAX_ALLOC：表示最多能分配的总的内存大小。如果为正数表示最大能分配 n(KB)。默认为 -1,表示无限制。
   * DUMA_FREE_ACCESS：这个选项可以帮助调试器捕捉到内存释放的动作。如果此值为非0,则表示开启。默认关闭。
   * DUMA_SHOW_ALLOC：是否在每次进行内存分配和释放时都打印到终端。这有助于检查内存的分配和释放的情况。 默认关闭。
   * DUMA_SUPPRESS_ATEXIT：是否在程序退出时忽略 DUMA 自定义的 exit 函数。这个函数用于检查内存泄漏，通常情况下不应该跳过。默认为禁止忽略。
   * DUMA_DISABLE_BANNER：禁止打印 DUMA 启动信息。默认不禁止。
   * DUMA_OUTPUT_STDOUT：将信息打印到 stdout。默认关闭。
   * DUMA_OUTPUT_STDERR：将信息打印到 stderr。默认开启。
   * DUMA_OUTPUT_FILE：将信息打印到指定文件中。默认关闭。

## jemalloc与tcmalloc
   * jemalloc是FreeBSD默认的内存分配器。
   * tcmalloc详情参考后文关于google-perftools的描述。

## 关于 dmalloc的一些使用经验
* 安装 
下载dmalloc源代码，执行
```bash
./configure --enable-threads
make
make install
```
* 备注
   * dmalloc的源代码中有文件settings.dist，其中有一些宏定义。 例如，将 ALLOW_FREE_NULL_MESSAGE 设置为 0 可以不输出 WARNING .... FREE 0 .... 的警告
   * 把dmalloc安装后的lib路径加入到/etc/ld.so.conf中，然后执行ldconfig -v
   * 在.bashrc, .profile或.zshrc文件中添加
```bash
function dmalloc { eval `/usr/local/bin/dmalloc -b $*`; }
```
* 使用
 执行`dmalloc -l logfile -i 100 low`
 在要检测程序中添加：
```c
#ifdef DMALLOC
#include "dmalloc.h"
#endif
```
 如果想在程序不退出时观察监测结构，需要在程序中使用函数
```c
dmalloc_debug_setup(getenv("DMALLOC_OPTIONS"));//初始化
```
在需要监测的程序段的前后分别使用函数
```c
mark = dmalloc_mark();
dmalloc_log_changed(mark,1,0,1);
```
执行程序，在logfile中观察结果
备注：
必须在相关的源文件中 #include "dmalloc.h" 在输出的日志文件中才能显示代码的哪行malloc的内存没有free，否则只能显示地址
必须在 #include <stdlib.h> 之后 #include "dmalloc.h" 否则编译会报错

* 根据结果分析数据
若在logfile中出现
```
not freed: '0x400177c8|s1047' (20 bytes) from 'ra=0x0x400618'
```
则说明程序存在内存泄露；
 
备注:
   * 环境变量  DMALLOC_OPTIONS=log=/path/logfile,debug=0x.......
   * 使用 `dmalloc -DV` 可以显示各个参数的意义

## 定位内存泄露代码所在行
使用**addr2line**
```bash
#/bin/sh
loc=$1
echo "loc=$loc"
lib=$(echo $loc | awk -F[+\(\)] '{print $1}')
func=$(echo $loc | awk -F[+\(\)] '{print $2}')
offset=$(echo $loc | awk -F[+\(\)] '{print $3}')
echo "lib=$lib"
echo "func=$func"
echo "offset=$offset"
echo "FIND FUNC $(nm $lib | grep -w $func)"
base=$(nm $lib | grep -w $func | awk '{print $1}')
echo $base
let b=0x$base
let o=offset
let x=b+o-1
echo $x
addr=$(echo "obase=16;$x" | bc)
echo $addr
addr2line -e $lib 0x$addr
```