#网络编程

## 以访问文件流(stream I/O)的方式进行网络数据传输
以文件流方式访问socket，必须分为读写两个stream，无法通过一个stream同时完成读写功能。   
可以使用以下程序完成操作：（设sockfd是已连接的连接描述符）
```c
FILE *cin, *cout;
cin = fdopen(sockfd, "r");
setbuf(cin, (char *)0);
cout = fdopen(sockfd, "w");
setbuf(cout, (char *)0);
```
   * 可以通过fgets, fread, fscanf等函数对文件流cin进行读操作（从socket中读）。
   * 可以通过fputs, fwrite, fprintf等函数对文件流cout进行写操作（写入socket）。
   * 需要注意的是，最好在写操作之后加一句fflush(cout)，使写入的数据尽快发送。
   * 在断开socket连接前，需要先执行fclose(cin); fclose(cout); 再执行close(sockfd)


## 关于send返回值
进程间通过socket通信，一个进程执行`listen`后，即使没有执行`accept`，对方进程`connect`也会成功，
并且`send`也会成功。该进程之后执行`accept`和`read`也都正常。

一个可能的解释是：操作系统内核负责socket数据的发送和接收以及维护TCP连接的状态；
上例中对方进程实际上已经将socket数据发送给了接收进程的操作系统内核。


## 如何获得信息
`man tcp`
