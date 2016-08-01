# GCC 预处理

## define
   * `#`:  将传递的宏字符串化
   * `##`: 将两个名字连接成一个
   * 使用`__VA_ARGS__`定义可变参数宏

示例:
```c
#define TEST(ARGTERM)  printf("the term " #ARGTERM "is a string\n")
#define PASTE(a, b)          a##b
#define err(...)  fprintf(stderr, __VA_ARGS__)
#define dprintf(fmt, arg...)    printf(fmt, ##arg)
int main()
{
  err("%s %d\n", "error code is", 22);  /*error code is 22*/
  TEST(abc);   /*the term abcis a string*/
  TEST(PASTE(var,val)); /*the term PASTE(var,val)is a string*/
  int PASTE(a,b)=1024;
  printf("ab=[%d]\n",ab);  /*ab=[1024]*/
  return 0;
}
```
**说明**： 使用`gcc -E`查看预处理生成的代码

## error 和 warning


## if, elif, else, endif

支持的运算符：`+-*/`, `<< >>`, `&&`, `||`, `!`等
```c
#if defined (CONFIG_A) || defined (CONFIG_B)
/*CODEHERE*/
#endif
```

## gcc预定义宏

| 名字  | 意义 |
|-------|------|
| __BASE_FILE__ | 完整的源文件名路径 |
| __cplusplus   | c++程序 |
| __DATE__ |  |
| __FILE__ | 源文件名 |
| __func__ | 替代__FUNCTION__, __FUNCTION__以被GNU不推荐使用 |
| __TIME__ |   |
| __LINE__ |   |
| __VERSION__ | gcc版本 |


## 示例
```c
#define   min(X,  Y)  \
    (__extension__ ({typeof (X) ____x = (X), ____y = (Y);  \
    (____x < ____y) ? ____x : ____y; }))

#define   max(X,  Y)  \
    (__extension__ ({typeof (X) ____x = (X), ____y = (Y);  \
    (____x > ____y) ? ____x : ____y; }))
/*这样做的目的是消除宏对X, Y的改变的影响. 例如. result = min(x++, --y);*/

/*圆括号定义的复合语句可以生成返回值*/
result = ({ int a = 5;
            int b;
            b = a + 3;
          });

#ifdef __cplusplus
extern "C"{
#endif
int foo1(void);
int foo2(void);
#ifdef __cplusplus
}
#endif
```

# GCC对C语言的扩展

```c
void fetal_error()  __attribute__(noreturn); //声明函数：无返回值
__attribute__((noinline)) int foo1(){}      //定义函数：不扩展为内联函数
int getlim()  __attribute__((pure, noinline)); //声明函数：不内联，不修改全局变量
void mspec(void)  __attribute__((section("specials"))); //声明函数：连接到特定节中
//补充：除非使用-O优化级别，否则函数不会真正的内联。
```

## gcc内嵌函数：
```c
void * __builtin_return_address(unsigned int level);
void * __builtin_frame_address(unsigned int leve);
```
以上两个函数可以用于回溯函数栈

## typeof

## 连接器脚本ld
`ld --verbose`查看默认链接脚本

##  `__attribute__`
`__attribute__`可以设置函数属性(Function Attribute), 变量属性(Variable Attribute)和类型属性(Type Attribute). 语法格式为:
```
__attribute__ ((attribute-list))
```
必须位于声明的尾部`;`之前.

同时使用多个属性的方式:
   1. 在同一个函数声明里使用多个 `__attribute__`
   2. `attribute-list`写多个

例如:
```c
extern void die(const char * format, ...)　\
__attribute__((noreturn)) __attribute__((format(printf, 1, 2)));
//或者写成
extern void die(const char * format, ...)  __attribute__((noreturn, format(printf, 1, 2)));
```

### 函数属性
常用的函数属性:
   * format
   * noreturn
   * const
   * constructor/destructor
   * no_instrument_function
   * pure: 函数没有`副作用`, 当不需要它的返回值时, 它可能被优化掉

示例:
```c
#include <stdio.h>
#include <stdlib.h>
int atoi(const char* ptr)
{
  printf("input arg=[%s]\n", ptr);
  return 0;
}
int main()
{
  const char* ptr = "12";
  atoi(ptr);
  /*int r=atoi(ptr)*/
  return 0;
}
```
在`gcc version 5.3.1`中编译运行不输出. 因为GCC中`atoi`的声明是`__attribute__ ((__pure__))`.
尽管代码的实现有`副作用`, 调用`atoi`没有获取其返回值, 被编译优化掉了.

### 变量属性
对变量或结构体成员进行属性设置. 常用属性:
   * aligned
   * packed

### 类型属性
对`struct`, `union`进行属性设置. 常用属性:
   * aligned
   * packed
