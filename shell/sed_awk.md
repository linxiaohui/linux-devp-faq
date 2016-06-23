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
      * sed 's/\x0D$//'
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
   `awk 'NR==2,NR==5' test.txt`
   * 打印文件的第2行始(包括第2行在内) 5行的内容  
   `sed -n '2,+4p' test.txt`
   * 打印倒数第二行  
   `tail -2 test.txt | head -1`  
   `tac test.txt | sed -n '2p'`

## 整词匹配
   * `sed 's/\<old\>/new/g' file`


## 文本间隔

### 在每一行后面增加一空行
    `sed G`

### 文本中每一行后面将有且只有一空行
    将原来的所有空行删除并在每一行后面增加一空行
    `sed '/^$/d;G'`

### 在每一行后面增加两行空行
    `sed 'G;G'`

### 删除所有偶数行
    `sed 'n;d'`

### 在匹配`regex`的行之前插入一空行
    `sed '/regex/{x;p;x;}'`

### 在匹配`regex`的行之后插入一空行
    `sed '/regex/G'`

### 在匹配式样“regex”的行之前和之后各插入一空行
    `sed '/regex/{x;p;x;G;}'`

## 编号
### nl

### 为文件中的每一行进行编号（简单的左对齐方式）
    `sed = filename | sed 'N;s/\n/\t/'`

### 对文件中的所有行编号（行号在左，文字右端对齐）
    `sed = filename | sed 'N; s/^/     /; s/ *\(.\{6,\}\)\n/\1  /'`

### 对文件中的所有行编号，但只显示非空白行的行号:
    `sed '/./=' filename | sed '/./N; s/\n/ /'`

### 计算行数 （模拟 "wc -l"）:
    `sed -n '$='`

## 文本转换和替代

### 将每一行前导的“空白字符”（空格，制表符）删除使之左对齐
    `sed 's/^[ \t]*//'`    

### 将每一行拖尾的“空白字符”（空格，制表符）删除
    `sed 's/[ \t]*$//'`

### 将每一行中的前导和拖尾的空白字符删除
    `sed 's/^[ \t]*//;s/[ \t]*$//'`

### 在每一行开头处插入5个空格（使全文向右移动5个字符的位置）:
    `sed 's/^/     /'`

### 以79个字符为宽度，将所有文本右对齐:
    `sed -e :a -e 's/^.\{1,78\}$/ &/;ta'`  # 78个字符外加最后的一个空格

### 以79个字符为宽度，使所有文本居中。在方法1中，为了让文本居中每一行的前头和后头都填充了空格。 在方法2中，在居中文本的过程中只在文本的前面填充空格，并且最终这些空格将有一半会被删除。此外每一行的后头并未填充空格:
```bash
sed  -e :a -e 's/^.\{1,77\}$/ & /;ta'                     # 方法1
sed  -e :a -e 's/^.\{1,77\}$/ &/;ta' -e 's/\( *\)\1/\1/'  # 方法2
```
### 在每一行中查找字串“foo”，并将找到的“foo”替换为“bar”:
sed 's/foo/bar/'                 # 只替换每一行中的第一个“foo”字串
sed 's/foo/bar/4'                # 只替换每一行中的第四个“foo”字串
sed 's/foo/bar/g'                # 将每一行中的所有“foo”都换成“bar”
sed 's/\(.*\)foo\(.*foo\)/\1bar\2/' # 替换倒数第二个“foo”
sed 's/\(.*\)foo/\1bar/'            # 替换最后一个“foo”

只在行中出现字串“baz”的情况下将“foo”替换成“bar”:

sed '/baz/s/foo/bar/g'
将“foo”替换成“bar”，并且只在行中未出现字串“baz”的情况下替换:

sed '/baz/!s/foo/bar/g'
不管是“scarlet”“ruby”还是“puce”，一律换成“red”:

sed 's/scarlet/red/g;s/ruby/red/g;s/puce/red/g'  #对多数的sed都有效
gsed 's/scarlet\|ruby\|puce/red/g'               # 只对GNU sed有效


### 倒置所有行，第一行成为最后一行，依次类推（模拟“tac”）。由于某些原因，使用下面命令时HHsed v1.5会将文件中的空行删除:

sed '1!G;h;$!d'               # 方法1
sed -n '1!G;h;$p'             # 方法2

### 将行中的字符逆序排列，第一个字成为最后一字，……（模拟“rev”）:
    `sed '/\n/!G;s/\(.\)\(.*\n\)/&\2\1/;//D;s/.//'`

