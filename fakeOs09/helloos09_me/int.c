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

#define PORT_KEYDAT		0x0060

/*PIC0中断的不完整策略
 这个中断在Athlon64X2上通过芯片组提供的便利，只需执行一次 
 这个中断只是接收，不执行任何操作 
 为什么不处理？
	→  因为这个中断可能是电气噪声引发的、只是处理一些重要的情况。*/
void inthandler27(int *esp)
{
	//通知PIC的IRQ-07
	io_out8(PIC0_OCW2, 0x67);
	return;
}
