//PIC相关
#include "bootpack.h"
//初始化PIC
void init_pic(void)
{
	//禁止主PIC所有中断
	io_out8(PIC0_IMR, 0xff);
	//禁止从PIC所有中断
	io_out8(PIC1_IMR, 0xff);

	//主PIC
	//边沿触发模式（edge trigger mode）
	io_out8(PIC0_ICW1, 0x11);
	//IRQ0-7 由 INT20-27 接收
	io_out8(PIC0_ICW2, 0x20);
	//PIC1 由 IRQ2 连接
	io_out8(PIC0_ICW3, 1 << 2);
	//无缓冲区模式
	io_out8(PIC0_ICW4, 0x01);

	//从PIC
	//边沿触发模式（edge trigger mode）
	io_out8(PIC1_ICW1, 0x11);
	//IRQ8-15 由 INT28-2f 接收
	io_out8(PIC1_ICW2, 0x28);
	//PIC1 由 IRQ2 连接
	io_out8(PIC1_ICW3, 2);
	//无缓冲区模式
	io_out8(PIC1_ICW4, 0x01);

	//11111011 PIC1以外全部禁止
	io_out8(PIC0_IMR, 0xfb);
	//11111111 禁止所有中断
	io_out8(PIC1_IMR, 0xff);

	return;
}

//理解为键盘端口
#define PORT_KEYDAT		0x0060

//添加键盘中断
void inthandler21(int *esp)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	unsigned char data, s[4];
	//通知PIC IRQ-01 已经受理完毕
	io_out8(PIC0_OCW2, 0x61);
	data = io_in8(PORT_KEYDAT);

	sprintf(s, "%02X", data);
	boxfill8(binfo->vram, binfo->scrnx, COL8_000000, 0, 16, 15, 31);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);

	return;
}

//添加鼠标中断
void inthandler2c(int *esp)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	boxfill8(binfo->vram, binfo->scrnx, COL8_000000, 0, 0, 32 * 8 - 1, 15);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, "INT 2C (IRQ-12) : mouse");
	for (;;) {
		io_hlt();
	}
}