### 将每两行连接成一行（类似“paste”）:
    `sed '$!N;s/\n/ /'`

### 如果当前行以反斜杠“\”结束，则将下一行并到当前行末尾，并去掉原来行尾的反斜杠:
    `sed -e :a -e '/\\$/N; s/\\\n//; ta'`

### 如果当前行以等号开头，将当前行并到上一行末尾，并以单个空格代替原来行头的“=”:
    `sed -e :a -e '$!N;s/\n=/ /;ta' -e 'P;D'`

### 为数字字串增加逗号分隔符号，将“1234567”改为“1,234,567”:
    `gsed ':a;s/\B[0-9]\{3\}\>/,&/;ta'`                     # GNU sed
    `sed -e :a -e 's/\(.*[0-9]\)\([0-9]\{3\}\)/\1,\2/;ta'` # 其他sed

### 为带有小数点和负号的数值增加逗号分隔符（GNU sed）:
    `gsed -r ':a;s/(^|[^0-9.])([0-9]+)([0-9]{3})/\1\2,\3/g;ta'`

### 在每5行后增加一空白行 （在第5，10，15，20，等行后增加一空白行）:
gsed '0~5G'                      # 只对GNU sed有效
sed 'n;n;n;n;G;'                 # 其他sed

## 选择性地显示特定行

### 显示文件中的第一行
    `sed q`

### 显示文件中的前10行
    `sed 10q`

### 显示文件中的最后10行
    `sed -e :a -e '$q;N;11,$D;ba'`

### 显示文件中的最后2行
    `sed '$!N;$!D'`

### 显示文件中的最后一行
    `sed '$!d'`                        # 方法1
    `sed -n '$p'`                      # 方法2

### 显示文件中的倒数第二行:
    `sed -e '$!{h;d;}' -e x`              # 当文件中只有一行时，输入空行
    `sed -e '1{$q;}' -e '$!{h;d;}' -e x ` # 当文件中只有一行时，显示该行
    `sed -e '1{$d;}' -e '$!{h;d;}' -e x ` # 当文件中只有一行时，不输出

### 只显示匹配正则表达式的行
    `sed -n '/regexp/p'`               # 方法1
    `sed '/regexp/!d'`                 # 方法2

### 只显示不匹配正则表达式的行(grep -v)
    `sed -n '/regexp/!p'`              # 方法1，与前面的命令相对应
    `sed '/regexp/d'`                  # 方法2，类似的语法

### 查找`regexp`并将匹配行的上一行显示出来，但并不显示匹配行:
    `sed -n '/regexp/{g;1!p;};h'`

### 查找`regexp`并将匹配行的下一行显示出来，但并不显示匹配行:
    `sed -n '/regexp/{n;p;}'`

### 显示包含`regexp`的行及其前后行，并在第一行之前加上`regexp`所在行的行号 (grep -A1 -B1)
    `sed -n -e '/regexp/{=;x;1!p;g;$!N;p;D;}' -e h`

### 显示包含`AAA`、`BBB`或`CCC`的行（任意次序）:
    `sed '/AAA/!d; /BBB/!d; /CCC/!d'`  # 字串的次序不影响结果

### 显示包含`AAA`、`BBB`和`CCC`的行（固定次序）:
    `sed '/AAA.*BBB.*CCC/!d'`

### 显示包含`AAA` `BBB` 或`CCC`的行(egrep)
    `sed -e '/AAA/b' -e '/BBB/b' -e '/CCC/b' -e d`    # 多数sed
    `gsed '/AAA\|BBB\|CCC/!d'`                        # 对GNU sed有效

### 显示包含65个或以上字符的行
    `sed -n '/^.\{65\}/p'`

### 显示包含65个以下字符的行:
    `sed -n '/^.\{65\}/!p'`            # 方法1，与上面的脚本相对应
    `sed '/^.\{65\}/d'`                # 方法2，更简便一点的方法

### 显示部分文本——从包含正则表达式的行开始到最后一行结束
    `sed -n '/regexp/,$p'`

### 显示部分文本——指定行号范围（从第8至第12行，含8和12行）:
    `sed -n '8,12p'`                   # 方法1
    `sed '8,12!d'`                     # 方法2

### 显示第52行
    `sed -n '52p'`                     # 方法1
    `sed '52!d'`                       # 方法2
    `sed '52q;d'`                      # 方法3, 处理大文件时更有效率

