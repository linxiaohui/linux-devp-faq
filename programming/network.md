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


## 关于send返回值，对方listen，没accept

## 如何获得信息
man tcp
