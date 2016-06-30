# 编写内核模块

## 示例(HelloWorld模块)
### 代码
```c
#include <linux/module.h>
#include <linux/init.h>
static int __init hello_init(void)
{
    printk(KERN_ERR "hello world!\n");
    return 0;
}
static void __exit hello_exit(void)
{
    printk(KERN_EMERG "hello exit!\n");
}

module_init(hello_init); //模块加载函数（必需）安装模块时被系统自动调用的函数
module_exit(hello_exit); //模块卸载函数（必需）卸载模块时被系统自动调用的函数
MODULE_LICENSE("GPL");		//许可证申明
MODULE_AUTHOR("LinuxUser");		//作者申明
MODULE_DESCRIPTION("Hello world module");	//模块描述
```

函数属性:
   1. `__init` 在初始化后释放内存以供重用.
   2. `__exit` 当代码被静态构建进内核时，该函数可以安全地优化了，不需要清理收尾


### 模块的编译
**注意：文件名应命名为Makefile, 不要用makefile**

单文件Makefile：
```
ifneq ($(KERNELRELEASE),)
obj-m :=hello.o
else
KDIR:= /lib/modules/$(shell uname -r)/build
all:
	make -C $(KDIR) M=$(PWD) modules
clean:
	rm -f *.ko *.o *.mod.o *.mod.c .symvers
endif
```
多文件Makefile：
```
ifneq ($(KERNELRELEASE),)
obj-m := mymodule.o
mymodule-objs := file1.o file2.o file3.o
else
KDIR := /lib/modules/$(shell uname -r)/build
all:
	make -C $(KDIR) M=$(PWD) modules
clean:
	rm -f *.ko *.o *.mod.o *.mod.c *.symvers
endif
```

## 安装与卸载
   * 加载insmod (insmod hello.ko)
   * 卸载rmmod (rmmod hello)
   * 查看lsmod
   * 加载modprobe （modprobe hello）

modprobe与insmod不同之处在于modprobe会根据文件`/lib/modules/$(shell uname -r)/modules.dep`来查看要加载的模块是否还依赖于其它模块，
如果是，modprobe 会首先找到这些模块, 把它们先加载到内核。

## 模块可选信息
   1. `MODULE_LICENSE`  
用来告知内核, 该模块带有一个许可证,没有这样的说明,加载模块时内核会抱怨。它不仅仅是一个标记。内核坚定地支持GPL兼容代码，因此如果你把许可证设置为其它非GPL兼容的（如，“Proprietary”[专利]），某些特定的内核功能将在你的模块中不可用。
有效的许可证有`GPL`、`GPLv2`、`GPL and additional rights`、`DualBSD/GPL`、`Dual MPL/GPL`和`Proprietary`
   2. 作者  
`MODULE_AUTHOR("LinuxUser");`
   3. 模块描述  
`MODULE_DESCRIPTION("Hello World Module");`, 可以用`modinfo`命令查看
   4. 模块版本  
`MODULE_VERSION("V1.0");`
   5. 模块别名  
`MODULE_ALIAS("a simple module");`
   6. 模块参数  
模块参数用于在加载模块时传递参数给模块。  
`module_param(name,type,perm)`
      * name 模块参数的名称
      * type 这个参数的类型, 常见值: bool,int,charp,
      * perm 模块参数的访问权限, 常见值:
          * S_IRUGO: 所有用户都对/sys/module中的该参数具有读权限
          * S_IWUSR: root用户可以修改/sys/module中的该参数

## 模块参数

有模块参数的代码示例
```c
/*
 * 模块参数
 * 模块加载时： insmod hello.ko name="xyz" age=20
 */
#include <linux/module.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("LinuxUser");		//作者申明
MODULE_DESCRIPTION("Hello world module with parameters");	//模块描述

static char *name = "xyz";
static int age = 21;

module_param(age, int, S_IRUGO);
module_param(name,charp,S_IRUGO);

MODULE_PARM_DESC(name, "Name");

static int __init hello_init(void)
{
	printk(KERN_EMERG "Name:%s\n",name);
	printk(KERN_EMERG "Age:%d\n",age);
	return 0;
}

static void __exit hello_exit(void)
{
	printk(KERN_EMERG "Module exit!\n");
}

module_init(hello_init);
module_exit(hello_exit);
```

