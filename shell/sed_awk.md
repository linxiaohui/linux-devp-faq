#SED和AWK

## DOS与UNIX文件格式转换

一般而言文本文件有Windows格式与UNIX格式，二者的区别是：
注意脚本要保存为UNIX格式，实践中发现Windows在一些情况下有问题
   * dos2unix
   * vim 
      * DOS转UNIX：:set fileformat=unix
      * UNIX转DOS：:set fileformat=dos
      * 简写 :set ff=
   * dos2unix: 
      * sed -i'' "s/\r//" file 
      * cat file | col -b > newfile 
      * cat file | tr -d "\r" > newfile 
      * cat file | tr -d "\015" > newfile 
   * unix2dos: 
     * sed -i'' "s/$/\r/" file 
     * sed -i'' "s/$/\x0d/" file
备注：
   * UTF-8，需要注意是否有BOM标志(EF BB BF)
   * cat –v 显示不可见字符

## 按行截取文件
   * sed   
   sed -n '起始行号,结束行号p' 文件名   
   若结束行号<开始行号 打印开始行号
   * awk   
   awk '{if ( NR==4 ) print $0}' list.txt
   * 打印文件的第一行  
   head -1 test.txt
   * 打印文件的第2行  
   sed -n '2p' test.txt
   * 打印文件的第2到5行  
   sed -n '2,5p' test.txt
   * 打印文件的第2行始(包括第2行在内) 5行的内容  
   sed -n '2,+4p' test.txt
   * 打印倒数第二行  
   tail -2 test.txt | head -1  
   tac test.txt | sed -n '2p'  