### 从第3行开始，每7行显示一次:
    `gsed -n '3~7p'`                   # 只对GNU sed有效
    `sed -n '3,${p;n;n;n;n;n;n;}'`     # 其他sed

### 显示两个正则表达式之间的文本（包含）:
    `sed -n '/Iowa/,/Montana/p'`       # 区分大小写方式

## 选择性地删除特定行

### 显示通篇文档，除了两个正则表达式之间的内容:
    `sed '/Iowa/,/Montana/d'`

### 删除文件中相邻的重复行，只保留重复行中的第一行，其他行删除(uniq)
    `sed '$!N; /^\(.*\)\n\1$/!P; D'`

### 删除文件中的重复行，不管有无相邻。注意hold space所能支持的缓存大小，或者使用GNU sed:
    `sed -n 'G; s/\n/&&/; /^\([ -~]*\n\).*\n\1/d; s/\n//; h; P'`

### 删除除重复行外的所有行
    `sed '$!N; s/^\(.*\)\n\1$/\1/; t; D'`

### 删除文件中开头的10行:
    `sed '1,10d'`

### 删除文件中的最后一行:
    `sed '$d'`

### 删除文件中的最后两行:
    `sed 'N;$!P;$!D;$d'`

###删除文件中的最后10行:
    `sed -e :a -e '$d;N;2,10ba' -e 'P;D'`   # 方法1
    `sed -n -e :a -e '1,10!{P;N;D;};N;ba'`  # 方法2

### 删除8的倍数行:
    `gsed '0~8d'`                           # 只对GNU sed有效
    `sed 'n;n;n;n;n;n;n;d;'`                # 其他sed

### 删除匹配式样的行:
    `sed '/pattern/d'`                      # 删除含pattern的行。当然pattern可以换成任何有效的正则表达式

### 删除文件中的所有空行（与“grep '.' ”效果相同）:
    `sed '/^$/d'`                           # 方法1
    `sed '/./!d'`                           # 方法2

### 只保留多个相邻空行的第一行。并且删除文件顶部和尾部的空行(`cat -s`)
    `sed '/./,/^$/!d'`        #方法1，删除文件顶部的空行，允许尾部保留一空行
    `sed '/^$/N;/\n$/D'`      #方法2，允许顶部保留一空行，尾部不留空行

### 只保留多个相邻空行的前两行:
    `sed '/^$/N;/\n$/N;//D'`

### 删除文件顶部的所有空行:
    `sed '/./,$!d'`

### 删除文件尾部的所有空行:
    `sed -e :a -e '/^\n*$/{$d;N;ba' -e '}'`  # 对所有sed有效
    `sed -e :a -e '/^\n*$/N;/\n$/ba'`        # 同上，但只对 gsed 3.02.*有效

## 注意事项
### 简写
sed -e '/AAA/b' -e '/BBB/b' -e '/CCC/b' -e d
GNU sed能让命令更紧凑:
sed '/AAA/b;/BBB/b;/CCC/b;d'      # 甚至可以写成
sed '/AAA\|BBB\|CCC/b;d'
### 速度优化
当由于某种原因（比如输入文件较大、处理器或硬盘较慢等）需要提高命令执行速度时，可以考虑在替换命令（“s/.../.../”）前面加上地址表达式来提高速度。举例来说:
sed 's/foo/bar/g' filename         # 标准替换命令
sed '/foo/ s/foo/bar/g' filename   # 速度更快
sed '/foo/ s//bar/g' filename      # 简写形式
当只需要显示文件的前面的部分或需要删除后面的内容时，可以在脚本中使用“q”命令（退出命令）。在处理大的文件时，这会节省大量时间。因此:
sed -n '45,50p' filename           # 显示第45到50行
sed -n '51q;45,50p' filename       # 一样，但快得多

#AWK 单行脚本

Unix:
awk '/pattern/ {print "$1"}'    # 标准 Unix shell环境

## 文本间隔
### 每行后面增加一行空行:
    `awk '1;{print ""}'`
    `awk 'BEGIN{ORS="\n\n"};1'`

### 每行后面增加一行空行。输出文件不会包含连续的两个或两个以上的空行
    `awk 'NF{print $0 "\n"}'`

### 每行后面增加两行空行:
    `awk '1;{print "\n"}'`

### 编号和计算

### 以文件为单位，在每句行前加上编号 （左对齐）
    `awk '{print FNR "\t" $0}' files*`

