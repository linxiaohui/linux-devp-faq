# ptrace

# 问题
   1. 怎么实现对系统调用的拦截
   2. 怎样在用户层拦截和修改系统调用, 改变系统调用的参数
   3. 如何在子进程中设置断点和往运行中的程序里插入代码
   4. 调试器是如何使运行中的进程暂停并且查看和修改其寄存器和数据段
   ......

Linux提供了一种优雅的机制来完成这些：ptrace系统函数.   

`ptrace()`提供了一种使父进程得以监视和控制其它进程的方式; 它还能够改变子进程中的寄存器和内核映像, 从而可以实现断点调试和系统调用的跟踪.

# 系统调用回顾
当用户程序进行系统调用时, 它将系统调用号以及其相关参数放入相关的寄存器, 然后调用软中断`0x80`. 调用该中断后将进入操作系统设置的的中断处理函数中,
中断处理函数从寄存器中获取系统调用号和参数, 完成系统调用的功能.

在x86体系中, 系统调用号将放入`%eax`, 系统调用的参数按参数个数不同依次放入`%ebx`, `%ecx`, `%edx`, `%esi` 和 `%edi`.

x86_64体系下, 系统调用号放在`%rax`寄存器, 参数依次放在 `%rdi`, `%rsi`, `%rdx`, `%rcx`, `%r8` and `%r9`等寄存器中, 用`syscall`指令进入内核.

注意到，应该使用64位寄存器。从内核返回后，返回值就在rax寄存器里面。再次注意，不能用int 0x80指令进入内核，x86_64下是使用一条更专用的指令：syscall。

系统调用号跟x86下也有区别


需要说明的是用户程序一般不直接调用系统调用(而是调用同名的C库函数), 例如:
```c
#include <unistd.h>
int main()
{
    write(2,"Hello",5);
    return 0;
}
```
`gcc -S `生成汇编代码
```
main:
.LFB0:
        .cfi_startproc
        pushq   %rbp
        .cfi_def_cfa_offset 16
        .cfi_offset 6, -16
        movq    %rsp, %rbp
        .cfi_def_cfa_register 6
        movl    $5, %edx
        movl    $.LC0, %esi
        movl    $2, %edi
        call    write
        movl    $0, %eax
        popq    %rbp
        .cfi_def_cfa 7, 8
        ret
        .cfi_endproc
```
注意其中的`call write`, 而不是形如
```
movl   $4, %eax
movl   $2, %ebx
movl   $hello, %ecx
movl   $5, %edx
int    $0x80
```

# ptrace的工作原理

当一个进程调用了`ptrace(PTRACE_TRACEME, …)`之后, 内核为该进程设置了一个标记, 注明该进程将被跟踪.
```c
//Source: arch/i386/kernel/ptrace.c
if (request == PTRACE_TRACEME) {
    /*are we already being traced?*/
    if (current->ptrace & PT_PTRACED)
        goto out;
    /*set the ptrace bit in the process flags.*/
    current->ptrace |= PT_PTRACED;
    ret = 0;
    goto out;
}
```
在执行系统调用之前, 内核会先检查当前进程是否处于"被跟踪(traced)"的状态. 如果进程被traced内核执行`trace`系统调用(`arch/i386/kernel/entry.S`),
进入`sys_trace()`(arch/i386/kernel/ptrace.c), 内核暂停子进程并给父进程发送信号, 然后父进程可以调用`ptrace`检查子进程.
父进程之后调用`ptrace(PTRACE_CONT, ..)` 或者`ptrace(PTRACE_SYSCALL,...)`内核将重新调度子进程运行.


## 演示程序
```c
#include <stdio.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/reg.h>   /* For constants ORIG_EAX etc */
int main()
{
   pid_t child;
    long orig_eax;
    child = fork();
    if(child == 0) {
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        execl("/bin/ls", "ls", NULL);
    }
    else {
        wait(NULL);
        orig_eax = ptrace(PTRACE_PEEKUSER,
                          child, 4 * ORIG_EAX,
                          NULL);
        printf("The child made a "
               "system call %ld ", orig_eax);
        ptrace(PTRACE_CONT, child, NULL, NULL);
    }
    return 0;
}
```
使用`gcc -m32`编译运行, 输出
```
linux@linux-virtual-machine ~ $ ls
a.out  hello.c  mem.c  p.c  scall.c  scall.s  公共的  模板  视频  图片  文档  下载  音乐  桌面
linux@linux-virtual-machine ~ $ ./a.out
The child made a system call 11 linux@linux-virtual-machine ~ $ a.out  hello.c	mem.c  p.c  scall.c  scall.s  公共的  模板  视频  图片	文档  下载  音乐  桌面
```
**说明** 11是execve的系统调用号
```
cat /usr/include/x86_64-linux-gnu/asm/unistd_32.h
...
#define __NR_execve 11
...
```

