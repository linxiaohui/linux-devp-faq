#进程检查

## 如何检查一个进程打开文件数量
```bash
ls /proc/_pid_/fd | xargs -i echo {} | wc -l
```
或许更合适的方式是使用 lsof
```bash
lsof -p _pid_ | awk 'BEGIN {n=0} $4~/[0-9].*/ {n=n+1} END {print n}'
```

## Linux每进程线程数问题
每进程可用线程数的上限 = VIRT上限/stack size
   * 其中 VIRT 上限:
      * 32位x86 = 3G
      * 64位x64=64G
   * statck size 默认是 10240 
      * 默认情况下32位系统上单进程最多可以创建300个线程，
      * 64系统在内存充足的情况下最多可以创建 6400 个线程。
   * 在机器硬件固定的情况下，可以通过 `ulimit -s`降低stack size 的设置值来获得更多的每进程线程数。
   * 使用 `ps -Lf pid `查看对应进程下的线程数


## 关于/proc文件系统
`man 5 proc`   
重点关注 `/proc/_pid_/status`, `/proc/_pid_/maps`
