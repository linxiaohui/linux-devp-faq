# 系统管理

## 找回LINUX"丢失"的磁盘空间
   * 磁盘文件删除后，但是原先对应写文件的进程还在，仍然抓着那个文件，在往里面写数据，空间继续被其耗用，同时删除后的空间得不到释放，du 命令也看不到这个文件占用的空间。
   * 使用mount命令，挂载到一个原本有数据的目录，之前目录下的文件不可见也不会被du看到统计

`lsof | grep deleted`查找被删除的文件

## 恢复删除的文件
   1. 找出打开文件的进程和文件描述符
   `lsof | grep _filename_`(此时lsof _filename_已经无法获取信息)
   2. 恢复文件
   `cat /proc/_pid_/fd/_fd_ > _filename_ `

## 查看引起`device busy`的进程
   `fuser /path_to_dir`

