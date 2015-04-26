#目标文件与可执行文件的属性

## 查看程序或库的信息
使用file命令可以查看可执行程序或库文件(.so)的一些信息，例如是32位还是64位的    
对于.a的库文件，可以使用nm查看里面的.o成员，也可以 ar x 解压处成员 file    
**备注**：    
`file -i 文本文件` 可以显示文件的编码方式     
检查文本文件格式，UTF-8格式需要注意是否有BOM标志(EF BB BF)    
`cat -v` 可以显示不可打印字符     
注意文件的UNIX格式或DOS格式     

## size
列出目标文件的各section的大小
   * 文本段：包含程序的指令，它在程序的执行过程中一般不会改变。
   * 数据段：包含了经过初始化的全局变量和静态变量，以及他们的值。
   * BSS段：包含未经初始化的全局变量和静态变量。

## 查看可执行文件依赖情况
ldd命令
Linux上ldd命令是一个 bash 的脚本。
Linux下执行ldd，设置LD_TRACE_LOADED_OBJECTS，而动态载入器检查该环境变量，
若设置了则不执行这个程序而是输出这个可执行程序所依赖的动态链接库。下面的命令都可以显示依赖的动态库     
  * `ldd /bin/grep`
  * `LD_TRACE_LOADED_OBJECTS=1 /bin/grep`
  * `LD_TRACE_LOADED_OBJECTS=1 /lib/ld-linux.so.2 /bin/grep`

## ldd注意事项
如果让别的装载器来取代系统默认的动态链接库（ld-linux.so）的话，
那么就可以让 ldd来载入并运行程序了——使用不同的载装器并且不处理LD_TRACE_LOADED_OBJECTS 环境变量，
而是直接运行程序。例如 uClibc C库。修改（ldso/ldso/ldso.c不处理LD_TRACE_LOADED_OBJECTS）并编译它后，
产生C库与ld-uClibc.so.0。编写程序并使用命令编译：
```shell
$ L=/home/you/app/uclibc
$ gcc -Wl,--dynamic-linker,$L/lib/ld-uClibc.so.0 \
    -Wl,-rpath-link,$L/lib \
    -nostdlib \
    myapp.c -o myapp \
    $L/usr/lib/crt*.o \
    -L$L/usr/lib/ \
    -lc
```
   * -Wl,–dynamic-linker,$L/lib/ld-uClibc.so.0 — 指定一个新的装载器。
   * -Wl,-rpath-link,$L/lib — 指定一个首要的动态装载器所在的目录，这个目录用于查找动态库。
   * -nostdlib — 不使用系统标准库。
   * myapp.c -o myapp — 编译myapp.c 成可执行文件 myapp,
   * $L/usr/lib/crt*.o — 静态链接runtime 代码
   * -L$L/usr/lib/ — libc 的目录（静态链接）
   * -lc —  C 库

这样若`LD_TRACE_LOADED_OBJECTS=1 /bin/grep` 实际上是以执行的用户的权限执行了这个程序(而不是像设想那样列出其以来的动态库)。
ldd脚本实际上是执行了`LD_TRACE_LOADED_OBJECTS=1 /lib/ld-linux.so.2 /bin/grep`
或`LD_TRACE_LOADED_OBJECTS=1 /lib64/ld-linux.so.2 /bin/grep` （取决于可执行文件是32位还是64位的）

.interp ELF section为程序的动态加载器。
`objdump –section=.interp –s ./myapp` 查看.interp段的内容     
或者   
`readelf –l ./myapp`

静态编译的可执行程序都可以作为动态加载器   
```shell
gcc -static myapp.c -o loader
gcc -Wl,--dynamic-linker,./loader myapp.c -o myapp
```
**备注**：在BSD 系统上的ldd 是一个C 程序。