子进程用`PTRACE_TRACEME`作为第一个参数调用了`ptrace()`, 子进程调用了`execve()`后将控制权交还给父进程.
此时父进程正使用`wait()`来等待来自内核的通知, `wait()`返回后父进程查看子进程的寄存器的值,
之后`PTRACE_CONT` 参数使子进程继续系统调用的过程.

# ptrace函数的参数
```
long ptrace(enum __ptrace_request request,
            pid_t pid,
            void *addr,
            void *data);

```

request: 决定了ptrace的行为与其它参数的使用方法:
   * PTRACE_ME
   * PTRACE_PEEKTEXT
   * PTRACE_PEEKDATA
   * PTRACE_PEEKUSER: 取得与子进程相关的寄存器值
   * PTRACE_POKETEXT
   * PTRACE_POKEDATA
   * PTRACE_POKEUSER
   * PTRACE_GETREGS
   * PTRACE_GETFPREGS
   * PTRACE_SETREGS
   * PTRACE_SETFPREGS
   * PTRACE_CONT
   * PTRACE_SYSCALL
   * PTRACE_SINGLESTEP
   * PTRACE_DETACH

## PTRACE_PEEKUSER
```c
#include <stdio.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/user.h>
#include <sys/syscall.h>   /* For SYS_write etc */
#include <sys/reg.h>
int main()
{   
    pid_t child;
    long orig_eax, eax;
    long params[3];
    int status;
    int insyscall = 0;
    child = fork();
    if(child == 0) {
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        execl("/bin/ls", "ls", NULL);
    }
    else {
       while(1) {
          wait(&status);
          if(WIFEXITED(status))
              break;
          orig_eax = ptrace(PTRACE_PEEKUSER,
                     child, 4 * ORIG_EAX, NULL);
          if(orig_eax == SYS_write) {
             if(insyscall == 0) {    
                /*Syscall entry*/
                insyscall = 1;
                params[0] = ptrace(PTRACE_PEEKUSER,
                                   child, 4 * EBX,
                                   NULL);
                params[1] = ptrace(PTRACE_PEEKUSER,
                                   child, 4 * ECX,
                                   NULL);
                params[2] = ptrace(PTRACE_PEEKUSER,
                                   child, 4 * EDX,
                                   NULL);
                printf("Write called with "
                       "%ld, %ld, %ld ",
                       params[0], params[1],
                       params[2]);
                }
          else { /*Syscall exit*/
                eax = ptrace(PTRACE_PEEKUSER,
                             child, 4 * EAX, NULL);
                    printf("Write returned "
                           "with %ld ", eax);
                    insyscall = 0;
                }
            }
            ptrace(PTRACE_SYSCALL,
                   child, NULL, NULL);
        }
    }
    return 0;
}
```
`gcc -m32 -I/usr/include/x86_64-linux-gnu/`编译运行, 发现并没有执行到`if(orig_eax == SYS_write)`分支.

原因与环境有关: 测试的环境是64-bit环境, `/bin/ls`为64-bit, 系统调用号是64位下的.
但编译使用32位的方式, 因此`SYS_write`为32位下系统调用号的值.

修改寄存器的访问方式: 将 `4*ORIG_EAX`等修改为`8*ORIG_RAX`等后`gcc `编译运行.

使用`PTRACE_SYSCALL`调用`ptrace`使内核在子进程做出系统调用或者准备退出的时候暂停它.
与使用`PTRACE_CONT`, 然后在下一个系统调用/进程退出时暂停它是等价的.

## PRACE_GETREGS
一次函数调用就取得所有的相关寄存器值

