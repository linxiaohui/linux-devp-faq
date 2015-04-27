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
 