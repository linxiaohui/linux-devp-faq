#GDB常用命令

## watch变量
watch _varname_ 可以在变量值发生变化的时候中断执行。这对检查内存越界覆盖"无辜"变量很有用。缺点是执行会比较慢。


## gdb时打印完整的字符串
可以通过`set print element 0`命令,之后print命令显示完整的字符串   
或者看一下  p var@1是否能显示完整的字符串

## 关闭其它进程的文件描述符
gdb --pid=<pid>之后call close(<fd>)   
可以使用如下脚本   
```bash
#!/bin/sh
pid=$1
fd=$2
echo "closeing $fd of $pid"
gdb --quiet --pid=$pid -batch -ex 'call close($fd)'
```