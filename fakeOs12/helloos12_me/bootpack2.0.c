//要使用sprintg函数 必须在开头加上 #include

//添加函数引用文件
#include "bootpack.h"
#include <stdio.h>

//程序入口 相当于Java中的Main函数
void HariMain(void)
{
	//声明BOOTINFO的对象并赋值 指针赋值
	struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
	//用于存放输出内容 鼠标 以及鼠标键盘的缓冲区
	char s[40], keybuf[32], mousebuf[128];
	struct  MOUSE_DEC mdec;
	//鼠标坐标
	int mx, my, i;
	//定义用于高速计算器的
	//unsigned int count = 0;


	//声明内存总数
	unsigned int memtotal;
	//声明内存管理对象
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;


	//图层管理
	struct SHTCTL *shtctl;
	//背景板  鼠标   窗口
	struct SHEET *sht_back, *sht_mouse, *sht_win;
	//背景板缓冲  鼠标缓冲
	unsigned char *buf_back, buf_mouse[256], *buf_win;


	//初始化GDTIDT
	init_gdtidt();
	//初始化PIC
	init_pic();
	//解除 禁止中断 在IDT初始化时所有中断都会被禁止，需要在此解除所有的禁止中断 让CPU接受外部设备指令
	io_sti();


	//接受外部设备 鼠标键盘
	//设置缓冲区
	fifo8_init(&keyfifo, 32, keybuf);
	fifo8_init(&mousefifo, 128, mousebuf);


	//初始化定时器
	init_pit();

	//修改PIT和PIC1和键盘的中断，许可为11111000
	io_out8(PIC0_IMR, 0xf8);

	//修改PIC的IMR，以便接受来自鼠标键盘的中断
	//io_out8(PIC0_IMR, 0xf9); /* 許可(11111001) */
	//修改PIC的IMR，以便接受来自鼠标的中断
	io_out8(PIC1_IMR, 0xef); /* 許可(11101111) */


	//初始化键盘
	init_keyboard();
	enable_mouse(&mdec);
	//进行内存管理
	memtotal = memtest( 0x00400000, 0xbfffffff);
	memman_init(memman);
	// 0x00001000 - 0x0009efff
	memman_free(memman, 0x00001000, 0x0009e000);
	memman_free(memman, 0x00400000, memtotal - 0x00400000);


	//设定调色板
	init_palette();
	//调用画界面方法并入参-> 启动图层管理
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
	//获取背景板图层对象
	sht_back = sheet_alloc(shtctl);
	//获取鼠标图层对象
	sht_mouse = sheet_alloc(shtctl);
	//获取窗口图层对象
	sht_win = sheet_alloc(shtctl);

	//背景板缓冲
	buf_back = (unsigned char *)memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	//窗口缓冲
	buf_win = (unsigned char *)memman_alloc_4k(memman, 160 * 52);

	//背景板没有透明色
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);
	//鼠标的透明色号99
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
	//窗口
	sheet_setbuf(sht_win, buf_win, 160, 52, -1);

	//初始化屏幕
	init_screen8(buf_back, binfo->scrnx, binfo->scrny);
	//初始化鼠标  背景色号99   替代原87行代码
	init_mouse_cursor8(buf_mouse, 99);
	//调用窗口函数
	make_window8(buf_win, 160, 52, "counter");

	//背景板位置
	sheet_slide(sht_back, 0, 0);
	
	//计算画面的中心坐标
	mx = (binfo->scrnx - 16) / 2; 
	my = (binfo->scrny - 28 - 16) / 2;
	//显示并勾勒鼠标
	sheet_slide(sht_mouse, mx, my);
	//显示窗口
	sheet_slide(sht_win, 80, 72);

	//设置背景板图层高度
	sheet_updown(sht_back, 0);
	//设置窗口图层高度
	sheet_updown(sht_win, 1);
	//设置鼠标图层高度
	sheet_updown(sht_mouse, 2);


	//打印鼠标坐标
	sprintf(s, "(%d, %d)", mx, my);
	//
	putfonts8_asc(buf_back, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
	
	//打印 内存大小以及空余空间
	sprintf(s, "memory %dMB  free : %dKB", memtotal / (1024*1024), memman_total(memman) / 1024);
	putfonts8_asc(buf_back, binfo->scrnx, 0, 32, COL8_FFFFFF, s);
	//刷新文字
	sheet_refresh(sht_back, 0, 0, binfo->scrnx, 48);
	

	for (;;) 
	{
		//高速计数器
		//count++;
		sprintf(s, "%010d", timerctl.count);
		boxfill8(buf_win, 160, COL8_C6C6C6, 40, 28, 119, 43);
		putfonts8_asc(buf_win, 160, 40, 28, COL8_000000, s);

		//刷新画面
		sheet_refresh(sht_win, 40, 28, 120, 44);
	
		io_cli();
		if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0) 
		{
			//不做HLT
			io_sti();
		} else 
		{
			if (fifo8_status(&keyfifo) != 0) {
				i = fifo8_get(&keyfifo);
				io_sti();
				sprintf(s, "%02X", i);
				boxfill8(buf_back, binfo->scrnx, COL8_008484,  0, 16, 15, 31);
				putfonts8_asc(buf_back, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
				//刷新文字
				sheet_refresh(sht_back, 0, 16, 16, 32);
			} else if (fifo8_status(&mousefifo) != 0) {
				i = fifo8_get(&mousefifo);
				io_sti();
				//鼠标的三个字节数据都齐了，显示出来
				if (mouse_decode(&mdec, i) != 0)
				{
					//显示鼠标移动数据
					sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
					if ((mdec.btn & 0x01) != 0) { s[1] = 'L'; }
					if ((mdec.btn & 0x02) != 0) { s[3] = 'R'; }
					if ((mdec.btn & 0x04) != 0) { s[2] = 'C'; }
					//鼠标指针的移动
					boxfill8(buf_back, binfo->scrnx, COL8_008484, 32, 16, 32+15*8-1, 31);
					putfonts8_asc(buf_back, binfo->scrnx, 31, 16, COL8_FFFFFF, s);

					//刷新文字
					sheet_refresh(sht_back, 32, 16, 32+15*8, 32);

					mx += mdec.x;
					my += mdec.y;
					//控制鼠标的边界范围
					if (mx < 0) { mx = 0; }
					if (my < 0) { my = 0; }

					if (mx > binfo->scrnx -1) { mx = binfo->scrnx -1; }
					if (my > binfo->scrny -1) { my = binfo->scrny -1; }

					sprintf(s, "(%3d, %3d)", mx, my);
					//隐藏坐标
					boxfill8(buf_back, binfo->scrnx, COL8_008484, 0, 0, 79, 15);
					//显示坐标
					putfonts8_asc(buf_back, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
					//鼠标移动动作显示
					//刷新文字
					sheet_refresh(sht_back, 0, 0, 80, 16);
					sheet_slide(sht_mouse, mx, my);
				}
			}
		}
	}
}


//创建窗口的函数
void make_window8(unsigned char *buf, int xsize, int ysize, char *title)
{
	static char closebtn[14][16] = {
		"OOOOOOOOOOOOOOO@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQQQ@@QQQQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"O$$$$$$$$$$$$$$@",
		"@@@@@@@@@@@@@@@@"
	};

	int x, y;
	char c;
	boxfill8(buf, xsize, COL8_C6C6C6, 0, 0, xsize - 1, 0 );
	boxfill8(buf, xsize, COL8_FFFFFF, 1, 1, xsize - 2, 1 );
	boxfill8(buf, xsize, COL8_C6C6C6, 0, 0, 0, ysize - 1);
	boxfill8(buf, xsize, COL8_FFFFFF, 1, 1, 1, ysize - 2);
	boxfill8(buf, xsize, COL8_848484, xsize - 2, 1, xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, xsize - 1, 0, xsize - 1, ysize - 1);
	boxfill8(buf, xsize, COL8_C6C6C6, 2, 2, xsize - 3, ysize - 3);
	boxfill8(buf, xsize, COL8_000084, 3, 3, xsize - 4, 20 );
	boxfill8(buf, xsize, COL8_848484, 1, ysize - 2, xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, 0, ysize - 1, xsize - 1, ysize - 1);
	putfonts8_asc(buf, xsize, 24, 4, COL8_FFFFFF, title);

	for (y = 0; y < 14; y++) {
		for (x = 0; x < 16; x++) {
			c = closebtn[y][x];
			if (c == '@') {
				c = COL8_000000;
			} else if (c == '$') {
				c = COL8_848484;
			} else if (c == 'Q') {
				c = COL8_C6C6C6;
			} else {
				c = COL8_FFFFFF;
			}
			buf[(5 + y) * xsize + (xsize - 21 + x)] = c;
		}
	}
	return;
}

