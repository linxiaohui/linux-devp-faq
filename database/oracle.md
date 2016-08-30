# 安装Oracle

## 环境与版本
   * CentOS 7 x86_64
   * Oracle 12c(linuxamd64_12102_database)

## 解决乱码问题
默认情况下, `LANG=zh_CN.UTF-8`是直接运行`runInstaller`界面为乱码(方框), 解决该问题可以`export LANG=C`修改为英文界面;
或者:
   * 建立目录`fallback`, 将字体(如wqy.ttf)复制到`fallback`中
   * 打开`database/stage/Components/oracle.jdk/1.6.0.75.0/1/DataFiles/filegroup2.jar`
   * 将`fallback/wqy.ttf`加入到 **其** `jdk/jre/lib/fonts/`中(目录结构为`jdk/jre/lib/fonts/fallback/wqy.ttf`)

## 修改Linux系统配置
   1. 修改 `/etc/security/limits.conf`
```
oracle           soft    nproc   2047
oracle           hard    nproc   16384
oracle           soft    nofile  1024
oracle           hard    nofile  65536
```
若不修改, 安装Oracle时检查通不过. 修改不需要重启.

   2. `/etc/profile`
```bash
if [ $USER = "oracle" ]; then
        if [ $SHELL = "/bin/ksh" ]; then
              ulimit -p 16384
              ulimit -n 65536
        else
              ulimit -u 16384 -n 65536
        fi
fi
```
