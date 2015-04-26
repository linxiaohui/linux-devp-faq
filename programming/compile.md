#编译相关环境变量

## 头文件的搜索路径    
`C_INCLUDE_PATH`(C头文件)    
`CPLUS_INCLUDE_PATH`(C++头文件)   
这两个环境变量指明的路径会在GCC命令行参数-I指定的路径之后，系统默认路径之前进行搜索。   
## 库搜索路径   
`LIBRARY_PATH`    
此环境变量指明路径会在-L指定路径之后，系统默认路径之前被搜索。   

**案例分析**   
一次在编译项目代码时，有个文件报错sqrt函数没有声明，但源文件中有`#include <math.h>`。
使用`gcc -E`查看预编译后的结果发现没有sqrt等函数的声明，显然不是标准库头文件。
后检查发现，在标准库头文件路径之前的include路径中有第三方库的math.h，但其声明的函数等与标准库无关，因此导致错误。    

由此案例可以看出GCC编译时将警告作为错误（-Werror）以及适当提高GCC编译的警告级别对程序的正确性有比较大的帮助。
否则，在这种情况下编译通过但sqrt的返回值会被作为整数存取，从而导致不可预知的错误。

## 编译时检查32位/64位的方式
例如`__WORDSIZE`的定义（在`/usr/include/gnu/stubs.h`里）
```c
#if defined __x86_64__
# define __WORDSIZE	64
# define __WORDSIZE_COMPAT32	1
#else define __i386__
# define __WORDSIZE	32
#endif
```

## 如何在编译结果中加入编译信息
定义src.c   
```c
BUILDINFO;
```
在Makefile中    
```
buildinfo="const char buildinfo[]=\"`cat ~/.baseline` Built on `uname -n` by `id -un` at `date`\""
gcc -DBUILDINFO=$(buildinfo) src.c
```
这样在结果里就定义了buildinfo字符数组，其值为运行make时的shell命令生成的内容。可以使用`strings`命令查看。


## 如何32位程序读写超过2G的文件
**_方式1：使用fopen 标准库_**    
`gcc -D _FILE_OFFSET_BITS=64 file_test.c -o file_test`   

**_方式2：使用open系统调用_**    
对与open，可以使用O_LARGEFILE参数，即:    
`fd = open("./bill_test",O_LARGEFILE|O_APPEND|O_RDWR,0666);`    
fopen没有这个参数，只能按照方法一定义宏`_FILE_OFFSET_BITS`来解决。  

**备注**： 解决方式2在AIX上也是适用的； AIX上相关的C库函数是fopen64。    
**备注**： 如若需要在64位系统64位程序中限制日志文件的大小，可以考虑`ulimit -f`。但是需要注意其影响，这种情况下若文件读写大于`ulimit -f`会产生`SIGXFSZ`信号。    



#符号重名问题
## 链接时忽略函数的重复定义
`gcc -Xlinker -zmuldefs`    
**注意**: -Xlinker-zmuldefs 是将 -zmuldefs传给ld，因此不能解决编译过程中的重复定义。

## 如何避免编译可执行文件时才发现函数未定义
**Linux**    
`gcc -shared -fPIC -Wl,--no-undefined x.c -o libx.so`    

**AIX**    
`xlc -bernotok xxx.so`

## GCC编译过程
GCC链接时默认是从前往后搜索，当在.o文件中找到的话，将会跳过后面的.a静态库，
但是如果是先在.a静态库找到的话，还会搜索后面的.o文件，如有重复定义则会报错。     
**原因如下**：    
链接器从左到右按照它们在编译器命令行上出现的顺序来扫描目标文件和静态链接库。整个扫描过程中，链接器维持3个集合：   
**E**：可重定位目标文件的集合（这个集合中的文件被合并起来形成可执行文件）    
**U**：未解析的符号集合（引用了但尚未定义的符号）   
**D**：前面输入文件中已定义的符号集合   
流程：   
1. 对于命令行上的每个输入文件f，链接器会判断f是一个目标文件还是一个存档文件(archive)。
  * 如果f是一个目标文件，那么链接器把f添加到E，修改U和D来反映f中的符号定义和引用，并继续下一个输入文件。    
  * 如果f是一个存档文件，那么链接器就尝试匹配U中未解析的符号和由存档文件成员定义的符号。
     * 如果某个存档文件成员m，定义了一个符号来解析U中的一个引用，那么就将m加到E中，并且链接器修改U和D来反映m中的符号定义和引用。
     * 对存档文件中所有的成员目标文件都反复进行这个过程，直到U和D都不再发生变化。
     * 在此时，任何不包含在E中的成员目标文件都被丢弃，而链接器将继续到下一个输入文件。     
