# GNU C内联汇编(AT&T语法)


# 基本格式
   * 指令必须在引号里
   * 指令超过一条，必须使用`\n`分隔
```c
asm ( "movl $1, %eax\n"
      "movl $0, %ebx\n"
      "int  $0x80" );
```
可以使用全局变量。示例：
```c
#include<stdio.h>
int a = 11;
int b = 22;
int result;
int main()
{
    asm ( "pusha\n"
          "movl     a,      %eax\n"
          "movl     b,      %ebx\n"
          "imull    %ebx,   %eax\n"
          "movl     %eax,   result\n"
          "popa" );
    printf ("result=[%d]\n", result);
    return 0;
}
```

编译运行
```
gcc -m32 t.c
./a.out
```

## volatile修饰符

`volatile`表示不希望编译器优化内联汇编代码

```c
asm volatile ("assembly code");
```

## __asm__

`__asm__`是ANSI C关键字, 可以使用`__volatile__`进行修饰


# 扩展格式

```c
asm ("assembly code" : output locations : input operands : changed registers);
```
   1. assembly code: 汇编代码
   2. output locations：输出位置, 包含内联汇编代码的输出值的寄存器和内存位置的列表
   3. input operands: 输入操作数, 包含内联汇编代码的输入值的寄存器和内存位置的列表
   4. changed registers：改动的寄存器, 内联代码改变的任何其他寄存器列表

若不生成输出值
```c
asm ("assembly code" : : input operands : changed registers);
```

若不改动任何寄存器:
```c
asm ("assembly code" : output locations : input operands);
```

## 指定输入和输出

扩展格式中, 可从寄存器和内存给输入,输出赋值. 输入输出列表的格式:  
```c
"constraint" (variable)
```

`variable`是C变量, 局部和全局变量都可以用.
`constraint`定义变量存放的位置. 使用它定义把变量存放在寄存器还是内存位置中. 单一字符的代码，定义如下：

|  约束 |  描述  |
|-------|---------|
|a      | Use the %eax, %ax, or %al registers.   |
|b      | Use the %ebx, %bx, or %bl registers. |
|c      | Use the %ecx, %cx, or %cl registers. |
|d      | Use the %edx, %dx, or $dl registers. |
|S      | Use the %esi or %si registers. |
|D      | Use the %edi or %di registers. |
|r      | Use any available general-purpose register. |
|q      | Use either the %eax, %ebx, %ecx, or %edx register. |
|A      | Use the %eax and the %edx registers for a 64-bit value. |
|m      | Use the variable  memory location. |
|o      | Use an offset memory location. |
|V      | Use only a direct memory location. |
|i      | Use an immediate integer value. |
|n      | Use an immediate integer value with a known value. |
|g      | Use any register or memory location available. |


除了这些约束外，输出值还包含一个约束修饰符，它指示编译器如何处理输出值：

| 输出修饰符 | 描述 |
| --------  | -------|
| +      |可以读取和写入操作数 |
| =      |只能写入操作数       |
| %      |如果必要，操作数可以和下一个操作数切换 |
| &      |在内联函数完成前，可以删除或者重新使用操作数 |

示例：
```c
asm ("assembly code" : "=a"(result) : "d"(data1), "c"(data2));
```
把变量`data1`放到`edx`中，`data2`放到`ecx`中，结果存放到`eax`中然后传送给`result`

## 寄存器

```c
#include<stdio.h>
int main()
{
    int data1 = 11;
    int data2 = 22;
    int result;
    __asm__ ("imull %%edx,  %%ecx\n"  //为了使用占位符寄存器时要写两个%
             "movl  %%ecx,  %%eax"
             : "=a"(result)
             : "d"(data1), "c"(data2));
    printf("The result is %d\n", result);
}
```

不一定要在内联汇编中指定输出值，一些汇编指令已经假设输入值包含输出值。比如movs指令输入值包含输出位置。
示例：
```c
#include<stdio.h>
int main()
{
    char input[30] = "Hello inline assembly.\n";
    char output[30];
    int len = 24;
    __asm__ __volatile__ (
            "cld\n"
            "rep    movsb"
            :
            :"S"(input), "D"(output), "c"(len));
    printf("%s", output);
    return 0;
}
```
程序把`movs`需要的三个输入值作为输入: 要复制的字符串的位置存放在`esi`中, 目标位置存放在`edi`中，要复制的字符串长度存放在`ecx`中.
输出值已被定义为输入值之一, 所以没有专门定义输出值。