模块可以接收参数
```
sudo insmod hello name="xyz"
```

`modinfo`命令显示了模块接受的所有参数，而这些也可以在/sys/module/_hello_/parameters下作为文件使用


## 内核符号导出
  * `EXPORT_SYMBOL`(符号名)
  * `EXPORT_SYMBOL_GPL`(符号名) 只能用于包含GPL许可证的模块。

示例代码
```c
/* calculator.c */
#include <linux/init.h>
#include <linux/module.h>
MODULE_LICENSE("GPL");
int add_integer(int a,int b)
{
	return a+b;
}
int sub_integer(int a,int b)
{
	return a-b;
}
static int __init sym_init(void)
{
	return 0;
}
static void __exit sym_exit(void)
{
}
module_init(sym_init);
module_exit(sym_exit);
/*
EXPORT_SYMBOL(add_integer);
EXPORT_SYMBOL(sub_integer);
*/
```
hello.c
```c
#include <linux/module.h>
#include <linux/init.h>
MODULE_LICENSE("GPL");
extern int add_integer(int a,int b);
extern int sub_integer(int a,int b);
static int __init hello_init(void)
{
	int res=add_integer(1,2);
	printk(KERN_EMERG"hello init , res=%d\n",res);
	return 0;
}
static void __exit hello_exit(void)
{
	int res=sub_integer(2,1);
	printk(KERN_EMERG"hello exit,res=%d\n",res);
}
module_init(hello_init);
module_exit(hello_exit);
```
Makefile
```
ifneq ($(KERNELRELEASE),)
obj-m := hello.o calculate.o
else
KDIR := /lib/modules/$(shell uname -r)/build
all:
	make -C $(KDIR) M=$(PWD) modules
clean:
	rm -f *.ko *.o *.mod.o *.mod.c *.symvers
endif
```
执行命令
```
   sudo insmod calculate.ko
   sudo insmod hello.ko
   cat /proc/kallsyms | grep add_integer
```

## 内核打印
在<linux/kernel.h>中定义了8种记录级别。按照优先级递减的顺序分别是：
   * KERN_EMERG '<0>'  用于紧急消息,常常是那些崩溃前的消息
   * KERN_ALERT '<1>'  需要立刻行动的消息
   * KERN_CRIT  '<2>'  严重情况
   * KERN_ERR   '<3>'  错误情况
   * KERN_WARNING '<4>' 有问题的警告
   * KERN_NOTICE  '<5>' 正常情况,但是仍然值得注意
   * KERN_INFO  '<6>'   信息型消息
   * KERN_DEBUG  '<7>'  用作调试消息
没有指定优先级的printk默认使用DEFAULT_MESSAGE_LOGLEVEL优先级，它是一个在kernel/printk.c中定义的整数。

## MISCELLANEOUS设备
Misc设备是Linux中的一种特殊的字符设备类型，是单一接入点的小型设备。
所有Misc设备共享同一个主设备号(10), 因此一个驱动(`drivers/char/misc.c`)就可以管理所有设备了, 不同的设备用从设备号来区分。


### 注册从设备号(接入点)
```c
#include <linux/miscdevice.h>
//为名为"reverse"的设备请求一个第一个可用的(动态的)次设备号
static struct miscdevice reverse_misc_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "reverse",
    .fops = &reverse_fops
};
static int __init reverse_init(void)
{
    int res;
    res = misc_register(&reverse_misc_device);
    printk(KERN_EMERG"misc_register , res=%d\n",res);
    return res;
}
//在模块卸下后注销掉该设备
static void __exit reverse_exit(void)
{
    misc_deregister(&reverse_misc_device);
    printk(KERN_EMERG"misc_deregister");
}
```
`struct miscdevice`中,`fops`字段是指向一个`file_operations`结构(`linux/fs.h`)的指针. `reverse_fops`定义：
```c
static struct file_operations reverse_fops = {
    .owner = THIS_MODULE,
    .llseek = noop_llseek
    .read   = reverse_read,
    .write = reverse_write,
    .open = reverse_open,
};
```
其中包含了一系列回调函数, 当用户空间代码`打开一个设备`, `读写`或者`关闭文件描述符`时就会执行.

