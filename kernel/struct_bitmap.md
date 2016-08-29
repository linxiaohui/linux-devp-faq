# 内核数据结构之位图

## 位图声明
有两种声明位数组的通用方法。

第一种简单的声明一个位数组的方法是定义一个 `unsigned long` 数组，例如：
```c
unsigned long my_bitmap[8]
```

第二种方法是使用 `DECLARE_BITMAP` 宏，它定义于`include/linux/types.h`:
```c
#define DECLARE_BITMAP(name,bits) \
    unsigned long name[BITS_TO_LONGS(bits)]
```

可以看到`DECLARE_BITMAP`宏使用两个参数：
   * name - 位图名称;
   * bits - 位图中位数;
   * `BITS_TO_LONGS` 宏计算 bits 中有多少个 8 字节元素
```c
#define BITS_PER_BYTE           8
#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))
#define BITS_TO_LONGS(nr)       DIV_ROUND_UP(nr, BITS_PER_BYTE * sizeof(long))
```

## API
   * set_bit;
   * clear_bit.

## 参考资料
   * [Linux 内核中的位数组和位操作](https://linux.cn/article-7707-1.html)
