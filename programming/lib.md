#常用第三方库

## ZeroMQ

## gSoap
   * gsoap_2.8.17.zip
   * `./configure && make `
   * 若需要SSL但系统中没有安装zlib.h和OpenSSL
```bash
export C_INCLUDE_PATH=$HOME/zlib-1.2.8:$HOME/openssl-1.0.1g/include
export CPLUS_INCLUDE_PATH=$HOME/zlib-1.2.8:$HOME/openssl-1.0.1g/include
export LIBRARY_PATH="$HOME/openssl-1.0.1g/:$HOME/zlib-1.2.8"
export LIBS=-ldl
```
   * 原因  
通过分析编译过程和查看stdsoap2.cpp, 对soap_ssl_init函数的定义需要WITH_OPENSSL和HAVE_OPENSSL_SSL_H
否则生成的.a文件中没有这个函数的定义.  
而示例程序默认是需要SSL的   
在/configure生成config.h时，若OpenSSL路径不准确则不会定义相关的宏  
可以修改 config.h
```c
#define HAVE_OPENSSL_SSL_H 1
```

   * 报错`undefined reference to 'soap_ssl_init'`
原因与上面描述类似.   
      * 安装OpenSSL
      * -DWITH_OPENSSL.
      * 编译时链接 stdsoap2.cpp

* 若不需要SSL
```bash
./configure --disable-ssl && make
```

   * 若需要32位的库
```bash
export CFLAGS=-m32
export CXXFLAGS=-m32
```

## Axis2C
gSoap通过工具从WebService的wsdl将每一个服务生成一个函数和其输入输出结构;
调用某个服务的时候, 填充其输入然后调用相应的函数, 获取输出.

有一类应用提供的不同的服务的WebService接口一样的, 不同的服务需要的参数放在同样的字段中;
这种场景下使用gSoap的话会根据提供的服务产生大量结构相同输入输出接口; 为了抽象WebService调用过程, 可以考虑使用Axis2C.
Axis2C时Apache Axis2的C语言实现.

   * 编译
```bash
cd axis2c-src-1.6.0
export LIBXML2_CFLAGS="-I $HOME/libxml2-2.8/include"
export LIBXML2_LIBS="-L $HOME/libxml2-2.8/ -lxml2"
export LIBRARY_PATH=$LIBRARY_PATH:$HOME/libxml2-2.8/.libs
./configure  --prefix=$HOME/axis2c  --enable-libxml2 --disable-guththila
make
make install
export AXIS2C_HOME=$HOME/axis2c
export C_INCLUDE_PATH=$C_INCLUDE_PATH:$AXIS2C_HOME/include/axis2-1.6.0
export CPLUS_INCLUDE_PATH=$CPLUS_INCLUDE_PATH:$AXIS2C_HOME/include/axis2-1.6.0
export LIBRARY_PATH=$LIBRARY_PATH:$AXIS2C_HOME/lib
```
   * 备注
      1. axis2c默认使用的的xml parser 为guththila不支持中文, 在返回报文中有汉字时程序获取不到数据;在编译的时候指定使用libxml2
      2. 需要设置`AXIS2C_HOME`环境变量为$HOME/axis2c, 运行时需要. **(不仅是axis2.xml和lib)**
      3. Axis2c根据$AXIS2C_HOME/axis2.xml的配置会发送不同的报文. 例如
```xml
        <parameter name="xml-declaration" insert="true"/>
        <parameter name="Transfer-Encoding">chunked</parameter>
```
前者决定了发送时是否有Xml declaration(<?xml version=\"1.0\" encoding=\"UTF-8\"?>);
后者决定了HTTP body是否分段。

   * 关于Action：  
程序中调用
```c
axis2_options_set_soap_version(options, env, AXIOM_SOAP11);
```
则Action在HTTP头的SOAPAction指定；否则默认为SOAP12,  
Action在HTTP Body的SOAP报文中.  
另外设置soap_version还影响报文中的namespace等。

   * AIX 7环境编译Axis2/C 1.6.0
      * 报错`#include file <sys/sockio.h> not found`
         * configure.ac: AC_CHECK_HEADERS([sys/sockio.h]) 检查文件是否存在
         * 修改util/src/platforms/unix/uuid_gen_unix.c, 若定义了`HAVE_SYS_SOCKIO_H`才需要`#include <sys/sockio.h>`
      * 报错`#include file <getopt.h> not found`
         * configure.ac: AC_CHECK_HEADERS([getopt.h])
         * 修改util/include/platforms/unix/axutil_unix.h, 若定义`HAVE_GETOPT_H`才需要`#include <getopt.h>`
      * 报错`Unexpected text __useconds encountered`
         * util/include/platforms/unix/axutil_unix.h: 根据unistd.h中声明的extern int usleep(useconds_t)修改
         * AIX中无 `__useconds_t`

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