```c
#include <stdio.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/user.h>
#include <sys/syscall.h>
#include <sys/reg.h>

int main()
{
    pid_t child;
    long orig_eax, eax;
    long params[3];
    int status;
    int insyscall = 0;
    struct user_regs_struct regs;
    child = fork();
    if(child == 0) {
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        execl("/bin/ls", "ls", NULL);
    }
    else {
       while(1) {
          wait(&status);
          if(WIFEXITED(status))
              break;
          orig_eax = ptrace(PTRACE_PEEKUSER,
                            child, sizeof(long) * ORIG_RAX,
                            NULL);
          if(orig_eax == SYS_write) {
              if(insyscall == 0) {
                 /*Syscall entry*/
                 insyscall = 1;
                 ptrace(PTRACE_GETREGS, child,
                        NULL, &regs);
                 printf("Write called with "
                        "%lld, %lld, %lld ",
                        regs.rbx, regs.rcx,
                        regs.rdx);
             }
             else { /*Syscall exit*/
                 eax = ptrace(PTRACE_PEEKUSER,
                              child, sizeof(long) * RAX,
                              NULL);
                 printf("Write returned "
                        "with %ld ", eax);
                 insyscall = 0;
             }
          }
          ptrace(PTRACE_SYSCALL, child,
                 NULL, NULL);
       }
   }
   return 0;
}
```
编译运行,输出
```
linux@linux-virtual-machine ~ $ ls
a.out  hello.c  mem.c  p.c  r.c  scall.c  scall.s  公共的  模板  视频  图片  文档  下载  音乐  桌面
linux@linux-virtual-machine ~ $ ./a.out
a.out  hello.c	mem.c  p.c  r.c  scall.c  scall.s  公共的  模板  视频  图片  文档  下载  音乐  桌面
Write called with 116, 139992913803792, 116 Write returned with 116 linux@linux-virtual-machine ~ $
```

# 处理系统调用参数
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/user.h>
#include <sys/syscall.h>
#include <sys/reg.h>

void reverse(char * str)
{   
    int i, j;
    char temp;
    for(i = 0, j = strlen(str) - 2;
        i < j; ++i, --j) {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
    }
}

void getdata(pid_t child, long addr, char * str, int len)
{   
    char * laddr;
    int i, j;
    union u {
            long val;
            char chars[sizeof(long)];
    }data;

    i = 0;
    j = len / sizeof(long);
    laddr = str;
    while(i < j) {
        data.val = ptrace(PTRACE_PEEKDATA,
                          child, addr + i * sizeof(long),
                          NULL);
        memcpy(laddr, data.chars, sizeof(long));
        ++i;
        laddr += sizeof(long);
    }
    j = len % sizeof(long);
    if(j != 0) {
        data.val = ptrace(PTRACE_PEEKDATA,
                          child, addr + i * sizeof(long),
                          NULL);
        memcpy(laddr, data.chars, j);
    }
    str[len] = '\0';
}

void putdata(pid_t child, long addr, char * str, int len)
{   
    char * laddr;
    int i, j;
    union u {
            long val;
            char chars[sizeof(long)];
    }data;
    i = 0;
    j = len / sizeof(long);
    laddr = str;
    while(i < j) {
        memcpy(data.chars, laddr, sizeof(long));
        ptrace(PTRACE_POKEDATA, child,
               addr + i * sizeof(long), data.val);
        ++i;
        laddr += sizeof(long);
    }
    j = len % sizeof(long);
    if(j != 0) {
        memcpy(data.chars, laddr, j);
        ptrace(PTRACE_POKEDATA, child,
               addr + i * sizeof(long), data.val);
    }
}

