#常用软件

## TrueCrypt

### 软件版本
   * `g++ -v`
      * x86_64-suse-linux  gcc version 4.8.1
   * `wx-configure --version`
      * 2.8.12
   * TrueCrypt版本
      * TrueCrypt-7.1a-Source.tar.bz2

### 编译安装步骤
   * 安装依赖软件
      * `sudo zypper install wxWidgets-devel nasm fuse-devel`
   * 安装[patch](truecrypt-7.1a.patch)
      * `cd truecrypt-7.1a-source`
      * `patch -p1 < truecrypt-7.1a-source.patch`
   * make
      * 编译后将在Main下产生truecrypt文件

### patch解决的问题
   * 报错 `fatal error: pkcs11.h: 没有那个文件或目录`
     *  因缺少PKCS #11 Cryptographic Token Interface
     ```
wget ftp://ftp.rsasecurity.com/pub/pkcs/pkcs-11/v2-20/pkcs11.h
wget ftp://ftp.rsasecurity.com/pub/pkcs/pkcs-11/v2-20/pkcs11f.h
wget ftp://ftp.rsasecurity.com/pub/pkcs/pkcs-11/v2-20/pkcs11t.h
```
      * 备注： 不正确的版本会导致报错(一些宏如CKR_NEW_PIN_MODE没有定义).

   * 报错`invalid conversion...`
```
Compiling UserInterface.cpp......Application.h: In member function 
'virtual void TrueCrypt::UserInterface::Test() const':Application.h:1420:118: 
error: invalid conversion from ‘const wchar_t*’ to ‘wxChar {aka wchar_t}’ [-fpermissive].....
```
根据编译输出的信息文件查看`Main/UserInterface.cpp`有如下的代码
```c
  // StringFormatter
 if (StringFormatter (L"{9} {8} {7} {6} {5} {4} {3} {2} {1} {0} 0",
                       "1", L"2", '3', L'4', 5, 6, 7, 8, 9, 10) 
     != L"10 9 8 7 6 5 4 3 2 1 {0}")
          throw TestFailed (SRC_POS);
```
查看Main/StringFormatter.h中StringFormatter类运算符的重载,可知是参数类型不符;
可以将这条语句注释掉。

   * 报错`ambiguous overload ...`
```
Compiling VolumeCreationWizard.cpp......Forms/VolumeSizeWizardPage.h: 
In member function 'virtual TrueCrypt::WizardPage* TrueCrypt::VolumeCreationWizard::GetPage
(TrueCrypt::WizardFrame::WizardStep)':Forms/VolumeSizeWizardPage.h:177:20: 
error: ambiguous overload for ‘operator=’ (operand types are 'wxString' and
'TrueCrypt::StringFormatter')
```
根据编译输出的信息发现`Main/Forms/VolumeCreationWizard.cpp`有如下的代码
```c
wxString freeSpaceText;
....
freeSpaceText=StringFormatter(_("Maximum possible hidden volume size for this volume is {0}."),
Gui->SizeToString (MaxHiddenVolumeSize));
```
可以看出这里应该修改为
```c
wxString tfreeSpaceText=
         StringFormatter(_("Maximum possible hidden volume size for this volume is {0}."),
Gui->SizeToString (MaxHiddenVolumeSize));
freeSpaceText = tfreeSpaceText;
```
   * 报错`undefined reference to symbol 'dlclose@@GLIBC_2.2.5'`
      * `Makefile` 修改LFLAGS   
`export LFLAGS := -ldl`

### 运行报错 `Failed to set up a loop device`
执行`sudo /sbin/modprobe loop`


# wxMEdit
## 版本
   * wxMEdit-2.9.9.tar.gz
## 依赖
   * wxWidgets-devel-2.8.12
   * libicu
   * libcurl-devel
   
## 编译安装
   * `./configure`
   * `make`
   * 将wxmedit, png, desktop复制到适当的目录中
      * cp wxmedit /usr/bin
      * cp wxmedit.png /usr/share/pixmaps/
      * cp wxmedit.desktop /usr/share/applications/

## 备注
编译`wxMEdit-3.0.2.tar.gz`时报错, 经分析是因为 wxMEdit中使用boost/tr1/unorderd_map等与c++4.8标准库tr1冲突,
从而导致重复定义。 [patch](wxMEdit-3.0.2.patch)后`./configure && make && make install`
