# filetransfer
文件传输

目录结构：
![](https://i.imgur.com/OmN24SJ.png)

###由于最近在加负载均衡的处理，所以代码有很多没有push更新
##主要职责
LinuxFun.h     项目中使用的Linux头文件
STL.h		项目中使用的C++ STL的头文件
cli.cpp        客户端的逻辑代码
ser.cpp        服务端的逻辑代码
sockpair.cpp   封装了sockpair创建双工管道的过程
Dir.h          目录流处理的代码
process.cpp    worker事件处理代码
FTP.h          处理连接的代码使用半同步半异步进程池

#主要逻辑图
用于管理文件与MD5的map表
![](https://i.imgur.com/EIaQLOj.png)
#用于管理目录的map表
![](https://i.imgur.com/GFX1lsD.png)
#设计最终目标
![](https://i.imgur.com/WNcWxx9.png)