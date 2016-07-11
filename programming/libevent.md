#[libevent](http://monkey.org/~provos/libevent/)

支持将SOCKET, 管道, 信号, 以及定时器统一为通用的逻辑, 给开发人员提供了一个简单高效的异步网络编程库.

创建`libevent`服务器的基本方法是: 注册当发生某一事件(如接受来自客户端的连接)时应该执行的函数, 然后调用`event_dispatch()`, 在应用程序运行时可以在事件队列中添加(注册)或删除(取消注册)事件.

# 示例

## 回显服务器
```c
#include <event.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#define SERVER_PORT 8080
int debug = 0;
struct client {
  int fd;
  struct bufferevent * buf_ev;
};

int setnonblock(int fd)
{
  int flags;

  flags = fcntl(fd, F_GETFL);
  flags |= O_NONBLOCK;
  fcntl(fd, F_SETFL, flags);
}

/*
当客户端套接字有要读的数据时调用它, 写回客户端. 套接字仍然打开, 可以接受新请求
*/
void buf_read_callback(struct bufferevent * incoming, void * arg)
{
  struct evbuffer * evreturn;
  char * req;
  req = evbuffer_readline(incoming->input);
  if (req == NULL)
    return;
  evreturn = evbuffer_new();
  evbuffer_add_printf(evreturn,"You said %s\n",req);
  bufferevent_write_buffer(incoming,evreturn);
  evbuffer_free(evreturn);
  free(req);
}

/*当有要写的数据时调用它*/
void buf_write_callback(struct bufferevent * bev, void * arg)
{
}

/*当出现错误时调用它. 客户端中断连接, 在出现错误的所有场景中, 关闭客户端套接字.
从事件列表中删除客户端套接字的事件条目, 释放客户端结构的内存*/
void buf_error_callback(struct bufferevent * bev, short what, void * arg)
{
  struct client * client = (struct client * )arg;
  bufferevent_free(client->buf_ev);
  close(client->fd);
  free(client);
}

/*
当接受连接时调用此函数. 接受到客户端的连接, 添加客户端套接字信息和一个 bufferevent 结构,
在事件结构中为客户端套接字上的读/写/错误事件添加回调函数; 作为参数传递客户端结构(和嵌入的 eventbuffer 和客户端套接字).
每当对应的客户端套接字包含读、写或错误操作时，调用对应的回调函数.
*/
void accept_callback(int fd, short ev, void * arg)
{
  int client_fd;
  struct sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);
  struct client * client;

  client_fd = accept(fd,
                     (struct sockaddr * )&client_addr,
                     &client_len);
  if (client_fd < 0)
  {
      warn("Client: accept() failed");
      return;
  }
  setnonblock(client_fd);
  client = calloc(1, sizeof(* client));
  if (client == NULL)
    err(1, "malloc failed");
  client->fd = client_fd;

  client->buf_ev = bufferevent_new(client_fd,
                                   buf_read_callback,
                                   buf_write_callback,
                                   buf_error_callback,
                                   client);
  bufferevent_enable(client->buf_ev, EV_READ);
}

int main(int argc, char ** argv)
{
  int socketlisten;
  struct sockaddr_in addresslisten;
  struct event accept_event;
  int reuse = 1;
  event_init();
  socketlisten = socket(AF_INET, SOCK_STREAM, 0);
  if (socketlisten < 0)
  {
      fprintf(stderr,"Failed to create listen socket");
      return 1;
  }
  memset(&addresslisten, 0, sizeof(addresslisten));
  addresslisten.sin_family = AF_INET;
  addresslisten.sin_addr.s_addr = INADDR_ANY;
  addresslisten.sin_port = htons(SERVER_PORT);
  if (bind(socketlisten, (struct sockaddr * )&addresslisten,
           sizeof(addresslisten)) < 0)
  {
      fprintf(stderr,"Failed to bind");
      return 1;
  }
  if (listen(socketlisten, 5) < 0)
  {
      fprintf(stderr,"Failed to listen to socket");
      return 1;
  }
  setsockopt(socketlisten,
             SOL_SOCKET,
             SO_REUSEADDR,
             &reuse,
             sizeof(reuse));
  setnonblock(socketlisten);
  event_set(&accept_event,
            socketlisten,
            EV_READ|EV_PERSIST,
            accept_callback,
            NULL);
  event_add(&accept_event,
            NULL);
  event_dispatch();
  close(socketlisten);
  return 0;
}
```