**此时volatile很重要，否则编译器或许会认为这个asm段是不必要的而删除它，因为它不生成输出**

## 使用占位符
占位符是前面加%的数字, 可以在内联汇编中引入输入和输出, 可以在对于编译器方便的任何寄存器或者内存位置中声明输入和输出。
按照内联汇编中列出的每个输入值和输出值在列表中的顺序被赋予一个从0开始的数字，然后可以在汇编代码中使用占位符表示值。  
例如:
```c
asm ("assembly code"
     : "=r"(result)
     : "r"(data1), "r"(data2));
```

将生成如下的占位符：
%0: 表示包含变量值result的寄存器
%1: 表示包含变量值data1的寄存器
%2: 表示包含变量值data2的寄存器
使用占位符：
```
imull   %1, %2
movl    %2, %0
```

### 引用占位符
如果内联汇编代码中的输入和输出共享C变量，可以指定占位符作为约束值，可减少代码中需要的寄存器数量：
```c
asm ("imull %1, %0"
     : "=r"(data2)
     : "r"(data1), "0"(data2));
```
`0标记`告诉编译器使用第一个命名的寄存器存放输出值data2.

### 替换占位符
当输入输出很多时，数字型的占位符会很混乱，GNU编译器允许声明替换的名称作为占位符.
示例：
```
asm ("imull %[val1], %[val2]"
     : [val2] "=r"(data2)
     : [val1] "r"(data1), "0"(data2));
```

## 改动的寄存器列表
编译器默认输入值和输出值使用的寄存器都会被改动并做了相应处理, 所以不需要指定这些是改动了的寄存器. 若指定了，会产生错误信息  
如果内联汇编代码使用了没有被初始地声明为输入输出的任何其他寄存器，则要通知编译器, 以便避免使用它们
示例：
```c
asm ("movl  %1,     %%eax\n"
     "addl  %%eax,  %0"
     : "=r"(result)
     : "r"(data1), "0"(result)
     : "%eax" );
```
在改变的寄存器中指明要使用`%eax`，则当用`"r"`指定要使用一个寄存器时就不会选`%eax`了。
如果在内联汇编中使用了没有在输入输出中定义的任何内存位置，必须标记为被破坏的。
在改动的寄存器列表中使用`memory`通知编译器这个内存位置在内联汇编中被改动。

## 使用内存位置

在内联汇编代码中使用寄存器比较快，但也可以直接使用C变量的内存位置。约束m用于引用输入输出的内存位置。
示例：
```c
asm ("divb  %2\n"
     "movl  %eax,   %0"
     : "=m"(result)
     : "a"(dividend), "m"(divisor));
```

## 跳转
内联汇编代码中也可以包含定义位置标签，实现跳转。
示例：
```c
int a = 11;
int b = 22;
int result;
asm ("cmp   %1, %2\n"
     "jge   greater\n"
     "movl  %1, %0\n"
     "jmp   end\n"
     "greater:\n"
     "movl  %2, %0\n"
     "end:"
     : "=r"(result)
     : "r"(a), "r"(b) );
```
内联汇编中使用标签的限制：
   1. 只能跳转到相同的asm段内的标签；
   2. 不同的`asm`段不能再次使用相同的标签(内联汇编也被编码到最终的汇编代码中)
   3. 另外如果试图整合使用C关键字（如函数名称或全局变量）的标签，也会出错。

使用局部标签。
条件分支和无条件分支都运行指定一个数字加上方向标志作为标签，
方向标志指出处理器应该向哪个方向查找数字型标签，第一个遇到的标签会被采用。
示例：
```c
asm ("cmp   %1, %2\n"
     "jge   0f\n"
     "movl  %1, %0\n"
     "jmp   1f\n"
     "0:\n"
     "movl  %2, %0\n"
     "1:"
     : "=r"(result)
     : "r"(a), "r"(b) );
```
其中f（forward）指出从跳转指令向前（即到后面的代码）查找标签，b（backword）则相反，到向后（到前面的代码）找标签。

## 内联汇编用作宏函数

### C宏函数
```c
#define SUM(a, b, result) \
    ((result) = (a) + (b))
```

2）内联汇编宏函数

```c
#define GREATER(a, b, result) ( { asm ( \
            "cmp    %1,     %2\n" \
            "jge    0f\n"         \
            "movl   %1,     %0\n" \
            "jmp    1f\n"         \
            "0:\n"                \
            "movl   %2,     %0\n" \
            "1:\n"                \
            : "=r"(result)          \
            : "r"(a), "r"(b) ); })
```
