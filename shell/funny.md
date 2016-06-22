#其它

## Funny

telnet towel.blinkenlights.nl

##
```bash
tr -c "[:digit:]" " " </dev/urandom|dd cbs=$COLUMNS conv=unblock|GREP_COLOR="1;32" grep --color "[^ ]"
```

## fork bomb
`:(){ :|:& };:`
上面的语句等同于
```bash
:()
{
:|:&
}
;
:
```
定义了函数名为 `:` 然后递归发调用fork进程

备注:
   * 一行命令注意其中 {和:之间有空格
   * 为了防止fork炸弹，方法就是限制用户能够启动的进程数。
      * 编辑`/etc/security/limits.conf`增加 `* hard nproc 200` 将用户的进程数限制为200
      * root账户不受这个限制

## chmod
`chmod -x /bin/chmod`后，如何恢复(假定有适当的权限)
### 方法1 重装 coreutils
### 方法2 安装 busybox, 执行`busybox chmod +x /bin/chmod`
### 方法3 使用脚本语言,如python `sudo python -c 'import os;os.chmod("/bin/chmod", 0755)'`
### 方法4 `/lib64/ld-linux-x86-64.so.2 /bin/chmod +x /bin/chmod`
### 方法5 C语言调用chmod函数
### 方法6 `cp /bin/chmod ~/chmod && install -m 755 ~/chmod /bin/`
### 方法7 写个main为空的C程序编译成可执行文件(当然也可以复制一个可执行文件)
```bash
cat /bin/chmod > a.out
mv a.out chmod
./chmod +x /bin/chmod
```
注意文件名必须改为`chmod`

### [出处](http://www.zhihu.com/question/19854702/answer/13161935)