## HTTP 示例
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     //for getopt, fork
#include <string.h>     //for strcat
#include <sys/queue.h>  //for struct evkeyvalq
#include <event.h>
#include <evhttp.h>     //for http
#include <signal.h>

#define MYHTTPD_SIGNATURE   "myhttpd v 0.0.1"
//处理模块
void httpd_handler(struct evhttp_request * req, void * arg) {
    char output[2048] = "\0";
    char tmp[1024];
    //获取客户端请求的URI(使用evhttp_request_uri或直接req->uri)
    const char * uri;
    uri = evhttp_request_uri(req);
    sprintf(tmp, "uri=%s\n", uri);
    strcat(output, tmp);
    sprintf(tmp, "uri=%s\n", req->uri);
    strcat(output, tmp);
    //decoded uri
    char * decoded_uri;
    decoded_uri = evhttp_decode_uri(uri);
    sprintf(tmp, "decoded_uri=%s\n", decoded_uri);
    strcat(output, tmp);
    //解析URI的参数(即GET方法的参数)
    struct evkeyvalq params;
    evhttp_parse_query(decoded_uri, {U+00B6}ms);
    sprintf(tmp, "q=%s\n", evhttp_find_header({U+00B6}ms, "q"));
    strcat(output, tmp);
    sprintf(tmp, "s=%s\n", evhttp_find_header({U+00B6}ms, "s"));
    strcat(output, tmp);
    free(decoded_uri);
    //获取POST方法的数据
    char * post_data = (char * ) EVBUFFER_DATA(req->input_buffer);
    sprintf(tmp, "post_data=%s\n", post_data);
    strcat(output, tmp);
    /*具体的：可以根据GET/POST的参数执行相应操作，然后将结果输出*/
    /*输出到客户端*/
    //HTTP header
    evhttp_add_header(req->output_headers, "Server", MYHTTPD_SIGNATURE);
    evhttp_add_header(req->output_headers, "Content-Type", "text/plain; charset=UTF-8");
    evhttp_add_header(req->output_headers, "Connection", "close");
    //输出的内容
    struct evbuffer * buf;
    buf = evbuffer_new();
    evbuffer_add_printf(buf, "It works!\n%s\n", output);
    evhttp_send_reply(req, HTTP_OK, "OK", buf);
    evbuffer_free(buf);
}

