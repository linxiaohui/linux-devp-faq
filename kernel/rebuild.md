# 重新编译内核

## 清除临时文件、中间文件和配置文件。
   * (1) make clean
         remove most generated  files but keep the config
   * (2) make mrproper
          remove all generated  files + config files
   * (3) make distclean
          mrproper + remove editor backup and patch files

## 确定系统的软硬件配置
   * CPU的类型
   * 网卡的型号
   * 所需支持的网络协议
   * ...

## 使用如下命令之一配置内核
   * make config
          基于文本模式的交互式配置, 配置时系统会逐个询问选择`Y or N`直到配置完成
   * make menuconfig
         基于文本模式的菜单型配置
   * make oldconfig
         使用已有的配置文件(.config), 但是会询问新增的配置选项
   * make xconfig
         图形化的配置, 依赖Qt
   * make gconfig
         图形化的配置, 依赖GTK

用 make menuconfig 配置时的一些注意事项：
```
<*>,< >,<M>用空格键切换, 或直接分别对应输入y, n, m
<*>: 选项编译和链接，编译进内核
<M>: 选项编译不链接，编译成模块
< >: 选项不编译不链接
```

由于配置内核的选项太多, 一般的配置方法是对模板进行修改, 模板在arch目录下, 例如将`/usr/src/linux/arch/x86/configs/`
中`x86_64_defconfig`复制到内核根目录下并改名成".config", 
然后再用`make menuconfig`就默认用".config"文件配置, 然后增删配置即可.

linux主机的config文件在/boot下, 如`config-4.1.12-1-default`
可以将其复制到内核源代码目录下并更名成`.config`.

## 编译内核
```
make
```

**备注**

2.6之前的内核需要分别编译内核和模块

   * make zImage/make bzImage
     二者的区别在于: 在X86平台下, zImage只能用于小于512k的内核. 如果大于512k的内核, 用make zImage编译就会报错
     如果需要获取详细的编译信息，可以使用
     make zImage  V=1
     make bzImage V=1
   * make  modules
     编译内核模块
   
编译好的内核位于 ${KERNELSRC}/arch/_cpu_/boot/, 

## 安装内核模块
```
make  modules_install
```

该命令会将编译好的内核模块从内核源代码根目录复制到/lib/modules/下, 生成一个以版本号命名的目录

## 安装内核
```
make install
```

安装内核主要工作是将编译生成的内核复制到/boot目录中, 生成initramfs并放在/boot目录中   
修改启动引导器(如GRUB2)配置文件.


# 参考资料
   1. Linux Kernel Hacks
