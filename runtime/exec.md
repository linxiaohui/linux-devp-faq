# 程序的加载,运行和终止

为了支持用户程序的正确运行需要解决以下几个重要问题：
   * 加载用户程序以及它所依赖的所有共享对象
   * 对用户程序和共享对象进行符号解析和重定位
   * 向用户程序传递环境变量和命令行参数
   * 根据C++标准的规定, 全局对象(包括用户程序和共享库中定义)必须在`main()`执行前初始化,并在程序结束时以相反的顺序析构.

## 程序头(Program Header)
程序头在`System V Application Binary Interface`的`5.2 Program Header`一节中定义,
它规定了ELF文件中需要加载的segment与加载的地址以及是否需要动态链接器等信息(若需要动态链接器, PT_INTERP指定了动态链接器的路径)

## 初始化代码和终止代码(Initialization and Termination code)
   参见`Initialization and Termination Functions`一节

## 加载时重定位(Load-time Relocation)和运行时重定位(Run-time Relocation)
   参见`System V Application Binary Interface: Intel386 Architecture Processor Supplement`的`Procedure Linkage Table`一节

## 程序运行的基本流程
   1. 操作系统通过执行exec(3)系统调用将程序映射到内存
   2. 操作系统将`PT_INTERP`指定的动态链接器映射进内存, 传递它所需要的参数, 并跳到动态链接器的入口处开始执行
   3. 动态链接器自举`Bootstrap`, 对自己进行重定位, 并开始构造`符号表`
   4. 自举完成后, 动态链接器根据可执行文件`.dynamic`段中的`DT_NEEDED`加载依赖的共享对象, 并加入符号表. 该过程是是递归的, 当这个过程结束时,所有需要的共享对象都已加载进内存, 动态链接器也具有了程序和所有共享库的符号表
   5. 动态链接器重新遍历共享库, 并进行`加载时重定位`, 加载时重定位包括：
      * 对数据的引用: 在`.rel.dyn`段中, 初始化一个GOT(在.got中)项为一个全局符号的地址
      * 对代码的引用: 在`.rel.plt`段中, 初始化一个 GOT(在.got.plt)项为PLT表中第二条指令的地址
   6. 如果共享对象有初始化代码(在`.init`中), 动态链接器会执行它, 并将终止代码(在`.fini`中)记录下来以便退出时执行. 动态链接器不会执行用户程序的初始化代码, 它由用户程序的启动代码自己执行.
   7. 这个过程完成后, 所有的共享对象都已重定位并初始化, 动态链接器跳到用户程序的入口处开始执行. **注意**: 为了能在程序退出时让动态链接器有机会调用共享对象的终止代码, 动态链接器会传递一个终止函数(用以调用共享对象的终止代码)给用户程序
   8. 用户程序开始执行. 首先它注册动态链接器的终止函数和它自己的终止函数, 然后调用用户程序的初始化代码, 然后调用用户定义的`main()`函数.
   9. `main()`函数返回后, 以注册的相反顺序调用终止函数(先调用用户程序的终止函数,再调用动态链接器的终止函数), 最后调用`_exit()`退出进程

## 用户程序的执行
可执行文件入口处在`_start(glibc/sysdeps/i386/elf/Start.S)`. 它首先设置一些寄存器后调用`__libc_start_main()(glibc/csu/libc-start.c)`. `__libc_start_main()`主要进行以下工作:
   * 调用`__cxa_atexit()(glibc/stdlib/cxa_atexit.c)` 注册动态连接器通过EDX寄存器传过来的终止函数
   * 调用`__cxa_atexit()`注册用户程序的终止函数
   * 调用用户程序的初始化函数
   * 调用用户提供的`main()`函数
   * `main()`返回后调用`exit() (glibc/stdlib/exit.c)`.  `exit()`以注册的相反顺序调用`atexit()(glibc/stdlib/atexit.c)`和`__cxa_atexit()`注册的函数
   * 调用`_exit()`结束进程

# 参考资料
   1. [Linker and Loader](http://www.iecc.com/linker/)
   2. [SYSTEM V APPLICATION BINARY INTERFACE](./gabi41.pdf)
   3. [SYSTEM V APPLICATION BINARY INTERFACE Intel386Ô Architecture Processor Supplement](./sysv-abi-i386-4.pdf)
