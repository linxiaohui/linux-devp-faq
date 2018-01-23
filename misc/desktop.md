#桌面相关

## GNOME3添加桌面图标
Desktop Entry 文件以".desktop"为后缀名，保存程序.desktop文件的目录有下面两个:
   * `~/.local/share/applications`   用户的应用程序快捷入口
   * `/usr/share/applications`       全局的应用程序快捷入口

除此之外还有几个文件用于存储指定类型文件的默认的打开程序，即文件关联程序，这几个文件分别是
   * `/etc/gnome/defaults.list`
   * `/usr/share/applications/defaults.list`（是前面的文件的link）
   * `/usr/share/applications/mimeinfo.cache`
   * `~/.local/share/applications/mimeapps.list`
   * `/.local/share/applications/mimeinfo.cache`

   前面三个文件保存全局设置，后面两个保存用户设置。

如果要修改 某个类型文件的关联程序，可以通过直接修改这几个文件的方式实现。
   1. 把`/usr/share/applications`下的`.desktop`文件拷贝到桌面目录, 或者新建desktop文件
   2. 用`gnome-tweak-tool`设置，显示桌面图标。

**备注**
   1. desktop文件需要有可执行权限
   2. 设置后若不能打开nautilus，可以先将nautilus进程kill掉

## gedit乱码
缺省配置下，Gedit打开GB18030类型的中文编码文本文件时，将会出现乱码。
出现这种情况的原因是，Gedit 使用一个编码匹配列表，只有在这个列表中的编码才会进行匹配，不在这个列表中的编码将显示为乱码。
需要将 GB18030 加入这个匹配列表。
   * 命令方式
```shell
gsettings set org.gnome.gedit.preferences.encodings \
auto-detected "['GB18030', 'UTF-8', 'CURRENT', 'ISO-8859-15', 'UTF-16']"
```
   * 图形化方式

运行`dconf-editor`
展开`/org/gnome/gedit/preferences/encodings`,  auto-detected的Value中加入 'GB18030' ，加在UTF-8前面；


## Xfce中关闭警报音
安装Xfce后, 在应用程序(如系统设置,vi等)按某方向键,如果到了该方向的末端还继续按该方向键,
或者在系统注销时会发出"嘟"的一声报警音。
   * 关闭
```bash
rmmod pcspkr
```
   * 打开
```bash
modprobe pcspk
```


上面的方法重新启动后beep依旧, 彻底关掉beep
   * bash
   在~/.bashrc的最后添加
```bash
setterm --blength 0
xset -b
```
   * 或者
```bash
sudo echo "blacklist pcspkr" >> /etc/modprobe.d/50-blacklist.conf
```

## 自动登录
   * OpenSUSE 中可以使用yast修改
   * `/etc/sysconfig/displaymanager`
   
**说明**: 在使用YAST修改时使用Linux的 inotify机制(通过使用inotidy-tools中的 inotifywait)定位配置文件
```
inotifywait -r -m -e CLOST_WRITE /etc
```
