# 重新编译内核

## 清除临时文件、中间文件和配置文件。
(1) make clean
         remove most generated  files but keep the config
(2) make mrproper
          remove all generated  files + config files
(3) make distclean
          mrproper + remove editor backup and patch files
## 确定目标系统的软硬件配置情况，比如CPU的类型、网卡的型号、所需支持的网络协议等。

## 使用如下命令之一配置内核。
(1) make config
          基于文本模式的交互式配置，配置时系统会逐个询问你选择Y or N ，直到配置完成。
(2) make menuconfig
         基于文本模式的菜单型配置。
(3) make oldconfig
         使用已有的配置文件(.config)，但是会询问新增的配置选项。
(4) make xconfig或make gconfig
         图形化的配置(需要安装图形化的系统)，前者是针对以qt为图形界面基础功能的图形化界面显示如kde ，后者是针对以gtk为图形界面基础功能的图形化界面显示如gnome。
 用 make menuconfig 配置时的一些注意事项：
```
<*>,< >,<M>用空格键切换，或直接分别对应输入y，n，m。
<*>：选项编译和链接，编译进内核
<M>：选项编译不链接，编译成模块
< >:选项不编译不链接
```
由于配置内核的选项太多，我们一般的配置方法是对模板进行修改，模板在arch目录下,以arm的s3c2410为例，则进入目录(本文以linux-2.6.29为例)：

 ../linux-2.6.29/arch/arm/configs中找到"s3c2410_defconfig"文件将它复制到内核根目录下，并改名成".config"即可，然后再用make  menuconfig就默认用".config"文件配置，然后增删配置即可。linux主机的config文件在/boot下，如："config-2.6.18-238.el5"文件就是其中一个，本文就是将这个文件改名成.config后复制至内核源码根目录下，进行编译的(本文的实验对该配置未作改动，直接保存后进行下一步动作)。

4. 编译内核

(1) make zImage

(2) make bzImage

二者的区别在于：在X86平台下，zImage只能用于小于512k的内核，如果大于512k的内核，用make zImage编译就会报错。

如果需要获取详细的编译信息，可以使用：

(1) make zImage  V=1

(2) make bzImage  V=1

编译好的内核位于../linux-2.6.29/arch/<cpu>/boot/目录下，例如用make bzImage编译x86时会在目录../linux-2.6.29/arch/x86/boot/下产生"bzImage"文件。

5.编译内核模块

make  modules

6.安装内核模块

make  modules_install

**该命令会将编译好的内核模块从内核源代码根目录(即../linux-2.6.29/)copy到/lib/modules/下，会在/lib/modules/下生成一个2.6.29/的目录**

7. 制作 init ramdisk

mkinitrd  initrd-$version  $version
例：mkinitrd  initrd-2.6.29  2.6.29
      $version 可以通过查询/lib/modules/下的目录得到,看上一步的命令是否有在/lib/modules/下产生2.6.29/目录。该命令执行后会在内核根目录下生成"initrd-2.6.29"文件。
      init ramdisk的作用：提供一种让内核可以简单使用ramdisk的能力。这些能力包括：格式化一个ramdisk；加载文件系统内容到ramdisk；将ramdisk作为根文件系统。

8.内核安装(X86平台)
       由于Linux系统启动时，会从/boot目录下来寻找内核文件与init ramdisk，所以需要将内核文件和init ramdisk拷贝到/boot目录。
(1) cp   ./arch/x86/boot/bzImage    /boot/vmlinuz-$version
例：cp ./arch/x86/boot/bzImage    /boot/vmlinuz-2.6.29
注：vmlinuz-2.6.29可以任意取名，这里只是取比较有意义的名字而已。
(2) cp   $initrd    /boot/
      $initrd指命令mkinitrd  initrd-$version  $version执行后在内核根目录下生成的"initrd-2.6.29"文件，即init ramdisk文件。
例：cp   initrd-2.6.29    /boot/
(3) 修改/etc/grub.conf或者/etc/lilo.conf
将代码：
```
title CentOS-base (2.6.18-238.el5)
        root (hd0,0)
        kernel /vmlinuz-2.6.18-238.el5 ro root=LABEL=/ rhgb quiet
        initrd /initrd-2.6.18-238.el5.img
```
复制并且改成：
```
title chen (2.6.29)
        root (hd0,0)
        kernel /vmlinuz-2.6.29 ro root=LABEL=/ rhgb quiet
        initrd /initrd-2.6.29
```
其只是根据本文的具体情况，对kernel的vmlinuz-2.6.18-238.el5 改成vmlinuz-2.6.29 ，将initrd的initrd-2.6.18-238.el5.img改成initrd-2.6.29


**$version为所编译的内核版本号，本文采用2.6.29(即$version=2.6.29)**
本文的命令均在内核源代码根目录(../linux-2.6.29/)下执行。
