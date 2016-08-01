# Shell脚本编程

## if

if list; then list; elif list; then list; ... else list; fi

`man bash`中`SHELL GRAMMAR`
```
Lists
    A list is a sequence of one or more pipelines separated by one of the operators
    ;, &, &&, or ||, 
    and optionally terminated by one of
    ;, &, or <newline>.
```

```
The return status of a pipeline is the exit status of the last command, unless the pipefail option is enabled.
If  pipefail  is  enabled,  the  pipeline's return  status is 
the value of the last (rightmost) command to exit with a non-zero status,
or zero if all commands exit successfully.
```

## while/until
```
while list-1; do list-2; done
until list-1; do list-2; done
```
```bash
#!/bin/bash
count=0
until ! [ $count -le 100 ]
do
    echo $count
    count=$[$count+1]
done
```

## case
```bash
case $1 in
        pattern1)
        list1
        ;;
        pattern2)
        list2
        ;;
        pattern3)
        list3
        ;;
esac
```
pattern就是bash中“通配符”的概念. 不是正则表达式。

## for
```
for name [ [ in [ word ... ] ] ; ] do list ; done
```
```
for (( expr1 ; expr2 ; expr3 )) ; do list ; done
```

## select

## continue/break
```
linux@linux-virtual-machine ~ $ type break
break 是 shell 内建
linux@linux-virtual-machine ~ $ type continue
continue 是 shell 内建
```
可以用在 for，while，until和select循环中

## 脚本参数
$0：命令名。
$n：n是一个数字，表示第n个参数。
$#：参数个数。
`$*`：所有参数列表。
`$@`：所有参数列表。

$?：上一个命令的返回值。
$$：当前shell的PID。
$!：最近一个被放到后台任务管理的进程PID。
$-：列出当前bash的运行参数，比如set -v或者-i这样的参数。


## 数组
```bash
#!/bin/bash
#定义一个一般数组
declare -a array
#为数组元素赋值
array[0]=1000
array[1]=2000
array[2]=3000
array[3]=4000
#直接使用数组名得出第一个元素的值
echo $array
#取数组所有元素的值
echo ${array[*]}
echo ${array[@]}
#取第n个元素的值
echo ${array[0]}
echo ${array[1]}
echo ${array[2]}
echo ${array[3]}
#数组元素个数
echo ${#array[*]}
#取数组所有索引列表
echo ${!array[*]}
echo ${!array[@]}
#定义一个关联数组
declare -A assoc_arr
#为关联数组赋值
assoc_arr[abc]='abc'
assoc_arr[def]='def'
assoc_arr[xyz]='xyz'
```

## /bin/bash v.s. /bin/sh
运行时argv[0]不一样，程序中可能根据此有不同的处理逻辑。
