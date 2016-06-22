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

## GRUB2 引导丢失
### 挂载原系统的各分区
   1. 首先查看各分区的情况`fdisk -l`
   2. 然后将各个分区挂载。例如，sda6 为 /boot 分区，sda7 为 swap 分区，sda8 为 / 分区，sda9 为 /home 分区
```bash
mount /dev/sda8 /mnt
mount /dev/sda6 /mnt/boot
mount /dev/sda9 /mnt/home
mount -t proc proc /mnt/proc
mount --rbind /sys /mnt/sys
mount --rbind /dev /mnt/dev
```
   3. chroot到需要修复的系统
```bash
chroot /mnt /bin/bash
```
#### 重装 GRUB2
   1. 首先生成 /boot/grub2/grub.cfg：
```bash
grub2-mkconfig -o /boot/grub2/grub.cfg
```
   2. 将GRUB2 安装到 sda：
```bash
grub2-install /dev/sda
```

## sparse file not allowed
   * / 分区为btrfs
   * 删除/boot/grub2/grubenv
   * grub2-editenv create