//当向进程发出SIGTERM/SIGHUP/SIGINT/SIGQUIT的时候，终止event的事件侦听循环
void signal_handler(int sig) {
    switch (sig) {
        case SIGTERM:
        case SIGHUP:
        case SIGQUIT:
        case SIGINT:
            event_loopbreak();  //终止侦听event_dispatch()的事件侦听循环，执行之后的代码
            break;
    }
}
int main(int argc, char * argv[]) {
    //自定义信号处理函数
    signal(SIGHUP, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);
    signal(SIGQUIT, signal_handler);
    //默认参数
    char * httpd_option_listen = "0.0.0.0";
    int httpd_option_port = 8080;
    int httpd_option_daemon = 0;
    int httpd_option_timeout = 120; //in seconds
    //获取参数
    int c;
    while ((c = getopt(argc, argv, "l:p:dt:h")) != -1) {
        switch (c) {
        case 'l' :
            httpd_option_listen = optarg;
            break;
        case 'p' :
            httpd_option_port = atoi(optarg);
            break;
        case 'd' :
            httpd_option_daemon = 1;
            break;
        case 't' :
            httpd_option_timeout = atoi(optarg);
             break;
        case 'h' :
        default :
            exit(EXIT_SUCCESS);
        }
    }
    //判断是否设置了-d，以daemon运行
    if (httpd_option_daemon) {
        pid_t pid;
        pid = fork();
        if (pid < 0) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        }
        if (pid > 0) {
            //生成子进程成功，退出父进程
            exit(EXIT_SUCCESS);
        }
    }
    /*使用libevent创建HTTP Server*/
    //初始化event API
    event_init();
    //创建一个http server
    struct evhttp * httpd;
    httpd = evhttp_start(httpd_option_listen, httpd_option_port);
    evhttp_set_timeout(httpd, httpd_option_timeout);
    //指定generic callback
    evhttp_set_gencb(httpd, httpd_handler, NULL);
    //也可以为特定的URI指定callback
    //evhttp_set_cb(httpd, "/", specific_handler, NULL);
    //循环处理events
    event_dispatch();
    evhttp_free(httpd);
    return 0;
}
```
重要函数:
```c
char * evhttp_encode_uri(const char * uri);
char * evhttp_decode_uri(const char * uri);
const char *evhttp_find_header(const struct evkeyvalq * , const char * );
int evhttp_remove_header(struct evkeyvalq * , const char * );
int evhttp_add_header(struct evkeyvalq * , const char * , const char * );
void evhttp_clear_headers(struct evkeyvalq * );
char * evhttp_htmlescape(const char * html);
```


## 定时器 示例
```c
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <event.h>
#include <evhttp.h>
#define RELOAD_TIMEOUT 5
#define DEFAULT_FILE "sample.html"
char *filedata;
time_t lasttime = 0;
char filename[80];
int counter = 0;

void read_file()
{
  int size = 0;
  char * data;
  struct stat buf;

  stat(filename,&buf);

  if (buf.st_mtime > lasttime)
    {
      if (counter++)
        fprintf(stderr,"Reloading file: %s",filename);
      else
        fprintf(stderr,"Loading file: %s",filename);

      FILE * f = fopen(filename, "rb");
      if (f == NULL)
      {
          fprintf(stderr,"Couldn't open file\n");
          exit(1);
      }
      fseek(f, 0, SEEK_END);
      size = ftell(f);
      fseek(f, 0, SEEK_SET);
      data = (char * )malloc(size+1);
      fread(data, sizeof(char), size, f);
      filedata = (char * )malloc(size+1);
      strcpy(filedata,data);
      fclose(f);
      fprintf(stderr," (%d bytes)\n",size);
      lasttime = buf.st_mtime;
    }
}

void load_file()
{
  struct event * loadfile_event;
  struct timeval tv;
  read_file();
  tv.tv_sec = RELOAD_TIMEOUT;
  tv.tv_usec = 0;
  loadfile_event = malloc(sizeof(struct event));
  evtimer_set(loadfile_event,
              load_file,
              loadfile_event);

  evtimer_add(loadfile_event,
              &tv);
}

void generic_request_handler(struct evhttp_request * req, void * arg)
{
  struct evbuffer * evb = evbuffer_new();
  evbuffer_add_printf(evb, "%s",filedata);
  evhttp_send_reply(req, HTTP_OK, "Client", evb);
  evbuffer_free(evb);
}

