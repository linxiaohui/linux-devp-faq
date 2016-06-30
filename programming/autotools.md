#autotools

## 简介
autoctools是一个用于生成可以自动地配置软件源代码包以适应多种Unix类系统的shell脚本的工具。

### m4
`m4`的名称取自Macro(M后面跟4个字母)，用来处理文本替换: `m4`是bash里的预处理器。`autoconf`基于`m4`这个工具来生成`configure`脚本

## 步骤
1.在主目录下运行autoscan
autoscan (autoconf): 扫描源代码以搜寻普通的可移植性问题，比如检查编译器，库，头文件等，生成文件configure.scan
     这个操作的主要做用是扫描目录及其子目录生成通用的configure.scan，以及autoscan.log。
     configure.scan主要包含（参考Autotools上手指南）
     * 1). 产生configure脚本的相关宏
     * 2). 检测程序相关选项的宏
     * 3). 检测库的宏
     * 4). 检测头文件的宏
     * 5). 检测  typedefs, structures, and compiler characteristics 的宏
     * 6). 检测库函数的宏
2.修改configure.scan
    加入用户自定义的一些宏或是修改一些宏，将其重命名为configure.ac。
    加入AM_INIT_AUTOMAKE来初始化automake，修改AC_CONFIG_FILES（[Makefile src/Makefile]).
```
增加
AC_INIT([_packname_], [_version_], [_email_])设置包的名字，版本和作者信息
AMINIT_AUTOMAKE需要生成automake文件
在执行configure.in之前，需要引入libtools，在configure.in里增加
AC_PROG_LIBTOOL
检查库文件
PKG_CHECK_MODULES([LIBEVENT],[libevent]) 使用pkg-config来检测是否存在libevent库，并保存在LIBEVENT相关参数保存在Makefile里，这些是可以用pkg-config命令获取的
(Makefile中)
LIBEVENT_LIBS = -levent
LIBEVENT_CFLAGS =
如果库文件没有pkg-config接口，可以用下列的方式指定库函数的位置
AC_ARG_WITH(crypto, [AS_HELP_STRING([--with-crypto],[with crypto library])], [WITH_CRYPTO="$withval"], [WITH_CRYPTO="-lcrypto"])
这样如果在configure时带了参数--with-crypto xxx, 则WITH_CRYPTO参数就设定为xxx，否则为-lcrypto，这个参数可以在Makefile.am里使用，并反馈到Makefile里
```

3.运行autoheader,生成config.h.in
autoheader(autoconf): 根据configure.ac中的某些宏，比如cpp宏定义，运行m4，生成config.h.in

        config.h.in根据configure.ac的头文件配置生成相应的选项
4.编写Makefile.am
需要在根目录和所有源码目录下添加Makefile.am. Makefile.am相当于是Makefile的配置文件.automake通过其生成`Makefile.in`文件，
`configure`命令将Makefile.in里的`@xxx@`替换成系统侦测到的参数生成Makefile
```
bin_PROGRAMS=xxx 指定需要生成什么可执行文件
xxx_SOURCES=xxx 生成xxx，依赖哪些源文件
INCLUDES= 指定头文件的搜索路径
LIBS= 指定额外链接的LIBS
AM_CFLAGS= 指定编译时使用的参数(不使用CFLAGS的原因是CFLAGS应该在configure.in里指定)
也可以针对单个可编译目标设定参数
xxx_LDFLAGS link时指定的参数，比如-static
xxx_LDADD link时需要额外链接的LIBS
打包库文件的参数:
lib_LTLIBRARIES=libxxx.la
libxxx_la_SOURCES=$(wildcard *.c)
libxxx_la_LIBADD=
libxxx_la_LDFLAGS=
如果有目录下有子目录，只需要在当前目录下的Makefile.am里增加下列参数即可
SUBDIRS=subdir1 subdir2
如果可执行文件或者库不想被make install安装
noinst_PROGRAMS=
noinst_LTLIBRARIES=
对于非可执行文件和库，如果也想被安装，则可以用下列参数来指定
xxx_DATAS=$(wildcard *.html)
xxxdir=@prefix@/documentroot/
```

5.执行aclocal
生成需要用到的M4处理器
aclocal (automake):根据已经安装的宏，用户定义宏和acinclude.m4文件中的宏将configure.ac文件所需要的宏集中定义到文件 aclocal.m4中

6.运行autoconf

autoconf:将configure.ac中的宏展开，生成configure脚本。这个过程可能要用到aclocal.m4中定义的宏。

从configure.in生成configure
打包库的时候，需要在执行configure.in之前，需要引入libtools，在configure.in里增加
AC_PROG_LIBTOOL
运行autoconf的过程是一个宏替换的过程，autoconf调用M4处理器来对configure中用的宏进行替换。

7.运行automake,生成Makefile.in
automake: automake将Makefile.am中定义的结构建立Makefile.in，然后configure脚本将生成的Makefile.in文件转换为Makefile。如果在configure.ac中定义了一些特殊的宏，比如AC_PROG_LIBTOOL，它会调用libtoolize，否则它会自己产生config.guess和config.sub

8.新建gnu的标准文件 AUTHORS   COPYING  NEWS ChangeLog  README.

9.运行./configure生成最终的Makefile

10.make编译
make dist

## 总结
