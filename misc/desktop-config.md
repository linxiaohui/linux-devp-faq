
#手动配置软件源

##禁用原有软件源
sudo zypper mr -da
##添加科大镜像源（以 openSUSE Leap 42.3 为例）
sudo zypper ar -fc https://mirrors.ustc.edu.cn/opensuse/distribution/leap/42.3/repo/oss USTC:42.3:OSS
sudo zypper ar -fc https://mirrors.ustc.edu.cn/opensuse/distribution/leap/42.3/repo/non-oss USTC:42.3:NON-OSS
sudo zypper ar -fc https://mirrors.ustc.edu.cn/opensuse/update/leap/42.3/oss USTC:42.3:UPDATE-OSS
sudo zypper ar -fc https://mirrors.ustc.edu.cn/opensuse/update/leap/42.3/non-oss USTC:42.3:UPDATE-NON-OSS
##命令中最后一个参数为每一个源指定了一个 alias （别称），可以根据个人喜好更改。

sudo zypper addrepo -f http://mirrors.hust.edu.cn/packman/suse/openSUSE_Leap_42.3/ packman
#手动刷新软件源
sudo zypper ref

sudo zypper install ffmpeg lame gstreamer-plugins-bad gstreamer-plugins-ugly gstreamer-plugins-ugly-orig-addon gstreamer-plugins-libav libdvdcss2

sudo zypper dup --from http://mirrors.hust.edu.cn/packman/suse/openSUSE_Leap_42.3/