int main(int argc, char * argv[])
{
  short           http_port = 8081;
  char          * http_addr = "192.168.1.110";
  struct evhttp * http_server = NULL;

  if (argc > 1)
  {
      strcpy(filename,argv[1]);
      printf("Using %s\n",filename);
  }
  else
  {
      strcpy(filename,DEFAULT_FILE);
  }
  event_init();
  load_file();
  http_server = evhttp_start(http_addr, http_port);
  evhttp_set_gencb(http_server, generic_request_handler, NULL);
  fprintf(stderr, "Server started on port %d\n", http_port);
  event_dispatch();
}
```
# bufferevent
```c
struct bufferevent {
    struct event_base * ev_base;  
    const struct bufferevent_ops * be_ops;  
    struct event ev_read;  
    struct event ev_write;  
    struct evbuffer * input;  
    struct evbuffer * output;                                                                                  
    bufferevent_data_cb readcb;                                                                                                                 bufferevent_data_cb writecb;
    bufferevent_event_cb errorcb;  
};
```
`struct bufferevent`内置了读/写两个event和对应的缓冲区. 当有数据被读入(input)的时候`readcb`被调用; 当output被输出完成的时候, `writecb`被调用;
当网络I/O出现错误, 如链接中断, 超时或其他错误时, `errorcb`被调用

## 使用bufferevent的过程
   1. 设置sock为非阻塞的
```c
evutil_make_socket_nonblocking(fd);
```
   2. 使用`bufferevent_socket_new`创建一个`struct bufferevent * bev`, 关联该sockfd, 托管给event_base
   3. 调用`bufferevent_setcb`设置读,写,错误处理对应的回调函数
   4. 调用`bufferevent_enable`启用读写事件

# 杂项

* 如何使HTTP服务端知道客户端断开   
可以给连接注册关闭回调, 但客户端强制断开连接时, 服务器并没有立即知道.
```c
evhttp_connection_set_closecb(req->evcon, on_close, NULL);
```
因为libevent在收到 HTTP 请求后,就不再监听读事件了,所以就不能通过 read() 返回 0 来知道连接断开,
只能通过 send() 导致 SIGPIPE 才能知道.   
为了让服务器立即知道客户端的断开, 只需要重新监听 EV_READ 事件即可.
```c
struct bufferevent *bev = evhttp_connection_get_bufferevent(req->evcon);
bufferevent_enable(bev, EV_READ);
```

 * evhttp处理POST请求的技巧
 evhttp在evhttp_request接口中包含一个请求类型type，用来表示HTTP的操作(EVHTTP_REQ_GET/EVHTTP_REQ_POST).
 但evhttp接口并没有区分GET和POST操作：
`evhttp_request_uri`: 解析HTTP请求中的url； `evhttp_parse_query`: 解析名值对,得到一个evkeyvalq结构，里面包含了key/value的数组；

但如果这个请求是用POST发送的，传入的数据是保存在消息体中而不是uri中，因此那么 evhttp_parse_query解析后是一个空的结构。

如何在evhttp中处理POST请求:
   1. 访问缓冲区获取POST消息体数据
evhttp_request结构中包含input_buf结构，input_buffer中的buffer就是存放消息体数据的缓冲区, input_buffer有一个offset字段, 用来指明具体的数据长度的.
   2. 模拟GET请求参数
evhttp_request_uri解析出uri,与获取的POST数据组装, 调用evhttp_parse_query解析组装的字符串



#libev
libev是一个高性能的事件循环库

## 示例
```c
#include <ev.h>
#include <stdio.h>
// every watcher type has its own typedef'd struct with the name ev_TYPE
ev_io stdin_watcher;
ev_timer timeout_watcher;
// all watcher callbacks have a similar signature
//this callback is called when data is readable on stdin
static void stdin_cb (EV_P_ ev_io * w, int revents)
{
  puts ("stdin ready");
  //for one-shot events, one must manually stop the watcher with its corresponding stop function.
  ev_io_stop (EV_A_ w);
  //this causes all nested ev_run's to stop iterating
  ev_break (EV_A_ EVBREAK_ALL);
}

// another callback, this time for a time-out
static void timeout_cb (EV_P_ ev_timer * w, int revents)
{
  puts ("timeout");
  //this causes the innermost ev_run to stop iterating
  ev_break (EV_A_ EVBREAK_ONE);
}