### 用制表符 （\t） 给所有文件加上连贯的编号:
    `awk '{print NR "\t" $0}' files*`

### 以文件为单位，在每句行前加上编号 （编号在左，右对齐）
    `awk '{printf("%5d : %s\n", NR,$0)}'`

### 给非空白行的行加上编号
    `awk 'NF{$0=++a " :" $0};{print}'`
    `awk '{print (NF? ++a " :" :"") $0}'`

### 计算行数
    `awk 'END{print NR}'`

### 计算每行每个区域之和
    `awk '{s=0; for (i=1; i<=NF; i++) s=s+$i; print s}'`

### 计算所有行所有区域的总和
    `awk '{for (i=1; i<=NF; i++) s=s+$i}; END{print s}'`

### 打印每行每区域的绝对值:
    `awk '{for (i=1; i<=NF; i++) if ($i < 0) $i = -$i; print }'`
    `awk '{for (i=1; i<=NF; i++) $i = ($i < 0) ? -$i : $i; print }'`

### 计算所有行所有区域（词）的个数
    `awk '{ total = total + NF }; END {print total}' file`

### 打印两个模式之间的行
    `awk '/START_PATTERN/,/END_PATTERN/' file`
    
### 打印包含`Beth`的行数
    `awk '/Beth/{n++}; END {print n+0}' file`

### 打印第一列最大的行，并且在行前打印出这个最大的数:
    `awk '$1 > max {max=$1; maxline=$0}; END{ print max, maxline}'`

### 打印每行的列数，并在后面跟上此行内容
    `awk '{ print NF ":" $0 } '`

### 打印每行的最后一列
    `awk '{ print $NF }'`

### 打印最后一行的最后一列:
    `awk '{ field = $NF }; END{ print field }'`

### 打印列数超过4的行
    `awk 'NF > 4'`

### 打印最后一列大于4的行:
    `awk '$NF > 4'`

## 构建字符串
### 构建一指定长度的字符串
    `awk 'BEGIN{while (a++<513) s=s " "; print s}'`

### 在某一位置中插入以特定长度的字符串。例子：在每行第6列后插入49个空格:
    `gawk --re-interval 'BEGIN{while(a++<49)s=s " "};{sub(/^.{6}/,"&" s)};1'`

## 构建数组
以下两个部分并不是一句话脚本，但是这些技巧相当便捷所以也包括进来。  
构建一个叫`month`的数组，以数字为索引，month[1]就是'Jan'，month[2]就是'Feb'，month[3]就是'Mar'，以此类推:
    `split("Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec", month, " ")`

构建一个叫`mdigit`的数组，以字符串为索引，mdigit["Jan"] 等于 1，mdigit["Feb"] 等于 2，等等。需要有"month"数组:
    `for (i=1; i<=12; i++) m_digit[month[i]] = i`

## 文本转换和替代
### 在Unix环境：转换DOS新行 （CR/LF） 为Unix格式:
    `awk '{sub(/\r$/,"")};1'`   # 假设每行都以Ctrl-M结尾

### 在Unix环境：转换Unix新行 （LF） 为DOS格式:
    `awk '{sub(/$/,"\r")};1'`

### 在DOS环境：转换Unix新行 （LF） 为DOS格式:
    `awk 1`

### 在DOS环境：转换DOS新行 （CR/LF） 为Unix格式。DOS版本的awk不能运行, 只能用gawk:
    `gawk -v BINMODE="w" '1' infile >outfile`

### 用 `tr` 替代的方法:
    `tr -d \r <infile >outfile`            # GNU tr 版本为 1.22 或者更高

### 删除每行前的空白（包括空格符和制表符），使所有文本左对齐:
    `awk '{sub(/^[ \t]+/, "")};1'`

### 删除每行结尾的空白（包括空格符和制表符）
    `awk '{sub(/[ \t]+$/, "")};1'`

### 删除每行开头和结尾的所有空白（包括空格符和制表符）
    `awk '{gsub(/^[ \t]+|[ \t]+$/,"")};1'`
    `awk '{$1=$1};1'`           # 每列之间的空白也被删除

### 在每一行开头处插入5个空格 （做整页的左位移）:
    `awk '{sub(/^/, "     ")};1'`

### 用79个字符为宽度，将全部文本右对齐
    `awk '{printf "%79s\n", $0}' file*`

### 用79个字符为宽度，将全部文本居中对齐
    `awk '{l=length();s=int((79-l)/2); printf "%"(s+l)"s\n",$0}' file*`