如果忽略某些回调函数, 一个 **sensible** 的回调函数会被使用. 
(上例中将llseek设置为`noop_llseek()`, 因为不指定的话的默认实现改变了一个文件指针).

### 关闭/打开
这个例子为了简化，为每个打开的文件描述符分配一个新的缓冲区，并在关闭时释放。（注意，若用户空间程序不关闭文件描述符则内存不会释放）。
```c
#include <linux/sched.h>
struct buffer {
    char * data; //a pointer to the string this buffer stores
    char * end;  //the first byte after the string end
    char * read_ptr; //where read() should start reading the data from
    unsigned long size;
    wait_queue_head_t read_queue;
};

static struct buffer *buffer_alloc(unsigned long size)
{
    struct buffer * buf;
    buf = kmalloc(sizeof(struct buffer), GFP_KERNEL);
    if(unlikely(!buf))
        goto out;
    buf->data = kmalloc(size, GFP_KERNEL);
    if(unlikely(!buf->data))
    {
        kfree(buf);
        buf=NULL;
        goto out;
    }
    buf->end = buf->data;
    buf->read_ptr = buf->data;
    buf->size = size;
    init_waitqueue_head(&buf->read_queue);
    out:
        return buf;

}

static int reverse_open(struct inode * inode, struct file * file)
{
    int err = 0;
    file->private_data = buffer_alloc(buffer_size);
    return err;
}

```
内核内存使用`kmalloc()`分配，使用`kfree()`来释放。  
`kmalloc()`将内存设置为全零, 第二个参数中指定请求的内存类型。
`GFP_KERNEL` 申请一块普通的内核内存(不是在DMA或高内存区中) 以及如果需要的话函数可以睡眠（重新调度进程）.

必须检查`kmalloc()`的返回值, 访问NULL指针将导致内核异常.

`unlikely()` `likely()` 不会影响到控制流程，但是能帮助现代处理器通过分支预测技术来提升性能

`struct file`是一个标准的内核数据结构，用以存储打开的文件的信息，如当前文件位置（file->f_pos)、标志(file->f_flags），或者打开模式（file->f_mode)等。
`file->privatedata`用于关联文件到一些专有数据，它的类型是`void *`.


### 设备读/写
```c
static ssize_t reverse_read(struct file * file, char __user * out,
        size_t size, loff_t * off)
{
    struct buffer * buf = file->private_data;
    ssize_t result;
    while (buf->read_ptr == buf->end) { //loop until the data is available
        if (file->f_flags & O_NONBLOCK) {
            //file->f_flags check accounts for files opened in non-blocking mode: if there is no data, we return -EAGAIN
            result = -EAGAIN;
            goto out;
        }
        if (wait_event_interruptible(buf->read_queue, buf->read_ptr != buf->end)) {
            //if wait_event_interruptible is interrupted, it returns a non-zero value, translate to -ERESTARTSYS.
            result = -ERESTARTSYS; //the system call should be restarted.
            goto out;
        }
    }
    size = min(size, (size_t) (buf->end - buf->read_ptr));
    //copy_to_user() kernel function  copy the data from buf->data to the userspace
    if (copy_to_user(out, buf->read_ptr, size)) {
        result = -EFAULT;
        goto out;
    }
    buf->read_ptr += size;
    result = size;
out:
    return result;
}

static ssize_t reverse_write (struct file * file, const char \__user * in , size_t size, loff_t * off)
{
    struct buffer * buf = file->private_data;
    ssize_t result;
    if(buf->size > size) {
        if(copy_from_user(buf->data, in, size))
            return -EFAULT;
        buf->end = buf->data + size;
        buf->read_ptr = buf->data;
        if (buf->end > buf->data)
            reverse_phrase(buf->data, buf->end - 1);
    }
    else
    {
        return -EFBIG;
    }
    wake_up_interruptible(&buf->read_queue);
    return size;
}
```
wait_event_interruptible()是一个宏，要通过值的方式调用


