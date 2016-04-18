#SED和AWK

## DOS与UNIX文件格式转换

一般而言文本文件有DOS格式与UNIX格式
   * 二者的区别是：
      * DOS格式换行为 0x0D0x0A
      * UNIX格式换行为 0x0A
      * `fopen`函数模式参数"wb"与"w"的区别
         * Windows下"w"打开文件，向文件中写0x0A会在前自动添加0x0D
         * 注意FTP等方式传输非文本文件需要用BIN模式

注意脚本要保存为UNIX格式，实践中发现DOS格式在一些情况下有问题
   * dos2unix
   * vim 
      * DOS转UNIX `:set fileformat=unix`
      * UNIX转DOS `:set fileformat=dos`
      * 简写 `:set ff=`
   * dos2unix: 
      * `sed -i'' "s/\r//" file` 
      * `cat file | col -b > newfile` 
      * `cat file | tr -d "\r" > newfile` 
      * `cat file | tr -d "\015" > newfile` 
   * unix2dos: 
     * `sed -i'' "s/$/\r/" file` 
     * `sed -i'' "s/$/\x0d/" file`
备注：
   * UTF-8格式的文本需要注意是否有BOM标志(EF BB BF)
   * `cat –v` 显示不可见字符

## 按行截取文件
   * sed   
   `sed -n '起始行号,结束行号p' 文件名`   
   若结束行号<开始行号 打印开始行号
   * awk   
   `awk '{if ( NR==4 ) print $0}' list.txt`
   * 打印文件的第一行  
   `head -1 test.txt`
   * 打印文件的第2行  
   `sed -n '2p' test.txt`
   * 打印文件的第2到5行  
   `sed -n '2,5p' test.txt`
   * 打印文件的第2行始(包括第2行在内) 5行的内容  
   `sed -n '2,+4p' test.txt`
   * 打印倒数第二行  
   `tail -2 test.txt | head -1`  
   `tac test.txt | sed -n '2p'`  

## 整词匹配
   * `sed 's/\<old\>/new/g' file` 