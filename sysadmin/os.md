## 查看操作系统版本
   * cat /proc/version 
   * more /etc/SuSE-release
   * lsb_release -a
   * uname -a

## 忘记Linux密码后的解决方法
在Grub的界面将启动参数的kernel项后增加一参数  init=/bin/bash
启动后将得到一个shell，此时，根分区的挂载模式默认是只读，要将其改为可写，否则不能更改root密码:
```bash
mount -no remount,rw /
passwd root
sync
mount -no remount,ro /
reboot
```

