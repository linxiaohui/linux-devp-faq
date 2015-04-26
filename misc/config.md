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
## openSuSE防火墙的关闭
```
sudo /usr/sbin/SuSEfirewall2 stop 
```
永远关闭则
```
chkconfig SuSEfirewall2 off
```
