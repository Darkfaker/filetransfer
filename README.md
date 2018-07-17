# filetransfer
## 文件传输
### 项目概览
- 使用半同步半异步进程池构建框架
- 使用STL容器实现用户表格的管理
- 使用一致性哈希进行负载的均衡
- 使用Nginx内存池完成内存的管理

### 项目工具
- Linux centos7 , vim , gcc, g++, gdb , git

### 项目时间
2018/5/2 -- 2018/4/4

### 项目展望
- 目前已经完成基本功能，后期准备加入 mysql, redis以及使用 libevent 编写 socket
- 成型之后会做成一个 HTTP 文件服务器，会加入中间层 dispatch负载均衡器
- 负载均衡器的负载策略由 Hash 文件中的一致性哈希实现

## 目录结构：

![](https://i.imgur.com/bFDrqPE.png)

## 由于最近在加负载均衡的处理，所以代码有很多没有push更新

## 主要职责
- LinuxFun.h     项目中使用的Linux头文件
- STL.h		项目中使用的C++ STL的头文件
- cli.cpp        客户端的逻辑代码
- ser.cpp        服务端的逻辑代码
- sockpair.cpp   封装了sockpair创建双工管道的过程
- Dir.h          目录流处理的代码
- process.cpp    worker事件处理代码
- FTP.h          处理连接的代码使用半同步半异步进程池
- hash.h 一致性哈希的主代码
- md5.h md5的主代码
- md5.cpp md5的实现
# 主要逻辑图
用于管理文件与MD5的map表

![](https://i.imgur.com/EIaQLOj.png)
# 用于管理目录的map表

![](https://i.imgur.com/GFX1lsD.png)
# 设计最终目标

![](https://i.imgur.com/WNcWxx9.png)