int main()
{   
   pid_t child;
   child = fork();
   if(child == 0) {
      ptrace(PTRACE_TRACEME, 0, NULL, NULL);
      execl("/bin/ls", "ls", NULL);
   }
   else {
      long orig_eax;
      long params[3];
      int status;
      char * str, * laddr;
      int toggle = 0;
      while(1) {
         wait(&status);
         if(WIFEXITED(status))
             break;
         orig_eax = ptrace(PTRACE_PEEKUSER,
                           child, sizeof(long) * ORIG_RAX,
                           NULL);
         if(orig_eax == SYS_write) {
            if(toggle == 0) {
               toggle = 1;
               params[0] = ptrace(PTRACE_PEEKUSER,
                                  child, sizeof(long) * RDI,
                                  NULL);
               params[1] = ptrace(PTRACE_PEEKUSER,
                                  child, sizeof(long) * RSI,
                                  NULL);
               params[2] = ptrace(PTRACE_PEEKUSER,
                                  child, sizeof(long) * RDX,
                                  NULL);
               printf("rbx=[%ld],rcx=[%ld],rdx=[%ld]\n",  params[0], params[1], params[2]);
               str = (char * )calloc((params[2]+1) , sizeof(char));
               getdata(child, params[1], str, params[2]);
               reverse(str);
               putdata(child, params[1], str, params[2]);
            }
            else {
               toggle = 0;
            }
         }
      ptrace(PTRACE_SYSCALL, child, NULL, NULL);
      }
   }
   return 0;
}
```
上面的例子, 将传给write系统调用的字符串反转.

# PTRACE_SINGLESTEP
使内核在子进程的每一条指令执行前先将其阻塞, 将控制权交给父进程.
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/user.h>
#include <sys/syscall.h>
#include <sys/reg.h>
int main()
{
    pid_t child;
    child = fork();
    if(child == 0) {
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        execl("./dummy1", "dummy1", NULL);
    }
    else {
        int status;
        union u {
            long val;
            char chars[sizeof(long)];
        }data;
        struct user_regs_struct regs;
        int start = 0;
        long ins;
        while(1) {
            wait(&status);
            if(WIFEXITED(status))
                break;
            ptrace(PTRACE_GETREGS,
                   child, NULL, &regs);
            if(start == 1) {
                ins = ptrace(PTRACE_PEEKTEXT,
                             child, regs.eip,
                             NULL);
                printf("EIP: %lx Instruction "
                       "executed: %lx \n",
                       regs.eip, ins);
            }
            if(regs.orig_eax == SYS_write) {
                start = 1;
                ptrace(PTRACE_SINGLESTEP, child,
                       NULL, NULL);
            }
            else
                ptrace(PTRACE_SYSCALL, child,
                       NULL, NULL);
        }
    }
    return 0;
}
```
被跟踪的程序,汇编代码如下
```
.data
hello:
    .string "hello world\n"
.globl  main
main:
    movl    $4, %eax
    movl    $2, %ebx
    movl    $hello, %ecx
    movl    $12, %edx
    int     $0x80
    movl    $1, %eax
    xorl    %ebx, %ebx
    int     $0x80
    ret
```
使用`-m32`参数以32位方式编译运行
```
hello world
EIP: 804a03b Instruction executed: 1b8
EIP: 804a040 Instruction executed: 80cddb31
EIP: 804a044 Instruction executed: c3
```
可以查看Intel的用户手册来了解这些指令代码的意义.

# PTRACE_ATTACH
使用`PTRACE_ATTACH`调用`ptrace()`并传入子进程的pid时, 会向进程发送`SIGSTOP信号`, 可以查看和修改子进程,
然后使用`PTRACE_DETACH`调用`ptrace()`来使子进程继续运行.

被测试程序
```c
#include <stdio.h>
#include <unistd.h>
int main()
{   
   int i;
    for(i = 0;i < 10; ++i) {
        printf("counter=[%d]\n", i);
        sleep(2);
    }
    return 0;
}
```