测试程序
```c
nclude <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

int main(int argc,char * argv[])
{
        char buf[1024];
        int len;
        memset(buf, 0x00, sizeof buf);
        len=0;
        int fd = open("/dev/reverse", O_RDWR);
        printf("fd=[%d]. err=[%s]\n", fd, strerror(errno));
        len = write(fd, argv[1], strlen(argv[1]));
        printf("write len=[%d].err=[%s]\n",len, strerror(errno));
        len=read(fd, buf, len);
        printf("Read: len=[%d].[%s].err=[%s]\n", len, buf,strerror(errno));
        close(fd);
        return 0;
}
```

## 锁
上面的实现不能满足read和write的原子性(atomic).
```c
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

int main()
{
    char buf[1024];
    int len;
    memset(buf, 0x00, sizeof buf);
    len=0;
    int fd = open("/dev/reverse", O_RDWR);
    printf("fd=[%d]. err=[%s]\n", fd, strerror(errno));
    char * phrase="abcdefg1234567";
    len=strlen(phrase);
    if (fork()) {
        /*Parent is the writer*/
        while (1)
            write(fd, phrase, len);
    }
    else {
        /*child is the reader*/
        while (1) {
            read(fd, buf, len);
            printf("Read: %s\n", buf);
        }
    }
}
```
可能产生如下的输出:
```
Read: 7654321gfedcba
Read: 765432g1fedcba
Read: 76543fg12edcba
Read: 7654321gfedcba
```
因为kernel是`concurrent`的, 当父进程在执行`write`未完成的时候，kernel可能就调度子进程`read`了。这样数据就不一致`inconsistent`了.

