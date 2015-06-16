#配置

## vmware相关
vmtool安装成功，但是hgfs下没有被挂接共享目录
   1. 使用vmtool 提供的命令`vmware-hgfsclient`查看系统中可以看到的由宿主机共享出的目录
   2. 使用mount命令将宿主机共享出的目录挂接到/mnt/hgfs/下；
```
      mount  -t   vmhgfs         .host:/ASources              /mnt/hgfs/
```
.host:/ASources 其中 .host:/表示的是宿主机；ASources 标识的是宿主机共享出的目录,1中命令查到的共享目录。
   3. 写到/etc/fstab 中进行自动挂接
```
     ./host:/ASources       /mnt/hgfs    vmhgfs     default    0     0
```

## 网络设备的名字
在VmWare中安装openSUSE, 执行`/sbin/ifconfig`查看网络设备显示: 
```
eno167777 Link encap:Ethernet  HWaddr XX:XX:XX:XX:XX:XX  
          inet addr:192.168.106.128  Bcast:192.168.106.255  Mask:255.255.255.0
```
而 `/sbin/ifconfig eno167777` 报错设备不错在.  
事实上, 设备名是 eno16777736 ,`/sbin/ifconfig eno16777736` 查看设备信息.   
具体名字可以查看 `/etc/sysconfig/network`文件夹下的ifcfg-XXXX文件   



## openSuSE防火墙的关闭
```
sudo /usr/sbin/SuSEfirewall2 stop 
```
永远关闭则
```
chkconfig SuSEfirewall2 off
```