用来检查其它进程的程序: ATTACH到进程并检查它的eip(指令指针)然后DETACH.
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/user.h>   /* For user_regs_struct etc. */
#include <sys/reg.h>
int main(int argc, char * argv[])
{
    pid_t traced_process;
    struct user_regs_struct regs;
    long ins;
    if(argc != 2) {
        printf("Usage: %s <pid to be traced>\n",
               argv[0]);
        exit(1);
    }
    traced_process = atoi(argv[1]);
    ptrace(PTRACE_ATTACH, traced_process,
           NULL, NULL);
    wait(NULL);
    ptrace(PTRACE_GETREGS, traced_process,
           NULL, &regs);
    ins = ptrace(PTRACE_PEEKTEXT, traced_process,
                 regs.eip, NULL);
    printf("EIP: %lx Instruction executed: %lx ",
           regs.eip, ins);
    ptrace(PTRACE_DETACH, traced_process,
           NULL, NULL);
    return 0;
}
```

需要注意的是根据系统设置可能需要`sudo`

# 设置断点

设置断点通常是将当前将要执行的指令替换成`trap指令`, 于是被调试的程序就会在这里停滞, 这时调试器就可以察看被调试程序的信息了.
被调试程序恢复运行以后调试器会把原指令再放回来

例子
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/user.h>   /* For user_regs_struct etc. */
#include <sys/reg.h>

void getdata(pid_t child, long addr, char * str, int len)
{   
    char * laddr;
    int i, j;
    union u {
            long val;
            char chars[sizeof(long)];
    }data;

    i = 0;
    j = len / sizeof(long);
    laddr = str;

    while(i < j) {
        data.val = ptrace(PTRACE_PEEKDATA, child,
                          addr + i * sizeof(long), NULL);
        memcpy(laddr, data.chars, sizeof(long));
        ++i;
        laddr += sizeof(long);
    }
    j = len % sizeof(long);
    if(j != 0) {
        data.val = ptrace(PTRACE_PEEKDATA, child,
                          addr + i * sizeof(long), NULL);
        memcpy(laddr, data.chars, j);
    }
}

void putdata(pid_t child, long addr, char * str, int len)
{   
    char * laddr;
    int i, j;
    union u {
            long val;
            char chars[sizeof(long)];
    }data;

    i = 0;
    j = len / sizeof(long);
    laddr = str;
    while(i < j) {
        memcpy(data.chars, laddr, sizeof(long));
        ptrace(PTRACE_POKEDATA, child,
               addr + i * sizeof(long), data.val);
        ++i;
        laddr += sizeof(long);
    }
    j = len % sizeof(long);
    if(j != 0) {
        memcpy(data.chars, laddr, j);
        ptrace(PTRACE_POKEDATA, child,
               addr + i * sizeof(long), data.val);
    }
}

int main(int argc, char * argv[])
{   
    pid_t traced_process;
    struct user_regs_struct regs, newregs;
    long ins;
    /*int 0x80, int3*/
    char code[] = {0xcd,0x80,0xcc,0};
    char backup[4];
    if(argc != 2) {
        printf("Usage: %s <pid to be traced>\n", argv[0]);
        exit(1);
    }
    traced_process = atoi(argv[1]);
    ptrace(PTRACE_ATTACH, traced_process,
           NULL, NULL);
    wait(NULL);
    ptrace(PTRACE_GETREGS, traced_process,
           NULL, &regs);
    /*Copy instructions into a backup variable*/
    getdata(traced_process, regs.eip, backup, 4);
    /*Put the breakpoint*/
    putdata(traced_process, regs.eip, code, 3);
    /*Let the process continue and execute
       the int 3 instruction*/
    ptrace(PTRACE_CONT, traced_process, NULL, NULL);
    wait(NULL);
    printf("The process stopped, putting back "
           "the original instructions\n");
    printf("Press <enter> to continue\n");
    getchar();
    putdata(traced_process, regs.eip, backup, 4);
    /*Setting the eip back to the original
       instruction to let the process continue*/
    ptrace(PTRACE_SETREGS, traced_process,
           NULL, &regs);
    ptrace(PTRACE_DETACH, traced_process,
           NULL, NULL);
    return 0;
}
```
需要 **特别注意** 的是: 上面例子中 getdata和putdata的长度: ptrace按 **字** 为单位读取/设置. 如这里长度使用trap指令的长度,
将原指令put回去后, 程序会coredump.

注意`PTRACE_SETREGS`恢复了设置断点时寄存器的值(包括eip), 因此进程从断点处继续执行.

# 往运行中的程序里面添加指令
```c
void main()
{
__asm__("jmp forward\n"
"backward:\n"
"         popl   %esi      #Get the address of     \n"
"                          # hello world string    \n"
"         movl   $4, %eax  # Do write system call  \n"
"         movl   $2, %ebx                          \n"
"         movl   %esi, %ecx                        \n"
"         movl   $12, %edx                         \n"
"         int    $0x80                             \n"
"         int3             # Breakpoint. Here the  \n"
"                          # program will stop and \n"
"                          # give control back to  \n"
"                          # the parent            \n"
"forward:                                          \n"
"         call   backward                          \n"
"         .string \"Hello World\\n\"               \n"
       );
}
```
编写改程序的主要目的是得到需要执行的指令的机器码. 使用`gdb`, `disassemble main`, `x /40bx main+3`获取要插入的机器码.

