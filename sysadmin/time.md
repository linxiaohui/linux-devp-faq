## 如何设置Linux Time Zone
`/usr/share/zoneinfo`存放glibc提供了事先编译好的timezone文件

要查看对于每个time zone当前的时间可以用`zdump`命令

### 方法1. 修改/etc/localtime
该文件定义了所在time zone, 可以将其link到`/usr/share/zoneinfo`中某个文件。 可使用`date`命令验证。使用`strace date`验证使用了该文件
### 方法2. 设置TZ环境变量的值
TZ的值可以有多种格式,最简单的设置方法就是使用`tzselect`命令

TZ变量的值会覆盖/etc/localtime设置。也就是说当TZ变量没有定义的时候系统才使用/etc/localtime来确定time zone.

## Real Time Clock(RTC) and System Clock
Linux系统中有两个时钟: 硬件时间时钟(RTC)和系统时钟(System Clock)。
硬件时钟是指嵌在主板上的特殊的电路, 系统时钟就是操作系统的kernel所用来计算时间的时钟。参见`hwclock`命令。


## NTP时间同步
### 架设NTP Relay Server
rpm -ivh ntp-4.2.2p1-5.el5.rpm
### 互联网提供同步服务的NTP Server [NTP的官方网站](http://www.pool.ntp.org)
### 与NTP Server同步 `ntpdate 0.pool.ntp.org`
### 配置NTP `/etc/ntp.conf`, 需要加入上面的NTP Server和一个driftfile就可以了
```
server 0.uk.pool.ntp.org
server 1.uk.pool.ntp.org
driftfile /var/lib/ntp/ntp.drift
```
### 启动NTP Server `/etc/init.d/ntpd start`
### 查看NTP服务的运行状况 `watch ntpq -p`
### `ntpdate`一般用于ntp的客户端使用
### 可以在/var/adm/messages中看到ntp的同步信息的情况

## ntpd与ntpdate的区别
```
ntpd在实际同步时间时是一点点的校准过来时间的，最终把时间慢慢的校正对。
而ntpdate不会考虑其他程序是否会阵痛，直接调整时间。
```
许多应用程序依赖时间的顺序。而ntpdate调整时间的方式可能使得时间往回调。

## 单调时间函数
一、编写linux下应用程序的时候，有时候会用到高精度相对时间的概念，比如间隔100ms。那么应该使用哪个时间函数更准确呢？
   1. time
        该函数返回的是自1970年以来的秒数
   2. gettimeofday
        该函数返回的是自1970年以来的秒数和微秒数
   3. clock_gettime
        该函数提供了4种类型
           * CLOCK_REALTIME
           * CLOCK_MONOTONIC
           * CLOCK_PROCESS_CPUTIMEID
           * CLOCK_THREAD_CPUTIME_ID
        从字面意思可以判断出来，CLOCK_MONOTONIC提供了单调递增的时间戳，该函数返回值为自系统启动后秒数和纳秒数，
        但是该函数没有考虑ntp的情况，所以并不是绝对意义上的单调递增
```
CLOCK_REALTIME is affected by settime()/settimeofday() calls and can also be frequency corrected by NTP via adjtimex().

CLOCK_MONOTONIC is not affected by settime()/settimeofday(), but is frequency adjusted by NTP via adjtimex().
With Linux,NTP normally uses settimeofday() for large corrections (over half a second).
The adjtimex() inteface allows for small clock frequency changes (slewing).
This can be done in a few different ways, see the man page for adjtimex.

CLOCK_MONOTONIC_RAW that will not be modified at all, and will have a linear correlation with the hardware counters.
```

   4. syscall(SYS_clock_gettime, CLOCK_MONOTONIC_RAW, &monotonic_time)
        该函数提供的单调递增时间
