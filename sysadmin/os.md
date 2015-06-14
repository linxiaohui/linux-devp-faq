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

## grub重装
   * grub2-install /dev/sda
   * 修改/etc/grub.d/下的配置文件
   * grub2-mkconfig > /boot/grub2/grub.cfg

## sparse file not allowed
   * / 分区为btrfs
   * 删除/boot/grub2/grubenv
   * grub2-editenv create

