#常用第三方库

## ZeroMQ

## libevent

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

## libev

## Axis2C

## gSoap

## libxml2

## OpenSSL

## libghttp
代码示例
```c
ghttp_request *request = NULL;
/* 申请HTTP request对象:用于设置HTTP请求的属性以及获取返回的Response*/
request = ghttp_request_new();
/* 使用HTTP request对象之前必须设置适当的参数 */
/* 设置URI */
ghttp_set_uri(request, "http://localhost:8080/index.html");
/* 
设置HTTP请求的header  ghttp_set_header(), 
设置代理服务器        ghttp_set_proxy(), 
设置HTTP请求的body    ghttp_set_body(). 
*/
ghttp_set_header(request, http_hdr_Connection, "close");

/* 设置好HTTP request后必须调用 ghttp_prepare()进行域名解析等准备工作 */
ghttp_prepare(request);

/* 进行HTTP请求 */
ghttp_process(request);

/*
ghttp_get_header()      查看HTTP Response头
ghttp_get_body()        指向HTTP Response的Body
http_get_body_len()     HTTP Response的Body的长度
ghttp_status_code()     HTTP响应的状态码
ghttp_reason_phrase()   HTTP响应的reason
*/

/* 输出HTTP响应的Body. 注意不能将Body作为\0结尾字符串 */
fwrite(ghttp_get_body(request), ghttp_get_body_len(request), 1, stdout);

/*
在完成一次HTTP请求之后, 若要继续访问同一服务器的其它资源, 可以使用同一ghttp_request
调用 ghttp_set_uri() 设置新的资源URI,
调用 ghttp_prepare() 准备,
调用 ghttp_clean() 清理原缓存等
调用 ghttp_process() 进行HTTP请求 
这种方式可以充分利用已有的资源, 提高效率
*/

/* 释放资源 */
ghttp_request_destroy(request);

```

## OpenCC
Open Chinese Convert（OpenCC）是一个开源的中文简繁转换项目.  
它包括一个可执行程序opencc以及一套库. [文档](http://byvoid.github.io/OpenCC/1.0.2/group__opencc__c__api.html).  
opencc的用法:  
   * 查看帮助 `opencc --help`   
   * 简体转繁体 `opencc -i <filename> -c zhs2zht.ini`
   * 繁体转简体 `opencc -i <filename> -c zht2zhs.ini`

