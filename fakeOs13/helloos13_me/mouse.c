//鼠标控制代码
#include "bootpack.h"

struct FIFO32 *mousefifo;
int mousedata0;

//鼠标中断
void inthandler2c(int *esp){
	int data;
	// 通知PIC IRQ-12 已经受理完毕
	io_out8(PIC1_OCW2, 0x64);
	// 通知PIC IRQ-02 已经受理完毕
	io_out8(PIC0_OCW2, 0x62);
	data = io_in8(PORT_KEYDAT);
	fifo32_put(mousefifo, data+mousedata0);
	return;
}

#define KEYCMD_SENDTO_MOUSE		0xd4
#define MOUSECMD_ENABLE			0xf4


void enable_mouse(struct FIFO32 *fifo, int data0, struct MOUSE_DEC *mdec){
	//将FIFO的数据存入全局
	mousefifo = fifo;
	mousedata0 = data0;
	 //激活鼠标 
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
	//顺利的话，键盘控制器会返回ACK(0xfa)
	//等待0xfa的阶段
	mdec->phase = 0;
	return; 
}


//鼠标操作
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat){
		//进入到等待鼠标的 0xfa 的状态
		if (mdec->phase == 0) {
			if (dat == 0xfa) {
				mdec->phase = 1;
			}
			return 0;
		}

		//等待鼠标的第一个字节
		if (mdec->phase == 1) {
			//如果第一个字节正确
			if ((dat & 0xc8) == 0x08) {
				mdec->buf[0] = dat;
				mdec->phase = 2;
			}
			return 0;
		}

		//等待鼠标的第二个字节
		if (mdec->phase == 2) {
			mdec->buf[1] = dat;
			mdec->phase = 3;
			return  0;
		}

		//等待鼠标的第三个字节
		if (mdec->phase == 3) {	
			//获取并存入数据
			mdec->buf[2] = dat;
			mdec->phase = 1;
			mdec->btn = mdec->buf[0] & 0x07;
			mdec->x = mdec->buf[1];
			mdec->y = mdec->buf[2];

			if ((mdec->buf[0] & 0x10) != 0) {
				mdec->x |= 0xffffff00;
			}

			if ((mdec->buf[0] & 0x20) != 0) {
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


















