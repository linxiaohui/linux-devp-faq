## 如何设置Linux Time Zone
`/usr/share/zoneinfo`存放glibc提供了事先编译好的timezone文件。要查看对于每个time zone当前的时间可以用zdump命令。
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
ntpd在实际同步时间时是一点点的校准过来时间的，最终把时间慢慢的校正对。而ntpdate不会考虑其他程序是否会阵痛，直接调整时间。
```
许多应用程序依赖时间的顺序。而ntpdate调整时间的方式可能使得时间往回调。

## 单调时间函数
一、编写linux下应用程序的时候，有时候会用到高精度相对时间的概念，比如间隔100ms。那么应该使用哪个时间函数更准确呢？
    1、time
        该函数返回的是自1970年以来的秒数，显然精度不够，不能使用
    2、gettimeofday
        该函数返回的是自1970年以来的秒数和微秒数，精度显然是够了。我想有很多程序员也是用的这个函数来计算相对时间的，如果说系统时间因为ntp等原因发生时间跳变，那么用这个函数来计算相对时间是不是就会出问题了。所以说这个函数也不能使用
    3、clock_gettime
        该函数提供了4种类型CLOCK_REALTIME、CLOCK_MONOTONIC、CLOCK_PROCESS_CPUTIMEID、CLOCK_THREAD_CPUTIME_ID。从字面意思可以判断出来，CLOCK_MONOTONIC提供了单调递增的时间戳，该函数返回值为自系统启动后秒数和纳秒数，但是该函数没有考虑ntp的情况，所以并不是绝对意义上的单调递增（见二）。
CLOCK_REALTIME is affected by settime()/settimeofday() calls and can also be frequency corrected by NTP via adjtimex().
CLOCK_MONOTONIC is not affected by settime()/settimeofday(), but is frequency adjusted by NTP via adjtimex().With Linux,NTP normally uses settimeofday() for large corrections (over half a second). The adjtimex() inteface allows for small clock frequency changes (slewing). This can be done in a few different ways, see the man page for adjtimex.
CLOCK_MONOTONIC_RAW that will not be modified at all, and will have a linear correlation with the hardware counters.
    4、syscall(SYS_clock_gettime, CLOCK_MONOTONIC_RAW, &monotonic_time)
        该函数提供了真正意义上的单调递增时间（见三）

二、glibc 中clock_gettime(CLOCK_MONOTONIC)的原理
    查看glibc的代码可以看到这个数值是由内核计算的。

    __vdso_clock_gettime-------->do_monotonic
    这个函数的实现如下：

点击(此处)折叠或打开
notrace static noinline int do_monotonic(struct timespec *ts)
{
        unsigned long seq, ns, secs;
        do {
                seq = read_seqbegin(&gtod->lock);
                secs = gtod->wall_time_sec;
                ns = gtod->wall_time_nsec + vgetns();
                secs += gtod->wall_to_monotonic.tv_sec;
                ns += gtod->wall_to_monotonic.tv_nsec;
        } while (unlikely(read_seqretry(&gtod->lock, seq)));

        /* wall_time_nsec, vgetns(), and wall_to_monotonic.tv_nsec
         * are all guaranteed to be nonnegative.
         */
        while (ns >= NSEC_PER_SEC) {
                ns -= NSEC_PER_SEC;
                ++secs;
        }
        ts->tv_sec = secs;
        ts->tv_nsec = ns;

        return 0;
}
这个代码读取墙上时间，然后加上相对于单调时间的便宜，从而得到单调时间，但是这里并没有考虑ntp通过adjtimex()调整小的时间偏差的情况，所以这个仍然不是绝对的单调递增。
三、内核clock_gettime系统调用
    在kernel/posix-timers.c中内核实现了clock_gettime的系统调用，包括CLOCK_REALTIME、CLOCK_MONOTONIC、CLOCK_MONOTONIC_RAW、CLOCK_REALTIME_COARSE、CLOCK_MONOTONIC_COARSE、CLOCK_BOOTTIME等类型，这里我们看一下CLOCK_MONOTONIC_RAW的实现

点击(此处)折叠或打开
struct k_clock clock_monotonic_raw = {
                .clock_getres = hrtimer_get_res,
                .clock_get = posix_get_monotonic_raw,
        };


posix_timers_register_clock(CLOCK_MONOTONIC_RAW, &clock_monotonic_raw);



/*
 * Get monotonic-raw time for posix timers
 */
static int posix_get_monotonic_raw(clockid_t which_clock, struct timespec *tp)
{
        getrawmonotonic(tp);
        return 0;
}


/**
 * getrawmonotonic - Returns the raw monotonic time in a timespec
 * @ts: pointer to the timespec to be set
 *
 * Returns the raw monotonic time (completely un-modified by ntp)
 */
void getrawmonotonic(struct timespec *ts)
{
        unsigned long seq;
        s64 nsecs;

        do {
                seq = read_seqbegin(&xtime_lock);
                nsecs = timekeeping_get_ns_raw();
                *ts = raw_time;

        } while (read_seqretry(&xtime_lock, seq));

        timespec_add_ns(ts, nsecs);
}
EXPORT_SYMBOL(getrawmonotonic);


static inline s64 timekeeping_get_ns_raw(void)
{
        cycle_t cycle_now, cycle_delta;
        struct clocksource *clock;

        /* read clocksource: */
        clock = timekeeper.clock;
        cycle_now = clock->read(clock);

        /* calculate the delta since the last update_wall_time: */
        cycle_delta = (cycle_now - clock->cycle_last) & clock->mask;

        /* return delta convert to nanoseconds using ntp adjusted mult. */
        return clocksource_cyc2ns(cycle_delta, clock->mult, clock->shift);
}

四、关于wall time和monotonic time
    wall time：xtime，取决于用于对xtime计时的clocksource，它的精度甚至可以达到纳秒级别，内核大部分时间都是使用xtime来获得当前时间信息，xtime记录的是自1970年当前时刻所经历的纳秒数。
    monotonic time: 该时间自系统开机后就一直单调地增加（ntp adjtimex会影响其单调性），它不像xtime可以因用户的调整时间而产生跳变，不过该时间不计算系统休眠的时间，也就是说，系统休眠时（total_sleep_time)，monotoic时间不会递增。
    raw monotonic time: 该时间与monotonic时间类似，也是单调递增的时间，唯一的不同是，raw monotonic time不会受到NTP时间调整的影响，它代表着系统独立时钟硬件对时间的统计。
    boot time:  与monotonic时间相同，不过会累加上系统休眠的时间(total_sleep_time)，它代表着系统上电后的总时间。
五、总结
    在linux下获取高精度单调递增的时间，只能使用syscall(SYS_clock_gettime, CLOCK_MONOTONIC_RAW, &monotonic_time)获取！