解决办法是确保`write`完成返回之前`read`不会被执行. 内核中也是通过`锁`的机制来实现的. 在进程上下文中(`process context`)，如果需要的锁已经被获取了, 进程可以sleep后重试. 在`interrupt context`中需要`spinlock`.  这个例子中使用`mutex`解决问题。完整代码如下:
```c
#include <linux/module.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("LinuxUser");
MODULE_DESCRIPTION("Reverse Device");

static int reverse_phrase(char * from, char * to)
{
    char * beg=from;
    char * end=to;
    char t;
    while(beg<end)
    {
        t=* beg;  * beg = * end; * end = t;
        beg++;
        end--;
    }
    return 0;
}

static unsigned long buffer_size = 8192;
module_param(buffer_size, ulong, (S_IRUSR | S_IRGRP | S_IROTH));
MODULE_PARM_DESC(buffer_size, "Internal buffer size");


struct buffer {
    char * data; //a pointer to the string this buffer stores
    char * end;  //the first byte after the string end
    char * read_ptr; //where read() should start reading the data from
    unsigned long size;
    wait_queue_head_t read_queue;
    struct mutex lock;
};

static struct buffer *buffer_alloc(unsigned long size)
{
    struct buffer * buf;
    buf = kmalloc(sizeof(struct buffer), GFP_KERNEL);
    if(unlikely(!buf))
        goto out;
    buf->data = kmalloc(size, GFP_KERNEL);
    if(unlikely(!buf->data))
    {
        kfree(buf);
        buf=NULL;
        goto out;
    }
    buf->end = buf->data;
    buf->read_ptr = buf->data;
    buf->size = size;
    init_waitqueue_head(&buf->read_queue);
    mutex_init(&buf->lock);
    out:
        return buf;
}

static int reverse_open(struct inode * inode, struct file * file)
{
    int err = 0;
    file->private_data = buffer_alloc(buffer_size);
    return err;
}

static int reverse_release(struct inode * inode, struct file * file)
{
    int err =0;
    struct buffer * buf = file->private_data;
    if(unlikely(!buf))
        goto out;
    if(likely(buf->data))
        kfree(buf->data);
    kfree(buf);
    printk(KERN_EMERG "close reverse \n");
out:
    return err;
}

static ssize_t reverse_read(struct file * file, char \__user * out,
        size_t size, loff_t * off)
{
    struct buffer * buf = file->private_data;
    ssize_t result;
    //mutex_lock_interruptible() either grabs the mutex and returns or puts the process to sleep until the mutex is available.
    if (mutex_lock_interruptible(&buf->lock)) {
        result = -ERESTARTSYS;
        goto out;
    }
    while (buf->read_ptr == buf->end) { //loop until the data is available
        //should never sleep when holding a mutex
        mutex_unlock(&buf->lock);
        if (file->f_flags & O_NONBLOCK) {
            //file->f_flags check accounts for files opened in non-blocking mode: if there is no data, we return -EAGAIN
            result = -EAGAIN;
            goto out;
        }
        if (wait_event_interruptible(buf->read_queue, buf->read_ptr != buf->end)) {
            //if wait_event_interruptible is interrupted, it returns a non-zero value, translate to -ERESTARTSYS.
            result = -ERESTARTSYS; //the system call should be restarted.
            goto out;
        }
        if (mutex_lock_interruptible(&buf->lock)) {
            result = -ERESTARTSYS;
            goto out;
        }
    }
    size = min(size, (size_t) (buf->end - buf->read_ptr));
    //copy_to_user() kernel function  copy the data from buf->data to the userspace
    if (copy_to_user(out, buf->read_ptr, size)) {
        result = -EFAULT;
        goto out_unlock;
    }
    buf->read_ptr += size;
    result = size;
out_unlock:
    mutex_unlock(&buf->lock);
out:
    return result;
}

static ssize_t reverse_write (struct file * file, const char \__user * in , size_t size, loff_t * off)
{
    struct buffer * buf = file->private_data;
    ssize_t result;
    if (mutex_lock_interruptible(&buf->lock)) {
        result = -ERESTARTSYS;
        goto out;
    }

    if(buf->size > size) {
        if(copy_from_user(buf->data, in, size))
        {
            result = -EFAULT;
            goto out_unlock;
        }
        buf->end = buf->data + size;
        buf->read_ptr = buf->data;
        if (buf->end > buf->data)
            reverse_phrase(buf->data, buf->end - 1);
    }
    else
    {
        result = -EFBIG;
        goto out_unlock;
    }
    result =size;
    wake_up_interruptible(&buf->read_queue);
out_unlock:
    mutex_unlock(&buf->lock);
out:
    return result;
}

static struct file_operations reverse_fops = {
    .owner = THIS_MODULE,
    .llseek = noop_llseek,
    .read   = reverse_read,
    .write = reverse_write,
    .open = reverse_open,
    .release = reverse_release
};


static struct miscdevice reverse_misc_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "reverse",
    .fops = &reverse_fops
};
static int __init reverse_init(void)
{
    int res;
    if (!buffer_size)
        return -1; //Non-zero return value from a module init function indicates a failure.
    res = misc_register(&reverse_misc_device);
    printk(KERN_EMERG"misc_register , res=%d\n",res);
    return 0;
}

//在模块卸下后注销掉该设备
static void __exit reverse_exit(void)
{
    misc_deregister(&reverse_misc_device);
    printk(KERN_EMERG"misc_deregister");
}

module_init(reverse_init);
module_exit(reverse_exit);
```

## 什么时候不该写内核模块
   1. USB驱动: 查看[libusb](http://www.libusb.org/)
   2. 文件系统: 试试[FUSE](https://github.com/libfuse/libfuse)
   3. 扩展Netfilter：[libnetfilter_queue](http://www.netfilter.org/projects/libnetfilter_queue/)可能有所帮助

通常，内核里面代码的性能会更好，但是对于许多项目而言，这点性能丢失并不严重。

## 参考资料
[BE A KERNEL HACKER](https://www.linuxvoice.com/be-a-kernel-hacker/)
