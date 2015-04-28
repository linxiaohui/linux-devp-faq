#目标文件与可执行文件的属性

## 如何检查一个库文件或可执行文件是32位的还是64位的
`file _a.out_` 或者 `file _libX.so_`   
对于.a的库文件，可以 ar x 解压出成员文件后使用 `file`命令
   * 使用nm查看.a里面的.o成员的地址长度也可以判断为32位或64位的

**备注**：    
   * `file -i _textfilename_` 可以显示文件的编码方式     
      * 检查文本文件格式，UTF-8格式需要注意是否有BOM标志(EF BB BF)
      * `cat -v` 可以显示不可打印字符
      * 注意文件的UNIX格式或DOS格式
   * 可以使用`enca`检查文件编码。其它的如`universalchardet` `icu`

## size
列出目标文件的各section的大小
   * 文本段：包含程序的指令，它在程序的执行过程中一般不会改变。
   * 数据段：包含了经过初始化的全局变量和静态变量，以及他们的值。
   * BSS段：包含未经初始化的全局变量和静态变量。

## 查看可执行文件依赖情况
Linux上`ldd`命令是一个 bash 的脚本，执行ldd，设置环境变量`LD_TRACE_LOADED_OBJECTS`，而动态载入器检查该环境变量，
若设置了则不执行这个程序而是输出这个可执行程序所依赖的动态链接库。    
下面的命令都可以显示依赖的动态库:    
  * `ldd /bin/grep`
  * `LD_TRACE_LOADED_OBJECTS=1 /bin/grep`
  * `LD_TRACE_LOADED_OBJECTS=1 /lib/ld-linux.so.2 /bin/grep`

## ldd注意事项
如果让别的装载器来取代系统默认的动态链接库（ld-linux.so）的话，
那么就可以让 ldd来载入并运行程序了——使用不处理`LD_TRACE_LOADED_OBJECTS`的载装器
(例如 uClibc C库（修改`ldso/ldso/ldso.c`不处理`LD_TRACE_LOADED_OBJECTS`）并编译产生C库与ld-uClibc.so.0)
编写程序并使用命令编译：
```bash
$ L=/path2uclibc
$ gcc -Wl,--dynamic-linker,$L/lib/ld-uClibc.so.0 \
    -Wl,-rpath-link,$L/lib \
    -nostdlib \
    testapp.c -o testapp \
    $L/usr/lib/crt*.o \
    -L$L/usr/lib/ \
    -lc
```
   * -Wl,–dynamic-linker,$L/lib/ld-uClibc.so.0  指定一个新的装载器。
   * -Wl,-rpath-link,$L/lib : 指定一个首要的动态装载器所在的目录，这个目录用于查找动态库。
   * -nostdlib : 不使用系统标准库。
   * $L/usr/lib/crt*.o : 静态链接runtime 代码
   * -L$L/usr/lib/ : libc 的目录（静态链接）
   * -lc —  C 库

这样若`LD_TRACE_LOADED_OBJECTS=1 /bin/grep` 实际上是以执行的用户的权限执行了这个程序(而不是像设想那样列出其依赖的动态库)。

ELF文件中`.interp`段为程序的动态加载器:
   * 查看.interp段的内容
      * `objdump –section=.interp –s ./testapp` 
      * `readelf –l ./testapp`

静态编译的可执行程序都可以作为动态加载器   
```bash
gcc -static testapp.c -o loader
gcc -Wl,--dynamic-linker,./loader testapp.c -o testapp
```
**备注**：在BSD 系统上的ldd 是一个ELF程序。