```c
int main(int argc, char * argv[])
{   pid_t traced_process;
    struct user_regs_struct regs, newregs;
    long ins;
    int len = 40;
    char insertcode[] = "\xeb\x15\x5e\xb8\x04\x00"
        "\x00\x00\xbb\x02\x00\x00\x00\x89\xf1\xba"
        "\x0c\x00\x00\x00\xcd\x80\xcc\xe8\xe6\xff"
        "\xff\xff\x48\x65\x6c\x6c\x6f\x20\x57\x6f"
        "\x72\x6c\x64\x0a\x00";
    char backup[len];
    if(argc != 2) {
        printf("Usage: %s <pid to be traced>\n", argv[0]);
        exit(1);
    }
    traced_process = atoi(argv[1]);
    ptrace(PTRACE_ATTACH, traced_process,
           NULL, NULL);
    wait(NULL);
    ptrace(PTRACE_GETREGS, traced_process,
           NULL, &regs);
    getdata(traced_process, regs.eip, backup, len);
    putdata(traced_process, regs.eip,
            insertcode, len);
    ptrace(PTRACE_SETREGS, traced_process,
           NULL, &regs);
    ptrace(PTRACE_CONT, traced_process,
           NULL, NULL);
    wait(NULL);
    printf("The process stopped, Putting back "
           "the original instructions\n");
    putdata(traced_process, regs.eip, backup, len);
    ptrace(PTRACE_SETREGS, traced_process,
           NULL, &regs);
    printf("Letting it continue with "
           "original flow\n");
    ptrace(PTRACE_DETACH, traced_process,
           NULL, NULL);
    return 0;
}
```

## 将机器码插入到进行的空闲空间
通过查看/proc/_pid_/maps可以知道这个进程中可用地址空间的分布
下面的函数可以找到这个内存映射的起始点
```c
long freespaceaddr(pid_t pid)
{
    FILE * fp;
    char filename[30];
    char line[85];
    long addr;
    char str[20];
    sprintf(filename, "/proc/%d/maps", pid);
    fp = fopen(filename, "r");
    if(fp == NULL)
        exit(1);
    while(fgets(line, 85, fp) != NULL) {
        sscanf(line, "%lx-%*lx %*s %*s %s", &addr,str, str, str, str);
        /* *表示读入的数据将被舍弃。带有*的格式指令不对应可变参数列表中的任何数据*/
        if(strcmp(str, "00:00") == 0)
            break;
    }
    fclose(fp);
    return addr;
}
```

下面的代码将机器码插入到进行的空闲空间, 之前的例子是将其插入到进程正在执行的指令流中.
```c
int main(int argc, char * argv[])
{   pid_t traced_process;
    struct user_regs_struct oldregs, regs;
    long ins;
    int len = 40;
    char insertcode[] = "\xeb\x15\x5e\xb8\x04\x00"
        "\x00\x00\xbb\x02\x00\x00\x00\x89\xf1\xba"
        "\x0c\x00\x00\x00\xcd\x80\xcc\xe8\xe6\xff"
        "\xff\xff\x48\x65\x6c\x6c\x6f\x20\x57\x6f"
        "\x72\x6c\x64\x0a\x00";
    char backup[len];
    long addr;
    if(argc != 2) {
        printf("Usage: %s <pid to be traced>\n", argv[0]);
        exit(1);
    }
    traced_process = atoi(argv[1]);
    ptrace(PTRACE_ATTACH, traced_process,
           NULL, NULL);
    wait(NULL);
    ptrace(PTRACE_GETREGS, traced_process,
           NULL, &regs);
    addr = freespaceaddr(traced_process);
    getdata(traced_process, addr, backup, len);
    putdata(traced_process, addr, insertcode, len);
    memcpy(&oldregs, &regs, sizeof(regs));
    regs.eip = addr;
    ptrace(PTRACE_SETREGS, traced_process,
           NULL, &regs);
    ptrace(PTRACE_CONT, traced_process,
           NULL, NULL);
    wait(NULL);
    printf("The process stopped, Putting back "
           "the original instructions\n");
    putdata(traced_process, addr, backup, len);
    ptrace(PTRACE_SETREGS, traced_process,
           NULL, &oldregs);
    printf("Letting it continue with "
           "original flow\n");
    ptrace(PTRACE_DETACH, traced_process,
           NULL, NULL);
    return 0;
}
```
# 参考资料
   1. [Playing with ptrace](http://www.linuxjournal.com/article/6100), [Part II](http://www.linuxjournal.com/article/6210)
   2. [Linux系统调用权威指南](http://www.chongh.wiki/blog/2016/04/08/linux-syscalls/)
   3. [ptraceDemo](https://github.com/yangbean9/ptraceDemo)
