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

//已配置状态
#define		TIMER_FLAGS_ALLOC		1
//定时器运行中
#define		TIMER_FLAGS_USING		2

//声明定时器控制器对象
struct TIMERCTL timerctl;

void init_pit(void)
{
	int i;
	io_out8(PIT_CTRL, 0x34);
	io_out8(PIT_CNT0, 0x9c);
	io_out8(PIT_CNT0, 0x2e);
	//将计数设为0
	timerctl.count  = 0;
	//将所有定时器设置为未使用
	for (i = 0; i < MAX_TIMER; i++)
	{
		timerctl.timer[i].flags = 0;
	}
	return;
}

//分配定时器
struct TIMER *timer_alloc(void)
{
	int i;
	for (i = 0; i < MAX_TIMER; i++)
	{
		if (timerctl.timer[i].flags == 0)
		{	
			//标记为已配置
			timerctl.timer[i].flags = TIMER_FLAGS_ALLOC;
			return &timerctl.timer[i];
		}
	}
	//没有空余定时器
	return 0;
}

//释放定时器
void timer_free(struct TIMER *timer)
{
	//标记为未使用
	timer->flags = 0;
	return;
}

//初始化定时器
void timer_init(struct TIMER *timer, struct FIFO *fifo, unsigned char data)
{
	timer->fifo = fifo;
	timer->data = data;
	return;
}

//设置计时器时间
void timer_settimer(struct TIMER *timer, unsigned int timeout)
{
	timer->timeout = timeout;
	//标记为已使用
	timer->flags = TIMER_FLAGS_USING;
	return;
}

//中断处理
void inthandler20(int *esp)
{
	int i;
	//把IRQ-0信号接受完了的信息通知PIC
	io_out8(PIC0_OCW2, 0x60);
	//中断一次就计数一次
	timerctl.count++;
	for (i = 0; i < MAX_TIMER; i++)
	{
		//判断该计时器是否在使用状态
		if (timerctl.timer[i].flags == TIMER_FLAGS_USING)
		{
			//时间递减
			timerctl.timer[i].timeout--;
			//如果该定时器的时间递减为0
			if (timerctl.timer[i].timeout == 0)
			{
				//将定时器设为已分配、等待下次使用
				timerctl.timer[i].flags = TIMER_FLAGS_ALLOC;
				//调用缓冲函数将相应数据存入
				fifo8_put(timerctl.timer[i].fifo, timerctl.timer[i].data);
			}
		}
	}
	return;
}


