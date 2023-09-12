Day 9	内存管理

一、 整理源文件
		1.增加 keybord.c 和 mouse.c 文件 分别用于管理键盘和鼠标
		2.将 bootpack.c 和 int.c 中的关于键盘和鼠标的相关移动至 keyboard.c 和  mouse.c 中
		3.修改 bootpack.h, 添加函数声明
		4.修改 makefile, 添加 keybord 和 mouse 的 .obj 声明

二、内存容量检查
		1.修改 bootpack.c 增加 第142-184、7、63-66行代码
		2.修改 naskfunc.nas 增加第17-18、170-204行代码
		PS.运行结果和参考书有差异，参考书为32M内存，我这边只有1M内存。不知道原因是不是和我这边的配置相关。目前是在MacOS上跑Win虚拟机进行操作。

		问题已解决，原因为memtest函数的返回有误。该返回i 写成了1；

三、内存分配
		修改 bootpack.c 增加 第7-32、43-46、75-80、97-98、222-353 行代码
		PS.运行结果和参考书有差异，参考书为29304KB，自己这边为0KB
	
		问题已解决，原因时memman_free函数中负责定位的for后括号范围被挪至了最后
	
	



