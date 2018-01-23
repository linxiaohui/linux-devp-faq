#配置

## vmware相关


### 安装vmtools时报错
   * 报错`implicit declaration of vfs_readlink`
   * 报错`implicit declaration of generic_file_aio_write`
   * VMWare Workstation 10.0.0 build-1295980 && VMwareTools-9.6.0-1294478.tar.gz
根据[vmware-tools-patches](https://github.com/rasa/vmware-tools-patches), 下载最新版本的VMWare Tools安装.

### vmtool安装成功，但是hgfs下没有被挂接共享目录
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

### 虚拟机无法获取IP
宿主vmware-dhcp服务终止导致虚拟机无法获取IP

### 安装vmware-tools 失败
报错
```
A previous installation of VMware software has been detected.
The previous installation was made by the tar installer (version 3).
Keeping the tar3 installer database format.
Error: Unable to find the binary installation directory (answer BINDIR)
in the installer database file “/etc/vmware-tools/locations”.
Failure
Execution aborted.
```

解决方法：
      1. 删除etc/vmware-tools目录 `rm -rf /etc/vmware-tools`
      2. 删除/tmp/vm* `rm -rf /tmp/vm*`
      3. 重新安装

## Virtual Box

### 共享
在设置的"共享文件夹中"设置宿主目录和共享名字(如C_DRIVE),   
使用命令 `mount -t vboxsf C_DRIVE /mnt/c/` 挂载,  
卸载使用命令 `umount -f /mnt/c`

### 网络
虚拟机里需要配置两块网卡:
   * NAT
   * Host-Only


## HOSTNAME有误
因`/etc/hosts`有误, 系统hostname不为`/etc/hostname`中配置的名字而是`bogon`.   
同时导致vim报错, 类似于
```
_IceTransSocketUNIXConnect: Cannot connect to non-local host
Could not open network socket.
```
当GTK程序启动很慢而且出现这种类似的提示, 一般来说是因为HOSTNAME有问题


## 网络设备的名字
在VmWare中安装openSUSE, 执行`/sbin/ifconfig`查看网络设备显示:
```
eno167777 Link encap:Ethernet  HWaddr XX:XX:XX:XX:XX:XX  
          inet addr:192.168.106.128  Bcast:192.168.106.255  Mask:255.255.255.0
```
而 `/sbin/ifconfig eno167777` 报错设备不错在.  
事实上, 设备名是 eno16777736 ,`/sbin/ifconfig eno16777736` 查看设备信息.   
具体名字可以查看 `/etc/sysconfig/network`文件夹下的ifcfg-XXXX文件   


## OpenSuSE防火墙的关闭
```
sudo /usr/sbin/SuSEfirewall2 stop
```
永远关闭则
```
chkconfig SuSEfirewall2 off
```

## 配置防火墙
   * 配置文件
```
/etc/sysconfig/SuSEfirewall2
```
   * 允许访问特定端口
      * TCP端口
         `FW_SERVICES_EXT_TCP = "6000"`
      * UDP端口
         `FW_SERVICES_EXT_UDP = "177"`
   * 只允许特定IP访问指定端口
      * `FW_SERVICES_ACCEPT_EXT`参数
      * 允许单个地址
         `FW_SERVICES_ACCEPT_EXT="192.168.1.103,tcp,80"`
      * 允许多个地址
         `FW_SERVICES_ACCEPT_EXT="192.168.2.103 192.168.1.103,tcp,80"`
      * 允许一个网段
         `FW_SERVICES_ACCEPT_EXT="192.168.2.0/24,tcp,80"`
      * 注意相应的端口不应该在`FW_SERVICES_EXT_TCP`或`FW_SERVICES_EXT_UDP`中
   * 修改后重启防火墙
```
sudo /usr/sbin/SuSEfirewall2 stop
sudo /usr/sbin/SuSEfirewall2 start
```

## OpenSuSE中服务的启停
   * `service sshd stop`
   * `service sshd start`

## 启动时报错piix4_smbus: Host SMBus controller not enabled的解决方法
` lsmod | grep piix4 `   
`echo "blacklist i2c_piix4 " | sudo tee -a /etc/modprobe.d/50-blacklist.conf `

## CentOS 关闭防火墙
### CentOS 6
   * `chkconfig iptables on/off`
   * `service iptables start/stop`

### CentOS 7
   * `systemctl start/stop firewalld.service`
   * `systemctl disable/enable firewalld.service`

## 连接SMB
  * `smbclient  -L 192.168.1.100`
  * `sudo mount -t cifs //192.168.1.100/移动磁盘-C /mnt/`

