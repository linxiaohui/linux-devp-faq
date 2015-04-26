#网络状态

## 查找占用端口的进程
* Linux    
`netstat -anp`
* Windows   
`netstat -ano`
* AIX
   1. 根据端口号找到该端口连接对应的PCB/ADDR和连接的协议类型
   `netstat -Aan | grep 389`    
```
    f1000089c27a2358 tcp4 0 0 *.389 *.* LISTEN
```
**注**：PCB ----Protocol Control Block

   2. * 如果是tcp连接，则`rmsock <PCB/ADDR> tcpcb`
      * 如果是udp连接，则`rmsock <PCB/ADDR> inpcb `
   3. `rmsock f100089c27a2358 tcpcb`   
```
The socket 0x702f6800 is being held by proccess 4986 (inetd). #得到id为4986
```

   4. `ps -ef |grep 4986`看到是什么进程

## 网络状态统计
   * 查看TCP状态
      * `netstat -n | awk  '/^tcp/ {++S[$NF]} END { for(a in S) print a,S[a]}'`
      * `netstat -nta| grep '^tcp' |awk '{print $NF}' |sort |uniq -c |sort -nr`
   * 结果中：
       * CLOSED：无连接是活动的或正在进行
       * LISTEN：服务器在等待进入呼叫
       * SYN_RECV：一个连接请求已经到达，等待确认
       * SYN_SENT：应用已经开始，打开一个连接
       * ESTABLISHED：正常数据传输状态
       * FIN_WAIT1：应用说它已经完成
       * FIN_WAIT2：另一边已同意释放
       * ITMED_WAIT：等待所有分组死掉
       * CLOSING：两边同时尝试关闭
       * TIME_WAIT：另一边已初始化一个释放
       * LAST_ACK：等待所有分组死掉

##查看socket状态
   * `cat /proc/net/sockstat`
   * `lsof -i ` 实时查看本机网络服务的活动状态

## TIME_WAIT
内核参数 查看超时设置 `sysctl -a | grep time | grep wait`
```
net.ipv4.netfilter.ip_conntrack_tcp_timeout_time_wait = 120
net.ipv4.netfilter.ip_conntrack_tcp_timeout_close_wait = 60
net.ipv4.netfilter.ip_conntrack_tcp_timeout_fin_wait = 120

#当出现SYN等待队列溢出时，启用cookies来处理，可防范少量SYN攻击，默认为0，表示关闭
net.ipv4.tcp_syncookies = 1 表示开启SYN Cookies

#允许将TIME-WAIT sockets重新用于新的TCP连接，默认为0，表示关闭；
net.ipv4.tcp_tw_reuse = 1 表示开启重用。

net.ipv4.tcp_tw_recycle = 1 表示开启TCP连接中TIME-WAIT sockets的快速回收，默认为0，表示关闭。
net.ipv4.tcp_fin_timeout = 30 表示如果套接字由本端要求关闭，这个参数决定了它保持在FIN-WAIT-2状态的时间。
net.ipv4.tcp_keepalive_time = 1200 表示当keepalive起用的时候TCP发送keepalive消息的频度。
net.ipv4.ip_local_port_range = 1024    65000 用于向外连接的端口范围。

#SYN队列的长度，默认为1024，加大队列长度为8192，可以容纳更多等待连接的网络连接数。 
net.ipv4.tcp_max_syn_backlog = 8192

#系统同时保持TIME_WAIT套接字的最大数量
net.ipv4.tcp_max_tw_buckets = 5000 超过这个数TIME_WAIT套接字将立刻被清除并打印警告信息
```

## SYN_RECV
```
net.ipv4.tcp_synack_retries 
net.ipv4.tcp_syncookies 
net.ipv4.tcp_max_syn_backlog  
```