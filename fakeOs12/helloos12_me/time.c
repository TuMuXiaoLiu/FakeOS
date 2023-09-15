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
	//最初时没有定时器运行
	timerctl.next = 0xffffffff;
	timerctl.using = 0;
	//将所有定时器设置为未使用
	for (i = 0; i < MAX_TIMER; i++)
	{
		timerctl.timers0[i].flags = 0;
	}
	return;
}

//分配定时器
struct TIMER *timer_alloc(void)
{
	int i;
	for (i = 0; i < MAX_TIMER; i++)
	{
		if (timerctl.timers0[i].flags == 0)
		{	
			//标记为已配置
			timerctl.timers0[i].flags = TIMER_FLAGS_ALLOC;
			return &timerctl.timers0[i];
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
	/*
	timer->timeout = timeout + timerctl.count;
	//标记为已使用
	timer->flags = TIMER_FLAGS_USING;
	if (timerctl.next > timer->timeout)
	{
		timerctl.next = timer->timeout;
	}
	*/
	int e, i, j;
	timer->timeout = timeout + timerctl.count;
	//标记为已使用
	timer->flags = TIMER_FLAGS_USING;
	e = io_load_eflags();
	//禁中断
	io_cli();
	//搜索注册位置
	for (i = 0; i < timerctl.using; i++)
	{
		if (timerctl.timers[i]->timeout >= timer->timeout)
		{
			break;
		}
	}
	//i号后面所有都后移一位
	for (j = timerctl.using; j > i; j--)
	{
		timerctl.timers[j] = timerctl.timers[j-1];
	}
	timerctl.using++;
	//插入到空位上
	timerctl.timers[i] = timer;
	timerctl.next = timerctl.timers[0]->timeout;
	io_store_eflags(e);
	return;
}

//中断处理
void inthandler20(int *esp)
{
	/*int i;
	//把IRQ-0信号接受完了的信息通知PIC
	io_out8(PIC0_OCW2, 0x60);
	//中断一次就计数一次  使用count进行计时
	timerctl.count++;
	for (i = 0; i < MAX_TIMER; i++)
	{
		//判断该计时器是否在使用状态
		if (timerctl.timer[i].flags == TIMER_FLAGS_USING)
		{
			//如果timeout 等于或者大于count，则代表已超时 
			if (timerctl.timer[i].timeout <= timerctl.count)
			{
				//设置成已分配
				timerctl.timer[i].flags = TIMER_FLAGS_ALLOC;
				//放入流和数据
				fifo8_put(timerctl.timer[i].fifo, timerctl.timer[i].data);
			}
		}
	}*/

	/*int i;
	//把IRQ-0信号接受完了的信息通知PIC
	io_out8(PIC0_OCW2, 0x60);
	//中断一次就计数一次  使用count进行计时
	timerctl.count++;
	//没到超时
	if (timerctl.next > timerctl.count)
	{
		return;
	}
	timerctl.next = 0xffffffff;
	for (i = 0; i < MAX_TIMER; i++)
	{
		if (timerctl.timer[i].flags == TIMER_FLAGS_USING)
		{
			if (timerctl.timer[i].timeout <= timerctl.count)
			{
				//超时
				timerctl.timer[i].flags = TIMER_FLAGS_ALLOC;
				fifo8_put(timerctl.timer[i].fifo, timerctl.timer[i].data);
			}else{
				//未超时
				if (timerctl.next > timerctl.timer[i].timeout)
				{
					timerctl.next = timerctl.timer[i].timeout;
				}
			}	
		}
	}*/

	int i, j;
	//把IRQ-0信号接受完了的信息通知PIC
	io_out8(PIC0_OCW2, 0x60);
	//中断一次就计数一次  使用count进行计时
	timerctl.count++;
	//没到超时
	if (timerctl.next > timerctl.count)
	{
		return;
	}
	for (i = 0; i < timerctl.using; i++)
	{
		//所有的定时器都处于活动中，所以不确认flags
		if (timerctl.timers[i]->timeout > timerctl.count)
		{
			break;
		}
		//超时
		timerctl.timers[i]->flags = TIMER_FLAGS_ALLOC;
		fifo8_put(timerctl.timers[i]->fifo, timerctl.timers[i]->data);
	}
	//有一个定时器超时则其余进位
	timerctl.using -= i;
	for (j = 0; j < timerctl.using; j++)
	{
		timerctl.timers[j] = timerctl.timers[i + j];
	}
	if (timerctl.using > 0)
	{
		timerctl.next = timerctl.timers[0]->timeout;
	} else {
		timerctl.next = 0xffffffff;	
	}
	return;
}