### 每行用 "bar" 查找替换 "foo":
    `awk '{sub(/foo/,"bar")}; 1'`           # 仅仅替换第一个找到的"foo"
    `gawk '{$0=gensub(/foo/,"bar",4)}; 1'`  # 仅仅替换第四个找到的"foo"
    `awk '{gsub(/foo/,"bar")}; 1'`          # 全部替换

### 在包含 "baz" 的行里，将 "foo" 替换为 "bar":
    `awk '/baz/{gsub(/foo/, "bar")}; 1'`

### 在不包含 "baz" 的行里，将 "foo" 替换为 "bar":
    `awk '!/baz/{gsub(/foo/, "bar")}; 1'`

### 将 "scarlet" 或者 "ruby" 或者 "puce" 替换为 "red"
    `awk '{gsub(/scarlet|ruby|puce/, "red")}; 1'`

### 倒排文本(`tac`)
    `awk '{a[i++]=$0} END {for (j=i-1; j>=0;) print a[j--] }' file*`

### 如果一行结尾为反斜线符，将下一行接到这行后面（如果有连续多行后面带反斜线符，将会失败）:
    `awk '/\\$/ {sub(/\\$/,""); getline t; print $0 t; next}; 1' file*`

### 排序并打印所有登录用户的姓名:
    `awk -F ":" '{ print $1 | "sort" }' /etc/passwd`

### 以相反的顺序打印出每行的前两列
    `awk '{print $2, $1}' file`

### 调换前两列的位置
    `awk '{temp = $1; $1 = $2; $2 = temp}' file`

### 打印每行，并删除第二列
    `awk '{ $2 = ""; print }'`

### 倒置每行并打印
    `awk '{for (i=NF; i>0; i--) printf("%s ",i);printf ("\n")}' file`

### 用逗号链接每5行
    `awk 'ORS=NR%5?",":"\n"' file`

## 选择性的打印某些行
### 打印文件的前十行
    `awk 'NR < 11'`

### 打印文件的第一行
    `awk 'NR>1{exit};1'`

### 打印文件的最后两行
    `awk '{y=x "\n" $0; x=$0};END{print y}'`

### 打印文件的最后一行
    `awk 'END{print}'`

### 打印匹配正则表达式的行
    `awk '/regex/'`
打印不匹配正则表达式的行
    `awk '!/regex/'`

### 打印第5列等于"abc123"的行:
    `awk '$5 == "abc123"'`

### 打印第5列不等于"abc123"的行
    `awk '$5 != "abc123"'`
    `awk '!($5 == "abc123")'`

### 用正则匹配某一列:
    `awk '$7  ~ /^[a-f]/'`    # 打印第7列匹配的行
    `awk '$7 !~ /^[a-f]/'`    # 打印第7列不匹配的行

### 打印匹配正则表达式的前一行，但是不打印当前行:
    `awk '/regex/{print x};{x=$0}'`
    `awk '/regex/{print (x=="" ? "match on line 1" : x)};{x=$0}'`

### 打印匹配正则表达式的后一行，但是不打印当前行:
    `awk '/regex/{getline;print}'`

### 以任何顺序查找包含 AAA、BBB 和 CCC 的行:
    `awk '/AAA/; /BBB/; /CCC/'`

### 以指定顺序查找包含 AAA、BBB 和 CCC 的行:
    `awk '/AAA.*BBB.*CCC/'`

### 打印长度大于64个字节的行
    `awk 'length > 64'`

### 打印长度小于64个字节的行
    `awk 'length < 64'`

### 打印从匹配正则起到文件末尾的内容
    `awk '/regex/,0'`
    `awk '/regex/,EOF'`

### 打印指定行之间的内容 （8-12行, 包括第8和第12行）:
    `awk 'NR==8,NR==12'`

### 打印第52行:
    `awk 'NR==52'`
    `awk 'NR==52 {print;exit}'`          # 对于大文件更有效率

### 打印两个正则匹配间的内容
    `awk '/Iowa/,/Montana/'`             # 大小写敏感

## 选择性的删除某些行
### 删除所有空白行(`grep '.'`)
    `awk NF`
    `awk '/./'`

### 删除重复连续的行
    `awk 'a !~ $0; {a=$0}'`

### 删除重复的、非连续的行:
    `awk '! a[$0]++'`                     # 最简练
    `awk '!($0 in a) {a[$0];print}'`      # 最有效
