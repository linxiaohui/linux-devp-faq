# SELinux(Security Enhanced Linux)
Mandatory Access Control（强制访问控制）
在 CentOS 7 中, SELinux 合并进了内核并且默认启用强制Enforcing模式. openSUSE 和 Ubuntu 使用的是 AppArmor.

# 运行模式
Security Enhanced Linux 可以以两种不同模式运行：
   * 强制Enforcing：SELinux 基于 SELinux 策略规则拒绝访问，策略规则是一套控制安全引擎的规则
   * 宽容Permissive：SELinux 不拒绝访问，但如果在强制模式下会被拒绝的操作会被记录下来(/var/log/audit/audit.log, SELinux 日志信息包含了词语`AVC`)
SELinux 也能被禁用。但 **学习如何使用这个工具强过只是忽略它**

# 命令

## `getenforce` 显示 SELinux 的当前模式。
## `setenforce` 更改模式
   * `setenforce  0` 设置为宽容模式
   * `setenforce  1` 强制模式
setenforce 的典型用法之一包括在 SELinux 模式之间切换（从强制到宽容或相反）来定位一个应用是否行为不端或没有像预期一样工作。

## policycoreutils-python包
   * semanage
   * restorecon

# 配置文件
 编辑`/etc/selinux/config` 并设置 `SELINUX` 变量为`enforcing`, `permissive` 或 `disabled`

**注意**
如果`getenforce`为`Disabled`, 无法利用`setenforce`设置运行模式.

# 常见问题
   * 改变一个守护进程监听的默认端口
   * 给一个虚拟主机设置 /var/www/html 以外的文档根路径值