int main (void)
{
  //use the default event loop unless you have special needs
  struct ev_loop * loop = EV_DEFAULT;
  // initialise an io watcher, then start it
  // this one will watch for stdin to become readable
  ev_io_init (&stdin_watcher, stdin_cb, /*STDIN_FILENO*/ 0, EV_READ);
  ev_io_start (loop, &stdin_watcher);

  // initialise a timer watcher, then start it
  // simple non-repeating 5.5 second timeout
  ev_timer_init (&timeout_watcher, timeout_cb, 5.5, 0.);
  ev_timer_start (loop, &timeout_watcher);

  // now wait for events to arrive
  ev_run (loop, 0);

  // break was called, so exit
  return 0;
}
```
先创建了一个事件循环，然后注册了两个事件：读取标准输入事件和超时事件。在终端输入或超时后，结束事件循环。

## 事件循环
使用libev的核心是事件循环，可以用 ev_default_loop 或 ev_loop_new 函数创建循环，或者直接使用 EV_DEFAULT宏，区别是 ev_default_loop 创建的事件循环不是线程安全的，而 ev_loop_new 创建的事件循环不能捕捉信号和子进程的观察器。大多数情况下，可以像下面这样使用：
事件循环
使用libev的核心是事件循环，可以用 ev_default_loop 或 ev_loop_new 函数创建循环，或者直接使用 EV_DEFAULT宏，区别是 ev_default_loop 创建的事件循环不是线程安全的，而 ev_loop_new 创建的事件循环不能捕捉信号和子进程的观察器。大多数情况下，可以像下面这样使用：
```c
if (!ev_default_loop (0))
  fatal ("could not initialise libev, bad $LIBEV_FLAGS in environment?");
//或者明确选择一个后端
struct ev_loop * epoller = ev_loop_new (EVBACKEND_EPOLL | EVFLAG_NOENV);
if (!epoller)
  fatal ("no epoll found here, maybe it hides under your chair");
```
如果需要动态分配循环的话，建议使用 ev_loop_new 和 ev_loop_destroy 。

在创建子进程后，且想要使用事件循环时，需要先在子进程中调用 ev_default_fork 或 ev_loop_fork 来重新初始化后端的内核状态，它们分别对应 ev_default_loop 和 ev_loop_new 来使用。

ev_run 启动事件循环。它的第二个参数为0时，将持续运行并处理循环直到没有活动的事件观察器或者调用了ev_break 。另外两个取值是 EVRUN_NOWAIT 和 EVRUN_ONCE 。

ev_break 跳出事件循环（在全部已发生的事件处理完之后）。第二个参数为 EVBREAK_ONE 或 EVBREAK_ALL 来指定跳出最内层的 ev_run 或者全部嵌套的 ev_run 。

ev_suspend 和 ev_resume 用来暂停和重启事件循环，比如在程序挂起的时候。

## 观察器
接下来创建观察器，它主要包括类型、触发条件和回调函数。将它注册到事件循环上，在满足注册的条件时，会触发观察器，调用它的回调函数。

上面的例子中已经包含了IO观察器和计时观察器，此外还有周期观察器、信号观察器、文件状态观察器等等。

初始化和设置观察器使用 ev_init 和 ev_TYPE_set ，也可以直接使用 ev_TYPE_init 。

在特定事件循环上启动观察器使用 ev_TYPE_start 。 ev_TYPE_stop 停止观察器，并且会释放内存。

libev中将观察器分为4种状态：初始化、启动/活动、等待、停止。

libev中的观察器还支持优先级。

不同类型的观察器就不详细解释了，只把官方的一些例子贴在这里吧。

## ev_io

获取标准输入：
```c
static void
stdin_readable_cb (struct ev_loop * loop, ev_io * w, int revents)
{
  ev_io_stop (loop, w);
  .. read from stdin here (or from w->fd) and handle any I/O errors
}

ev_io stdin_readable;
ev_io_init (&stdin_readable, stdin_readable_cb, STDIN_FILENO, EV_READ);
ev_io_start (loop, &stdin_readable);
```
## ev_timer
创建一个60s之后启动的计时器：
```c
static void
one_minute_cb (struct ev_loop * loop, ev_timer * w, int revents)
{
  .. one minute over, w is actually stopped right here
}

