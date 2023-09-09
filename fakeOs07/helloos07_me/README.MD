Day 7	FIFO与鼠标控制

一、获取按键编码
	int.c	+> 修改 inthandler21()  将按键编码在屏幕上显示
	int.c	+> 添加 #define PORT_KEYDAT		0x0060

二、加快中断处理
	int.c	+> 添加键盘缓冲 struct KEYBUF
	int.c	+> 修改键盘中断 inthandler
	

三、添加FIFO缓冲区
	fifo.c
	Makefile	+> 首行添加 fifo.obj参数


