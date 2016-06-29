#网络编程

## 以访问文件流(stream I/O)的方式进行网络数据传输
以文件流方式访问socket，必须分为读写两个stream，无法通过一个stream同时完成读写功能。   
可以使用以下程序完成操作：（设sockfd是已连接的连接描述符）
```c
FILE *cin, *cout;
cin = fdopen(sockfd, "r");
setbuf(cin, (char * )0);
cout = fdopen(sockfd, "w");
setbuf(cout, (char * )0);
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

# epoll
epoll是在Linux2.6内核中引入I/O多路复用技术。

```c
int epoll_create(int size);
int epoll_ctl(int epfd, int op, int fd, struct epoll_event * event);
int epoll_wait(int epfd,struct epoll_event * events,int maxevents,int timeout);
```

### select的问题
   1. 最大并发数限制，由FD_SETSIZE设置，默认值是1024/2048
   2. 效率: select每次调用都会线性扫描 **全部** 的FD集合
   3. 内核/用户空间 内存拷贝问题

`poll`解决了第一个

### ET和LT模式
   1. LT(level triggered)是默认方式, 同时支持block和no-block socket. 内核通知一个文件描述符fd就绪了，然后可以对这个就绪的fd进行IO操作。如果不作操作， 下次调用epoll时内核还是会继续通知, 传统的select/poll都是这种模型的代表。
   2. ET(edge-triggered)只支持no-block socket。在这种模式下，当描述符从未就绪变为就绪时，内核通过epoll通知。如果一直不对这个fd作操作，再次调用epoll内核不会发送的通知。

ET和LT的区别在于LT事件不会丢弃，而是只要读buffer里面有数据可以让用户读，则不断的通知你。而ET则只在事件发生之时通知,如果要采用ET模式,需要一直read/write直到出错为止。

### [程序实例](https://banu.com/blog/2/how-to-use-epoll-a-complete-example-in-c/epoll-example.c)
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>

#define MAXEVENTS 128

static int make_socket_non_blocking(int sfd)
{
    int flags, s;
    flags = fcntl(sfd, F_GETFL, 0);
    if(flags == -1)
    {
        perror("fcntl getfl");
        return -1;
    }
    flags |= O_NONBLOCK;
    s = fcntl(sfd, F_SETFL, flags);
    if(s == -1)
    {
        perror("fcntl settl");
        return -1;
    }
    return 0;
}

static int create_and_bind(char * port)
{
    struct addrinfo hints;
    struct addrinfo * result, * rp;
    int s, sfd;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;     /*Return IPv4 and IPv6 choices*/
    hints.ai_socktype = SOCK_STREAM; /*We want a TCP socket*/
    hints.ai_flags = AI_PASSIVE;     /*All interfaces*/
    s = getaddrinfo(NULL, port, &hints, &result);
    if(s != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        return -1;
    }
    for(rp = result; rp != NULL; rp = rp->ai_next)
    {
      sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
      if(sfd == -1)
          continue;
      s = bind(sfd, rp->ai_addr, rp->ai_addrlen);
      if(s == 0)
      {
          /*managed to bind successfully!*/
          break;
      }
      close(sfd);
    }
    if(rp == NULL)
    {
        fprintf(stderr, "Could not bind\n");
        return -1;
    }
    freeaddrinfo(result);
    return sfd;
}

int
main(int argc, char * argv[])
{
    int sfd, s;
    int epfd;
    struct epoll_event event;
    struct epoll_event * events;
    if(argc != 2)
    {
        fprintf(stderr, "Usage: %s [port]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    sfd = create_and_bind(argv[1]);
    if(sfd == -1)
      abort();
    s = make_socket_non_blocking(sfd);
    if(s == -1)
        abort();
    s = listen(sfd, SOMAXCONN);
    if(s == -1)
    {
        perror("listen");
        abort();
    }
    epfd = epoll_create1(0);
    if(epfd == -1)
    {
        perror("epoll_create");
        abort();
    }
    event.data.fd = sfd;
    event.events = EPOLLIN | EPOLLET;
    s = epoll_ctl(epfd, EPOLL_CTL_ADD, sfd, &event);
    if(s == -1)
    {
        perror("epoll_ctl");
        abort();
    }
    events = calloc(MAXEVENTS, sizeof event);
    while(1)
    {
        int n, i;
        n = epoll_wait(epfd, events, MAXEVENTS, -1);
        for(i = 0; i < n; i++)
        {
            if((events[i].events & EPOLLERR) ||
               (events[i].events & EPOLLHUP) ||
               (!(events[i].events & EPOLLIN)))
            {
                /*An error has occured on this fd,
                or the socket is not ready for reading*/
                fprintf(stderr, "epoll error\n");
                close(events[i].data.fd);
                continue;
            }
            else if(sfd == events[i].data.fd)
            {
                /*have a notification on the listening socket,
                means one or more incoming connections.*/
                while(1)
                {
                    struct sockaddr in_addr;
                    socklen_t in_len;
                    int infd;
                    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
                    in_len = sizeof in_addr;
                    infd = accept(sfd, &in_addr, &in_len);
                    if(infd == -1)
                    {
                        if((errno == EAGAIN) ||
                           (errno == EWOULDBLOCK))
                        {
                            /*have processed all incoming connections.*/
                            break;
                        }
                        else
                        {
                            perror("accept");
                            break;
                        }
                    }
                    s = getnameinfo(&in_addr, in_len,
                                    hbuf, sizeof hbuf,
                                    sbuf, sizeof sbuf,
                                    NI_NUMERICHOST | NI_NUMERICSERV);
                    if(s == 0)
                    {
                        fprintf(stdout, "Accepted connection on descriptor %d "
                                        "(host=%s, port=%s)\n", infd, hbuf, sbuf);
                    }
                    /*Make the incoming socket non-blocking,
                     add it to the list of fds to monitor.*/
                    s = make_socket_non_blocking(infd);
                    if(s == -1)
                      abort();
                    event.data.fd = infd;
                    event.events = EPOLLIN | EPOLLET;
                    s = epoll_ctl(epfd, EPOLL_CTL_ADD, infd, &event);
                    if(s == -1)
                    {
                        perror("epoll_ctl");
                        abort();
                    }
                } /*End while*/
            } /*End if*/
            else
            {
                /*have data on the fd waiting to be read. Read and display it.
                  must read whatever data is available completely(in ET mode)
                  won't get a notification again for the same data.*/
                int done = 0;
                while(1)
                {
                    ssize_t count;
                    char buf[512];
                    count = read(events[i].data.fd, buf, sizeof buf);
                    if(count == -1)
                    {
                        /*If errno != EAGAIN, that means have read all data.
                        So go back to the main loop.*/
                        if(errno != EAGAIN)
                        {
                            perror("read");
                            done = 1;
                        }
                        break;
                    }
                    else if(count == 0)
                    {
                        /*End of file. The remote has closed the connection.*/
                        done = 1;
                        break;
                    }
                    /*Write the buffer to standard output*/
                    s = write(1, buf, count);
                    if(s == -1)
                    {
                        perror("write");
                        abort();
                    }
                }
                if(done)
                {
                    fprintf(stdout, "Closed connection on descriptor %d\n",
                            events[i].data.fd);
                    /*Closing the descriptor will make epoll
                    remove it from the set of descriptors which are monitored.*/
                    close(events[i].data.fd);
                }
            }
        }/*End for*/
    }
    free(events);
    close(sfd);
    return EXIT_SUCCESS;
}
```
