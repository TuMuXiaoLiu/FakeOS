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

void init_pit(void) {
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
	for (i = 0; i < MAX_TIMER; i++)  {
		timerctl.timers0[i].flags = 0;
	}
	return;
}

//分配定时器
struct TIMER *timer_alloc(void) {
	int i;
	for (i = 0; i < MAX_TIMER; i++) {
		if (timerctl.timers0[i].flags == 0) {	
			//标记为已配置
			timerctl.timers0[i].flags = TIMER_FLAGS_ALLOC;
			return &timerctl.timers0[i];
		}
	}
	//没有空余定时器
	return 0;
}

//释放定时器
void timer_free(struct TIMER *timer) {
	//标记为未使用
	timer->flags = 0;
	return;
}

//初始化定时器
void timer_init(struct TIMER *timer, struct FIFO32 *fifo, int data) {
	timer->fifo = fifo;
	timer->data = data;
	return;
}

//设置计时器时间
void timer_settimer(struct TIMER *timer, unsigned int timeout) {
	int e;
	struct TIMER *t, *s;
	timer->timeout = timeout + timerctl.count;
	timer->flags = TIMER_FLAGS_USING;
	e = io_load_eflags();
	io_cli();
	timerctl.using++;
	if (timerctl.using == 1){
		//处于运行状态的定时器只有这一个时
		timerctl.t0 = timer;
		//没有下一个
		timer->next = 0;
		timerctl.next = timer->timeout;
		io_store_eflags(e);
		return;
	}
	t = timerctl.t0;
	//插入最前面的情况下
	if (timer->timeout <= t->timeout){
		timerctl.t0 = timer;
		//下一个是t
		timer->next = t;
		timerctl.next = timer->timeout;
		io_store_eflags(e);
		return;
	}
	//搜寻插入位置
	for (; ; ){
		s = t;
		t = t->next;
		if (t == 0){
			break;
		}
		if (timer->timeout <= t->timeout){
			s->next = timer;
			timer->next = t;
			io_store_eflags(e);
			return;
		}
	}
	//插入到最后面
	s->next = timer;
	timer->next = 0;
	io_store_eflags(e);
	return;
}

//中断处理
void inthandler20(int *esp) {
	struct TIMER *timer;
	//把IRQ-0信号接受完了的信息通知PIC
	io_out8(PIC0_OCW2, 0x60);
	//中断一次就计数一次  使用count进行计时
	timerctl.count++;
	//没到超时
	if (timerctl.next > timerctl.count) {
		return;
	}
	//先把最前面的地址赋值给timer
	timer = timerctl.t0;
	for (; ; ) {
		//所有的定时器都处于活动中，所以不确认flags
		if (timer->timeout > timerctl.count) {
			break;
		}
		//超时
		timer->flags = TIMER_FLAGS_ALLOC;
		fifo32_put(timer->fifo, timer->data);
		//赋值下一个定时器地址
		timer = timer->next;
	}
	//新移位
	timerctl.t0 = timer;
	//timerctl.next 的设定
	timerctl.next = timer->timeout;
	return;
}


