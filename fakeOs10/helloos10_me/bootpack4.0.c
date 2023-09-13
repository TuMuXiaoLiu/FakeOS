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
	char s[40], mcursor[256], keybuf[32], mousebuf[128];
	struct  MOUSE_DEC mdec;
	//鼠标坐标
	int mx, my, i;


	//声明内存总数
	unsigned int memtotal;
	//声明内存管理对象
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;


	//图层管理
	struct SHTCTL *shtctl;
	//背景板  鼠标
	struct SHEET *sht_back, *sht_mouse;
	//背景板缓冲  鼠标缓冲
	unsigned char *buf_back, buf_mouse[256];


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
	//修改PIC的IMR，以便接受来自鼠标键盘的中断
	io_out8(PIC0_IMR, 0xf9); /* 許可(11111001) */
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
	//init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
	//获取背景板图层对象
	sht_back = sheet_alloc(shtctl);
	//获取鼠标图层对象
	sht_mouse = sheet_alloc(shtctl);
	//背景板缓冲
	buf_back = (unsigned char *)memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	//背景板没有透明色
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);
	//鼠标的透明色号99
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
	//初始化屏幕
	init_screen8(buf_back, binfo->scrnx, binfo->scrny);
	//初始化鼠标  背景色号99   替代原87行代码
	init_mouse_cursor8(buf_mouse, 99);
	//背景板位置
	sheet_slide(shtctl, sht_back, 0, 0);
	
	//计算画面的中心坐标
	mx = (binfo->scrnx - 16) / 2; 
	my = (binfo->scrny - 28 - 16) / 2;
	//显示并勾勒鼠标
	//init_mouse_cursor8(mcursor, COL8_008484);
	sheet_slide(shtctl, sht_mouse, mx, my);
	//设置背景板图层高度
	sheet_updown(shtctl, sht_back, 0);
	//设置鼠标图层高度
	sheet_updown(shtctl, sht_mouse, 1);

	//打印鼠标坐标
	sprintf(s, "(%d, %d)", mx, my);
	//
	putfonts8_asc(buf_back, binfo->scrnx, 0, 0, COL8_FFFFFF, s);

	
	//打印 内存大小以及空余空间
	sprintf(s, "memory %dMB  free : %dKB", memtotal / (1024*1024), memman_total(memman) / 1024);
	putfonts8_asc(buf_back, binfo->scrnx, 0, 32, COL8_FFFFFF, s);
	//刷新文字
	sheet_refresh(shtctl, sht_back, 0, 0, binfo->scrnx, 48);
	

	for (;;) 
	{
		io_cli();
		if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0) 
		{
			io_stihlt();
		} else 
		{
			if (fifo8_status(&keyfifo) != 0) {
				i = fifo8_get(&keyfifo);
				io_sti();
				sprintf(s, "%02X", i);
				boxfill8(buf_back, binfo->scrnx, COL8_008484,  0, 16, 15, 31);
				putfonts8_asc(buf_back, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
				//刷新文字
				sheet_refresh(shtctl, sht_back, 0, 16, 16, 32);
			} else if (fifo8_status(&mousefifo) != 0) {
				i = fifo8_get(&mousefifo);
				io_sti();
				//鼠标的三个字节数据都齐了，显示出来
				if (mouse_decode(&mdec, i) != 0)
				{
					//显示鼠标移动数据
					sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
					if ((mdec.btn & 0x01) != 0)
					{
						s[1] = 'L';
					}
	
					if ((mdec.btn & 0x02) != 0)
					{
						s[3] = 'R';
					}

					if ((mdec.btn & 0x04) != 0)
					{
						s[2] = 'C';
					}
					boxfill8(buf_back, binfo->scrnx, COL8_008484, 32, 16, 32+15*8-1, 31);
					putfonts8_asc(buf_back, binfo->scrnx, 31, 16, COL8_FFFFFF, s);
					//鼠标指针的移动
					//刷新文字
					sheet_refresh(shtctl, sht_back, 32, 16, 32+15*8, 32);

					mx += mdec.x;
					my += mdec.y;
					//控制鼠标的边界范围
					if (mx < 0)
					{
						mx = 0;
					}
					if (my < 0)
					{
						my = 0;
					}
					if (mx > binfo->scrnx -16)
					{
						mx = binfo->scrnx -16;
					}
					if (my > binfo->scrny -16)
					{
						my = binfo->scrny -16;
					}
					sprintf(s, "(%3d, %3d)", mx, my);
					//隐藏坐标
					boxfill8(buf_back, binfo->scrnx, COL8_008484, 0, 0, 79, 15);
					//显示坐标
					putfonts8_asc(buf_back, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
					//鼠标移动动作显示
					//刷新文字
					sheet_refresh(shtctl, sht_back, 0, 0, 80, 16);
					sheet_slide(shtctl, sht_mouse, mx, my);
				}
			}
		}
	}
}


