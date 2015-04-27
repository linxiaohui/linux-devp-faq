#注意事项

## ftok
ftok不能保证唯一性。根据`man ftok`与ftok的源代码，生成的key_t只包括文件inode的低2字节。   
使用下面的命令检查文件的inode的低字节
```bash
ls -a -i -l | awk ‘{printf(“%04X %s %d\n”, and($1,0xffff), $NF, $1)}’ \
| sort | awk ‘{print $2,$3,$1}’ | uniq -D -f2
```

## signed char v.s. unsigned char
C标准表示类型`char`可以带符号也可以不带符号，
   * 由具体的编译器、处理器或由它们两者共同决定
默认GCC x86_64下char是signed char; 这与AIX下不一样.
编译器参数   
   * AIX：    
   `-qchars=[signed or unsigned]`
   * Linux：    
   `-fsigned-char funsigned-char`

## sizeof
`sizeof(int) < -1`    
   * sizeof 返回的类型是 unsigned int
   * unsigned int 与 int 进行运算还是 unsigned int     
-1 和 unsigned int 比较，会先把 -1 转化为 unsigned int，这样 -1 的 unsigned int 就很大了。

`sizeof(i++)`    
   * 实际上在编译时这个值已经确定了，即i不会自增

## continue
for 循环遇到 continue 会执行for 小括号内的第三个语句。while 和 do...while 则会跳到循环判断的地方。

## 宏
```
Macro arguments are completely macro-expanded before they are substituted into a macro body,
unless they are stringified or pasted with other tokens. 
After substitution, the entire macro body, including the substituted arguments, 
is scanned again for macros to be expanded. 
The result is that the arguments are scanned twice to expand macro calls in them.
```

## pread, pwrite
在文件描述符给定的位置偏移上读取或写入数据

pread与read的区别: pread每次读取都要指定offset，这样防止了当有多个线程读取文件时，read之间的相互干扰。
因为read时，对于在同一fd上的读写是共用的，会共用一个记录offset的结构。
由于lseek和read调用之间，内核可能会临时挂起进程，所以对同步问题造成了问题，
调用pread相当于顺序调用了lseek 和 read，这两个操作相当于一个捆绑的原子操作。
