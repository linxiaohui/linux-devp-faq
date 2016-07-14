# 关于MTU
MTU(Maximum Transmission Unit), 网络上传送的最大数据包, 单位字节. MTU设置不当, 或者网络中的设备MTU设置不当可能会导致许多网络问题.
例如, 如果数据包过大而又设置了不分片标志, 路由器会直接丢弃该包, 从而导致网络问题.

# 查看MTU
   1. `netstat -i`
   2. `cat /sys/class/net/eth0/mtu`

# 设置MTU
   1. `ifconfig eth0 mtu 1500`
   2. `echo "1500" > /sys/class/net/eth0/mtu`

# PING
   * **Windows**: `ping -f -l 1472 192.168.1.1`
   * **Linux**  : `ping -c 1 -M do -s 1472 192.168.1.1`

1472是数据包的大小, 包头部分占28字节, 因此MTU值为 1472+28=1500
