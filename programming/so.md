#动态库

## 编译为动态库
`gcc -fpic -shared`  
  * -fpic 使输出的对象模块是按照可重定位地址方式生成的
  * -fpie和 -fPIE与 -fpic及 -fPIC的用法很相似，区别在于前者总是将生成的位置无关代码看作是属于程序本身，并直接链接进该可执行程序，而非存入全局偏移表 GOT中。    

**备注**: `man gcc`: -fpic、-fPIC、-fpie、-fPIE  

## 使用动态库
若生成的对象模块需要使用共享库，链接时使用参数`-ldl`   
   * dlopen(char \* filename, int flag)    
filename: 指定共享库的名称。将会在下面位置查找指定的共享库:
       * 环境变量LD_LIBRARY_PATH列出的用分号间隔的所有目录;
       * 编译时加入-rpath连接选项（-Wl,-rpath,./），这个选项指定生成的ELF的RUNPATH
           * `readelf -d xxxx.so`
       * 文件`/etc/ld.so.cache`中找到的库的列表
           * 通过`/etc/ld.so.conf`中添加路径，然后执行`ldconfig`构建`/etc/ld.so.cache`
       * 系统目录
       * 当前目录
   * dlsym(void \*phandle, char \* funcname)   
       调用dlsym时，使用dlopen返回的共享库的phandle以及函数名称作为参数，返回要加载函数的入口地址。
       指定的phandle里没有，则在phandle自动加载的so中找。
   * dlclose()   
       如果dlopen了一个so后，cp -f了这个so，dlclose的时候进程会coredump。
   * dlerror()   
       该函数用于检查调用共享库的相关函数出现的上一次错误（因此不能重复调用）。

**备注**:    
   * ld.so和ld-linux.so 负责加载需要使用的动态库。
   * ld.so程序处理a.out类二进制程序；ld-linux.so处理ELF格式。
   除此之外两者一样，并使用相同的支持文件和程序，如ldd(1)，ldconfig(8)和/etc/ld.so.conf。
   * ld会将-l 指定的动态库作为依赖，而不管是否用到其中的定义。
   * ldd -u demo 查看不需要链接的so。
   * -Wl,--as-needed 不连接不依赖的so。
   * -z nodefaultlib编译选项禁止搜索缺省路径。
   * 使用 ld -verbose | grep -I search 查看so查找路径。

##动态库so的更新
   * Linux中 cp mv rm ln 命令对于 inode 和 dentry 的影响
       * inode （索引节点），包含文件的大部分信息：
           * 文件的字节数
           * 文件拥有者的User ID
           * 文件的Group ID
           * 文件的读、写、执行权限
           * 文件的时间戳
               * ctime:inode上一次变动的时间
               * mtime:文件内容上一次变动的时间
               * atime:文件上一次打开的时间。
           * 链接数:有多少目录项指向这个inode
           * 文件数据block的位置
       * 通过stat命令可以查看文件inode。
       * dentry （宏目录项）
           * 它包含文件名和指向inode的指针等信息
           * dentry可以找到对应的inode，再通过inode找到文件存储的block位置。
       * cp命令
           1.  若目标文件是新的文件: 分配一个未使用的inode号，在inode 表中添加一个新项目，
               如果是cp到一个已经存在的文件，则inode号采用被覆盖之前的目标文件的inode号。
           2. 在目录中新建一个目录项，并指向上步中的inode；
           3. 把数据复制到block中。
       * mv命令
           * 如果mv命令的目标和源文件所在的文件系统相同：
                1. 使用新文件名建立目录项；
                2. 删除带有原来文件名的目录项；    
**备注**：该操作对inode表除时间戳没有影响，对数据的位置也没有影响，不移动任何数据。
即使是mv到一个已经存在的目标文件，会先删除目标文件的目录项. 新目录项指向源文件inode.
           * 如果目标和源文件所在文件系统不相同，就是cp和rm；
       * rm命令
           * 减少链接数量，如果链接数为0释放inode（inode号码可以被重新使用）；
           * 如果inode被释放，则数据块放到可用空间列表中；
           * 删除目录中的目录项
       * ln命令
           * 硬链接 ln 文件名 链接名
                * 多个目录项指向同一个inode号；
                * 用rm来删除硬链接文件：
                    1. 仅是减少链接数量；
                    2. 只要有一个链接存在文件就存在；
                    3. 当链接数为零，文件就被删除了。
            * 软链接(符号链接) ln -s 文件名 链接名
                   * 符号链接的内容就是它所指向的文件名；
                   * 符号链接文件有自己的inode；
                   * rm删除的话只是删除这个链接文件。
   * cp的方式更新so    
`strace cp libx1.so libx2.so`    
可以看到有如下的系统调用
```c
open("libx1.so", O_RDONLY)=3
fstat(3, {st_mode=S_OFREG|0755, st_size=..........} ) =0
open("libx2.so", O_WDONLY|O_TRUNC )=4
```

关于动态库    
   1. 应用程序通过dlopen打开so的时候，kernel通过mmap把so加载到进程地址空间，对应于vma里的page。
   2. 在这个过程中loader会把so里面引用的外部符号(例如`malloc` `printf`等)解析成真正的虚存地址。
   3. 当so被cp trunc时，kernel会把so文件在虚拟内的页purge掉
   4. 当运行到so里面的代码时，因为物理内存中不再有实际的数据（仅存在于虚存空间内），会产生一次缺页中断。
   5. Kernel从so文件中copy一份到内存中去
       * 全局符号表并没有经过解析，当调用到时就产生segment fault
       * 如果需要的文件偏移大于新的so的地址范围，就会产生SIGBUS信号.
所以，如果用相同的so去覆盖
    * 如果so里面依赖了外部符号，coredump
    * 如果so里面没有依赖外部符号，不会coredump

上面的问题由于复制so时so被trunc了，因此如果不用turnc的方式就避免这个问题。
   * install的方式更新so    
`strace install new.so old.so`    
其系统调用如下
```c
unlink("libx2.so")
open("libx1.so", O_RDONLY)=3
fstat(3, {st_mode=S_OFREG|0755, st_size=..........} ) =0
open("libx2.so", O_WDONLY|O_CREAT|O_EXCL, 0755)=4
```
   * 先unlink再creat
   * unlink的时候，已经map的虚拟空间vma中的inode结点没有变(新的so和旧的so用的不是同一个inode结点)
   * 只有inode结点的引用计数为0时，kernel才将其删除。
   * 只有得启进程才会使用到新的so。

**备注**：   
不修改inode不trunc更新文件的一种方式 `dd conv=notrunc`    
在两个so关系比较明确，引用的外部符号地址不变的情况下时，可以考虑使用这种方式不重启进程更新so。

