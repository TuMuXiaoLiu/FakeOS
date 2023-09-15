//定时器相关
/*
	IRQ0的中断周期变化
	AL = 0x34:OUT(0x34,AL)
	AL = 中断周期的低8位; OUT(0x40,AL)
	AL = 中断周期的高8位; OUT(0x40,AL)

	PS:如果指定周期为0，会被看作时指定为65536，实际的中断产生的频率时单位时间时钟周期数(主频)/设定的数值
*/
#include	"bootpack.h"

#define		PIT_CTRL		0x0043
#define		PIT_CNT0		0x0040

//声明定时器控制器对象
struct TIMERCTL timerctl;

void init_pit(void)
{
	io_out8(PIT_CTRL, 0x34);
	io_out8(PIT_CNT0, 0x9c);
	io_out8(PIT_CNT0, 0x2e);
	//将计数设为0
	timerctl.count  = 0;
	return;
}

//中断处理
void inthandler20(int *esp)
{
	//把IRQ-0信号接受完了的信息通知PIC
	io_out8(PIC0_OCW2, 0x60);
	//中断一次就计数一次
	timerctl.count++;
	return;
}
