//要使用sprintg函数 必须在开头加上 #include

//添加函数引用文件
#include "bootpack.h"
#include <stdio.h>

//引用外部对象 鼠标键盘
extern struct FIFO8 keyfifo, mousefifo;
void init_keyboard(void);

//声明鼠标类
struct MOUSE_DEC
{
	//buf 用于存放数据 phase 用于记录读取了多少数据
	unsigned char buf[3], phase;
	//x,y 用于存放鼠标的移动信息， btn 用于存放鼠标的按键信息
	int x, y, btn;
};

//鼠标激活
void enable_mouse(struct MOUSE_DEC *mdec);
//鼠标操作
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat);


//程序入口 相当于Java中的Main函数
void HariMain(void)
{
	//声明BOOTINFO的对象并赋值 指针赋值
	struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
	//用于存放输出内容 鼠标 以及鼠标键盘的缓冲区
	char s[40], mcursor[256], keybuf[32], mousebuf[128];

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

	//设定调色板
	init_palette();
	//调用画界面方法并入参
	init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);
	

	//显示鼠标
	int mx, my;
	//计算画面的中心坐标
	mx = (binfo->scrnx - 16) / 2; 
	my = (binfo->scrny - 28 - 16) / 2;
	//显示并勾勒鼠标
	init_mouse_cursor8(mcursor, COL8_008484);
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
	//打印鼠标坐标
	sprintf(s, "(%d, %d)", mx, my);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);


	struct  MOUSE_DEC mdec;
	enable_mouse(&mdec);

	//
	int i;
	for (;;) {
		io_cli();
		if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0) {
			io_stihlt();
		} else {
			if (fifo8_status(&keyfifo) != 0) {
				i = fifo8_get(&keyfifo);
				io_sti();
				sprintf(s, "%02X", i);
				boxfill8(binfo->vram, binfo->scrnx, COL8_008484,  0, 16, 15, 31);
				putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
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
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 32, 16, 32+15*8-1, 31);
					putfonts8_asc(binfo->vram, binfo->scrnx, 31, 16, COL8_FFFFFF, s);
					//鼠标指针的移动
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, mx, my, mx+15, my+15);

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
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 0, 79, 15);
					//显示坐标
					putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
					//鼠标移动动作显示
					putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
				}
			}
		}
	}
}


#define PORT_KEYDAT				0x0060
#define PORT_KEYSTA				0x0064
#define PORT_KEYCMD				0x0064
#define KEYSTA_SEND_NOTREADY	0x02
#define KEYCMD_WRITE_MODE		0x60
#define KBC_MODE				0x47

void wait_KBC_sendready(void)
{
	/* 等待键盘控制电路准备完毕 */
	for (;;) {
		if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
			break;
		}
	}
	return;
}

void init_keyboard(void)
{
	/* 初始化键盘控制电路 */
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE);
	return;
}

#define KEYCMD_SENDTO_MOUSE		0xd4
#define MOUSECMD_ENABLE			0xf4

struct MOUSE_DEC *mdec;

void enable_mouse(struct MOUSE_DEC *mdec)
{
	 //激活鼠标 
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
	//顺利的话，键盘控制器会返回ACK(0xfa)
	return; 
}

//鼠标操作
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat){
		//进入到等待鼠标的 0xfa 的状态
		if (mdec->phase == 0)
		{
			if (dat == 0xfa)
			{
				mdec->phase = 1;
			}
			return 0;
		}

		//等待鼠标的第一个字节
		if (mdec->phase == 1)
		{
			//如果第一个字节正确
			if ((dat & 0xc8) == 0x08)
			{
				mdec->buf[0] = dat;
				mdec->phase = 2;
			}
			
			return 0;
		}

		//等待鼠标的第二个字节
		if (mdec->phase == 2)
		{
			mdec->buf[1] = dat;
			mdec->phase = 3;
			return  0;
		}

		//等待鼠标的第三个字节
		if (mdec->phase == 3)
		{	
			//获取并存入数据
			mdec->buf[2] = dat;
			mdec->phase = 1;
			mdec->btn = mdec->buf[0] & 0x07;
			mdec->x = mdec->buf[1];
			mdec->y = mdec->buf[2];

			if ((mdec->buf[0] & 0x10) != 0)
			{
				mdec->x |= 0xffffff00;
			}

			if ((mdec->buf[0] & 0x20) != 0)
			{
				mdec->y |= 0xffffff00; 
			}
			//鼠标的y方向与画面符号相反
			mdec->y = - mdec->y;
			//读取完成 返回1
			return 1;
		}
		
		//一般情况下不会执行到这一步
		return -1;
}