2. 如果链接器完成对命令行上输入文件的扫描后，U是非空的，那么链接器会输出一个错误并终止。
否则，它会合并和重定位E中的目标，从而构建输出的可执行文件。

因此：库的顺序是重要的，如果一个库依赖另外一个库，被依赖的库在命令行中需要在后面。    
另，如果使用项目在使用第三方库等场合有循环依赖的情况，可以使用下面的命令解决编译问题   
`gcc -Wl,Xlinker "-(" -lA -lB  -lC "-)" `

## 符号重名总结
  * 一个全局变量或函数在多个 .c源文件中定义，并且这些 .c编译出的.o 文件被链接到同一个动态库 .so 或一个可执行文件中时，
  ld会报错重复定义。
  * 如果这些 .o被ar 到一个静态库中，不会报错。
  * 如果一个全局变量或函数在多个 .so中定义，ld 不报错，以第一个为准。
  * ld  为.so 的时候，允许有未定义的符号，在 ld为可执行文件的时候，不能有未定义的符号。

## 动态库和可执行文件中的符号
  * 如果在创建动态链接的可执行文件不加`-Wl,--export-dynamic`选项，则它所export的动态符号仅仅包括在链接时动态对象所用到的，
  * 如果动态库中的函数调用了可执行文件中的函数（主程序优先），则会找不到符号。    
  * 如果二者中都定义了同样的符号，编译动态库时加入`-fvisibility=hidden`可以避免调用可执行文件中的符号。
此时，需要使用 `__attribute ((visibility("default")))` 显式的声明被外部调用的函数。

## 如何查看动态符号表
`objdump -T a.out`


## 强符号与弱符号
已初始化的全局变量是强符号（strong symbol）；未初始化的全局变量是弱符号（weak symbol）   
链接时候的使用规则
   * 不允许有多个强符号。
   * 如果一个强符号和多个弱符号同时存在，那么使用强符号。
   * 如果有多个弱符号，那么选择使用弱符号中占用空间最大的那一个。
因此，如果出现重复定义，比如两个全局变量，第一个初始化了，第二个没有，
那么第二个变量所指向的符号其实是第一个符号，这可能导致很多潜在问题。    
自定义全局变量最好初始化，冲突了编译器会报错。    
最好在编译时指定`-fno-common`参数，会警告那些重复出现的符号，无论强弱。  

## 静态链接动态链接
gcc默认使用动态链接，若要静态链接需要指定参数`-static`    
若要同时使用静态链接和动态链接： `-Wl,-Bstatic -l3rdparty -Wl,-Bdynamic`    

#GCC扩展

## __attribute__

详细参见`info gcc`。比较重要的有：    
在main函数执行前执行特定的函数
```c
__attribute__((constructor)) void before_main();
__attribute__((destructor)) void after_main();
```
format
```c
__attribute__((format(printf,m,n)))
```

## 数组初始化
数组初始化（C99标准）
```c
int my_array[6] = { [4] = 29, [2] = 15 };
```
GNU 扩展：在需要将一个范围内的元素初始化为同一值时，可以使用 `[first ... last] = value `这样的语法。


## 如何得到一个函数地址属于哪个动态库
使用gnu的扩展函数`dladdr`（`man dladdr`可以查看使用方式）。    
另外`dl_iterate_phdr`可以查到当前进程所装在的所有符号，每查到一个就会调用指定的回调函数。