ev_timer mytimer;
ev_timer_init (&mytimer, one_minute_cb, 60., 0.);
ev_timer_start (loop, &mytimer);
创建一个10s超时的超时器：

static void
timeout_cb (struct ev_loop * loop, ev_timer * w, int revents)
{
  .. ten seconds without any activity
}

ev_timer mytimer;
ev_timer_init (&mytimer, timeout_cb, 0., 10.); /* note, only repeat used */
ev_timer_again (&mytimer); /* start timer */
ev_run (loop, 0);

// and in some piece of code that gets executed on any "activity":
// reset the timeout to start ticking again at 10 seconds
ev_timer_again (&mytimer);
```

## ev_periodic

创建一个小时为单位的周期定时器：
```c
static void
clock_cb (struct ev_loop * loop, ev_periodic * w, int revents)
{
  ... its now a full hour (UTC, or TAI or whatever your clock follows)
}

ev_periodic hourly_tick;
ev_periodic_init (&hourly_tick, clock_cb, 0., 3600., 0);
ev_periodic_start (loop, &hourly_tick);
或者自定义周期计算方式：

#include <math.h>

static ev_tstamp
my_scheduler_cb (ev_periodic * w, ev_tstamp now)
{
  return now + (3600. - fmod (now, 3600.));
}

ev_periodic_init (&hourly_tick, clock_cb, 0., 0., my_scheduler_cb);
如果想从当前时间开始：

ev_periodic hourly_tick;
ev_periodic_init (&hourly_tick, clock_cb,
                  fmod (ev_now (loop), 3600.), 3600., 0);
ev_periodic_start (loop, &hourly_tick);
```

## ev_signal

在收到 SIGINT 时做些清理：
```c
static void
sigint_cb (struct ev_loop * loop, ev_signal * w, int revents)
{
  ev_break (loop, EVBREAK_ALL);
}

ev_signal signal_watcher;
ev_signal_init (&signal_watcher, sigint_cb, SIGINT);
ev_signal_start (loop, &signal_watcher);
```

## ev_child

fork 一个新进程，给它安装一个child处理器等待进程结束：
```c
ev_child cw;

static void
child_cb (EV_P_ ev_child * w, int revents)
{
  ev_child_stop (EV_A_ w);
  printf ("process %d exited with status %x\n", w->rpid, w->rstatus);
}

pid_t pid = fork ();

if (pid < 0)
  // error
else if (pid == 0)
  {
    // the forked child executes here
    exit (1);
  }
else
  {
    ev_child_init (&cw, child_cb, pid, 0);
    ev_child_start (EV_DEFAULT_ &cw);
  }
```

## ev_stat

监控/etc/passwd是否有变化：
```c
static void
passwd_cb (struct ev_loop * loop, ev_stat * w, int revents)
{
  /*/etc/passwd changed in some way*/
  if (w->attr.st_nlink)
    {
      printf ("passwd current size  %ld\n", (long)w->attr.st_size);
      printf ("passwd current atime %ld\n", (long)w->attr.st_mtime);
      printf ("passwd current mtime %ld\n", (long)w->attr.st_mtime);
    }
  else
    /*you shalt not abuse printf for puts*/
    puts ("wow, /etc/passwd is not there, expect problems. "
          "if this is windows, they already arrived\n");
}

...
ev_stat passwd;

ev_stat_init (&passwd, passwd_cb, "/etc/passwd", 0.);
ev_stat_start (loop, &passwd);
```


#总结
libevent 和 libev 的基本过程是相同的。创建所需的网络监听套接字，注册在执行期间要调用的事件，然后启动主事件循环，让 libev 处理过程的其余部分。

libevent 和 libev 都提供灵活且强大的环境，支持为处理服务器端或客户端请求实现高性能网络（和其他 I/O）接口。目标是以高效（CPU/RAM 使用量低）的方式支持数千甚至数万个连接。在本文中，您看到了一些示例，包括 libevent 中内置的 HTTP 服务，可以使用这些技术支持基于 IBM Cloud、EC2 或 AJAX 的 web 应用程序。
