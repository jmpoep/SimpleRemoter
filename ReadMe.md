﻿# 项目简介

**原始来源：** [zibility](https://github.com/zibility/Remote)

**功能概述：** 基于gh0st的远程控制器：实现了终端管理、进程管理、窗口管理、桌面管理、文件管理、语音管理、视频管理、服务管理、注册表管理等功能。
如果您热爱研究控制程序，喜欢本项目，请您对该项目添加星标。Fork、Watch此项目，提交Issues，发起Pull Request都是受欢迎的。

根据本人空闲情况，此项目会不定期更新。若您想对该项目了解更多技术细节，喜欢讨论软件的各方面，学习和交流请通过适当的方式联系。

**起始日期**：2019.1.1

## 主控程序
主控程序为**YAMA.exe**是Server端，Release发布版本在单台电脑只能运行一个示例。
下面展示主控程序运行界面，所有功能均可用，程序运行稳定。
某些功能要求受控程序以管理员权限运行。

![主界面](./images/Yama.png)

主界面以列表形式展示连接到本机的受控程序。
选中某个主机以便进行远程控制。

![终端管理](./images/Console.png)

终端管理打开命令行窗口，可以执行远程命令。

![进程管理](./images/Process.png)

进程管理显示受控机器上面正在运行的进程，可对进程进行启停操作。

![窗口管理](./images/Window.png)

窗口管理显示受控机器上面打开的窗口或程序，可对其进行操作。

![桌面管理](./images/Remote.png)

桌面管理即"远程桌面"，控制远程机器。

![文件管理](./images/FileManage.png)

文件管理即在本机和受控机器之间传输文件。

![语音管理](./images/Voice.png)

语音管理即监听受控机器的声音，需受控机器有声音输入设备。

![视频管理](./images/Video.png)

视频管理即打印受控机器的摄像头，需受控机器有摄像头。

![服务管理](./images/Service.png)

服务管理即打开受控机器上面的服务列表。

![注册表管理](./images/Register.png)

注册表管理即打开受控机器上面的注册表。

## 受控程序
![主界面](./images/TestRun.png)

受控程序是Client端，分为2种运行形式（"类型"）：单个程序 **（1）** ghost.exe和 **（2）** TestRun.exe+ServerDll.dll形式。
（1）单个程序运行时，不依赖其他动态链接库，而第（2）种情况运行时，由EXE程序调用核心动态链接库。

# 更新日志

2019.1.5

1、整理垃圾排版，优化上线下线处理逻辑。
2、修复部分内存泄漏问题，改善线程处理逻辑。
3、修复客户端不停断线重连的缺陷。解决部分内存泄漏缺陷。
4、解决几处缺陷。【遗留问题】文件管理对话框释放资源导致第2次打开崩溃。

2019.1.6

1、改用EnumDisplaySettings获取屏幕大小，原方法获取屏幕大小不准。

2、将FileManagerDlg、InputDlg、FileTransferModeDlg、TrueColorToolBar还原到gh0st最初版本。

3、新增项目"ghost"，不通过TestRun调用dll，而是直接生成可执行文件。

4、修复开启视频，客户端产生的一处内存泄漏缺陷，m_pCapture需要释放。

2019.1.7

1、ghost单台电脑只允许启动唯一的实例。

2、远程桌面反应迟钝，改用每秒传送8帧屏幕，后续有待优化。

2019.1.8

1、发现传屏的瓶颈在zlib压缩数据，更新zlib到版本V1.2.11，提高传送屏幕速度到每秒10帧。

2、ghost的类CBuffer不需要临界区。

2019.1.9

1、服务端IOCPServer类的工作线程改为计算机核心个数的2倍。

2、解决服务端主动退出的内存泄漏问题，泄漏源在OVERLAPPEDPLUS。

2019.1.10

1、服务端远程控制增加全屏（系统右键菜单）、退出全屏（F11）的功能。

2、修复客户端机器屏幕缩放时远程桌面鼠标光标位置不准确的问题。（跟踪光标受影响）

3、发现服务端需要采用默认英文输入法，才能在远程桌面输入中文（怀疑本地输入法截获消息）。

4、添加崩溃时写dump文件的代码。

2019.1.11

1、修复文件管理对话框多次打开崩溃的问题（【遗留问题】）。

2、遗留问题：远程cmd窗口总是将输入命令输出2次、文件对话框的菜单操作可能已失效。

2019.1.12

1、还原客户端的文件管理模块代码为gh0st的源码3.6版本.

2、修复上述"cmd窗口总是将输入命令输出2次"遗留问题。

3、打开注册表关闭后崩溃，参照按对文件管理窗口的修改进行处理。遗留问题：
	并无内存泄漏，但退出时报"HEAP: Free Heap modified after it was freed"问题。

4、退出时睡眠一会，等待服务端清理，发现这样可以避免退出时崩溃的概率。

5、发布稍微稳定的版本V1.0.0.1。

2019.1.13

1、在主对话框清理子窗口的资源（原先在各自的OnClose函数），通过CLOSE_DELETE_DLG控制。

2、修正CFileManagerDlg的构造函数调用SHGetFileInfo和FromHandle方法，解决多次打开崩溃。

3、更新服务端zlib版本为V1.2.11。（与客户端不同，因inflate_fast 崩溃，没有采用汇编）

2019.1.15

1、修复主控端CTalkDlg的内存泄漏问题，被控端即时消息对话框置于顶层。

2、SAFE_DELETE(ContextObject->olps)有崩溃概率。改为主控端退出时先令被控端退出，就没有内存泄漏。

3、开关音频时偶有内存泄漏，waveInCallBack线程不能正常退出。

2019.1.16

1、智能计时宏AUTO_TICK有问题，不应该用无名的局部变量auto_tick。

2、采用由Facebook所开发的速度更快的压缩库zstd，提高程序运行效率。
	参看：https://github.com/facebook/zstd

2019.1.17

1、添加比zstd更快的压缩库（压缩率不如zstd和zlib）lz4 1.8.3，参看
	https://github.com/lz4/lz4

2、修复被控端屏幕被缩放显示时远程桌面跟踪鼠标的位置不准的问题。

3、修复语音监听的问题，2个事件CAudio修改为非"Manual Reset"。

2019.1.18

1、整理部分垃圾代码。

2、发布V1.0.0.2。

2018.1.19

1、发现使用lz4压缩库时监控端程序进行远程桌面操作时容易崩溃，原因不明。

2、修复内存泄漏缺陷，在throw "Bad Buffer"的情况需要释放申请的内存。

2019.1.20

1、发现不管是采用zstd还是zlib，主控端在进行桌面控制时均有崩溃的几率（zlib较小）。

2、改用zlib压缩解压库。

3、完善追踪鼠标时鼠标形态变化时的展现效果。

4、当退出远程桌面窗口全屏状态时，不再向远程被控端发送F11。

5、发现在有线网络条件下主控端崩溃几率较小。

6、禁用主控端输入法，解决使用远程桌面在被控端输入时的麻烦问题。

2019.1.21

减少远程桌面new缓冲区的频率，将部分从堆上new固定内存的操作改用从栈上分配内存。

2019.1.22

减少音频视频捕获过程中频繁申请内存。

2019.1.25

1、修复被控端消息提示对话框在消息换行时显示不完整的问题。

2、添加/完善录制远程被控端视频的功能。

3、修复语音监听对话框显示已收到数据不更新状态的问题。

4、发现"发送本地语音"会导致主控端容易崩溃的问题，现象类似于操作远程桌面时的随机崩溃。

5、设置视频监控对话框为可调整大小，为其设置图标。

2019.1.26

1、发布V1.0.0.3。

2、修复Release模式打不开远程视频，或打开视频时画面卡住的问题，问题出在CCaptureVideo GetDIB。

2019.2.4

清理垃圾注释、整理不良排版，对代码略有改动。

遗留问题：文件管理功能无效、主控端随机崩溃。因此有必要将文件管理的功能屏蔽。

发布V1.0.0.4。

2019.3.24

1、将"2015Remote.rc"的一个光标文件"4.cur"的路径由绝对路径改为相对路径。

2、新增Release模式编译后控制台运行时不可见，新增TestRun向注册表写入开机自启动项。

2019.3.29

1、主控端和受控端同时修改LOGIN_INFOR结构，修复了受控端上报的操作系统信息不准确的问题。

2、发布V1.0.0.5。

注意：此次更新后的主控端需要和受控端匹配使用，否则可能出现问题。

2019.4.4

ghost项目采用VS2012 xp模式编译，以便支持在XP系统上运行。

2019.4.14

在2015RemoteDlg.h添加宏CLIENT_EXIT_WITH_SERVER，用于控制ghost是否随Yama退出。

2019.4.15

明确区分开退出被控端和退出主控端2个消息，只有发送退出被控端消息才会停止Socket客户端。

2019.4.19
1、TestRun读取配置文件改为setting.ini，配置项为 [settings] localIp 和 ghost。
2、CAudio的线程waveInCallBack在while循环有一处return，已改为break.

2019.4.20
TestRun在写入开机自启动项时先提升权限，以防止因权限不足而写注册表失败。

2019.4.30
升级全部项目采用Visual Studio Community 2015编译。

2019.5.6
当TestRun、ClientDemo运行时若未成功加载ServerDll.dll，则给出提示。
所有项目均采用平台工具集"Visual Studio 2012 - Windows XP (v110_xp)"，以支持在XP上运行。

2019.5.7
1、添加对远程IP使用域名时的支持，若IP为域名，先将域名进行解析后再连接。
2、添加文档“使用花生壳.txt”，介绍了如何使用花生壳软件搭建远程监控系统。

2019.5.8
优化左键点击Yama托盘图标的效果。

2019.5.11
优化远程桌面发送屏幕的功能，可动态调整发送屏幕的速率。

2019.8.25
调整项目设置，解决采用VS2015编译时某些项目不通过的问题。

2021.3.14
修复了若干个问题。

2024.9.6
1.新增"2019Remote.sln"支持使用Visual Studio 2019编译项目。
2.增加了使用VLD的操作方法，详见"server\2015Remote\stdafx.h"。
注意：自VS2019开始，不支持XP系统了（微软已经声明这个变更）。如果有需要在XP系统进行监控的需求，推荐使用"2015Remote.sln"。
如果使用VS2015编译，需将WindowsTargetPlatformVersion修改为8.1，将PlatformToolset修改为v140_xp。

2024.12.26
解决主控程序概率性崩溃的问题，增强主控程序运行的稳定性。本人未进行广泛测试，不保证彻底根治，但稳定性有明显改观。
fix: client threads number excceeding bug
fix: #19 the CBuffer causing server crash
fix: showing the wrong host quantity in status bar

2024.12.27
solve some issues according to code analysis result
reorg: Move commands to common/commands.h
此次提交的重点是将重复代码移动到公共目录，减少代码的冗余。

2024.12.28
1.修改了注册指令内容，新生成的主控程序和被控程序不能和以往的程序混用!! 预留了字段，以便未来之需。
2.解决客户端接收大数据包的问题! 主控程序增加显示被控端版本信息，以便实现针对老版本在线更新(仅限基于TestRun的服务)的能力。
在主控程序上面增加了显示被控端启动时间的功能，以便掌握被控端程序的稳定性。
3.完善生成服务程序的功能。

2024.12.29
增加显示被控程序"类型"的功能：如果被控程序为单个EXE则显示为"EXE"，如果被控程序为EXE调用动态库形式，则显示为"DLL".
当前，只有类型为DLL的服务支持在线升级。本次提交借机对前一个更新中的"预留字段"进行了验证。

在动态链接库中增加导出函数Run，以便通过rundll32.exe调用动态链接库。这种形式也是支持在线对DLL进行升级的。

2024.12.31
生成服务时增加加密选项，当前支持XOR加密。配合使用解密程序来加载加密后的服务。

2025.01.12
修复被控程序关于远程桌面相关可能的2处问题（#28 #29）。增加对主控端列表窗口的排序功能（#26 #27），以便快速定位窗口、服务或进程。

发布一个运行**非常稳定**的版本v1.0.6，该版本不支持在较老的Windows XP系统运行（注：VS2019及以后版本已不支持XP工具集，为此需要更早的VS）。
您可以从GitHub下载最新的Release，也可以clone该项目在相关目录找到。如果杀毒软件报告病毒，这是正常现象，请信任即可，或者您可以亲自编译。

2025.02.01

参考[Gh0st](https://github.com/yuanyuanxiang/Gh0st/pull/2)，增加键盘记录功能。实质上就是拷贝如下四个文件：

*KeyboardManager.h、KeyboardManager.cpp、KeyBoardDlg.h、KeyBoardDlg.cpp*

# 沟通反馈

QQ：962914132

联系方式： [Telegram](https://t.me/doge_grandfather), [Email](mailto:yuanyuanxiang163@gmail.com), [LinkedIn](https://www.linkedin.com/in/wishyuanqi)

问题报告： [Issues](https://github.com/yuanyuanxiang/SimpleRemoter/issues) 

欢迎提交： [Merge requests](https://github.com/yuanyuanxiang/SimpleRemoter/pulls)
