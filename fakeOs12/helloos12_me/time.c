//��ʱ�����
/*
	IRQ0���ж����ڱ仯
	AL = 0x34:OUT(0x34,AL)
	AL = �ж����ڵĵ�8λ; OUT(0x40,AL)
	AL = �ж����ڵĸ�8λ; OUT(0x40,AL)

	PS:���ָ������Ϊ0���ᱻ����ʱָ��Ϊ65536��ʵ�ʵ��жϲ�����Ƶ��ʱ��λʱ��ʱ��������(��Ƶ)/�趨����ֵ
*/
#include	"bootpack.h"

#define		PIT_CTRL		0x0043
#define		PIT_CNT0		0x0040

//������״̬
#define		TIMER_FLAGS_ALLOC		1
//��ʱ��������
#define		TIMER_FLAGS_USING		2

//������ʱ������������
struct TIMERCTL timerctl;

void init_pit(void)
{
	int i;
	io_out8(PIT_CTRL, 0x34);
	io_out8(PIT_CNT0, 0x9c);
	io_out8(PIT_CNT0, 0x2e);
	//��������Ϊ0
	timerctl.count  = 0;
	//���ʱû�ж�ʱ������
	timerctl.next = 0xffffffff;
	timerctl.using = 0;
	//�����ж�ʱ������Ϊδʹ��
	for (i = 0; i < MAX_TIMER; i++)
	{
		timerctl.timers0[i].flags = 0;
	}
	return;
}

//���䶨ʱ��
struct TIMER *timer_alloc(void)
{
	int i;
	for (i = 0; i < MAX_TIMER; i++)
	{
		if (timerctl.timers0[i].flags == 0)
		{	
			//���Ϊ������
			timerctl.timers0[i].flags = TIMER_FLAGS_ALLOC;
			return &timerctl.timers0[i];
		}
	}
	//û�п��ඨʱ��
	return 0;
}

//�ͷŶ�ʱ��
void timer_free(struct TIMER *timer)
{
	//���Ϊδʹ��
	timer->flags = 0;
	return;
}

//��ʼ����ʱ��
void timer_init(struct TIMER *timer, struct FIFO *fifo, unsigned char data)
{
	timer->fifo = fifo;
	timer->data = data;
	return;
}

//���ü�ʱ��ʱ��
void timer_settimer(struct TIMER *timer, unsigned int timeout)
{
	/*
	timer->timeout = timeout + timerctl.count;
	//���Ϊ��ʹ��
	timer->flags = TIMER_FLAGS_USING;
	if (timerctl.next > timer->timeout)
	{
		timerctl.next = timer->timeout;
	}
	*/
	int e, i, j;
	timer->timeout = timeout + timerctl.count;
	//���Ϊ��ʹ��
	timer->flags = TIMER_FLAGS_USING;
	e = io_load_eflags();
	//���ж�
	io_cli();
	//����ע��λ��
	for (i = 0; i < timerctl.using; i++)
	{
		if (timerctl.timers[i]->timeout >= timer->timeout)
		{
			break;
		}
	}
	//i�ź������ж�����һλ
	for (j = timerctl.using; j > i; j--)
	{
		timerctl.timers[j] = timerctl.timers[j-1];
	}
	timerctl.using++;
	//���뵽��λ��
	timerctl.timers[i] = timer;
	timerctl.next = timerctl.timers[0]->timeout;
	io_store_eflags(e);
	return;
}

//�жϴ���
void inthandler20(int *esp)
{
	/*int i;
	//��IRQ-0�źŽ������˵���Ϣ֪ͨPIC
	io_out8(PIC0_OCW2, 0x60);
	//�ж�һ�ξͼ���һ��  ʹ��count���м�ʱ
	timerctl.count++;
	for (i = 0; i < MAX_TIMER; i++)
	{
		//�жϸü�ʱ���Ƿ���ʹ��״̬
		if (timerctl.timer[i].flags == TIMER_FLAGS_USING)
		{
			//���timeout ���ڻ��ߴ���count��������ѳ�ʱ 
			if (timerctl.timer[i].timeout <= timerctl.count)
			{
				//���ó��ѷ���
				timerctl.timer[i].flags = TIMER_FLAGS_ALLOC;
				//������������
				fifo8_put(timerctl.timer[i].fifo, timerctl.timer[i].data);
			}
		}
	}*/

	/*int i;
	//��IRQ-0�źŽ������˵���Ϣ֪ͨPIC
	io_out8(PIC0_OCW2, 0x60);
	//�ж�һ�ξͼ���һ��  ʹ��count���м�ʱ
	timerctl.count++;
	//û����ʱ
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
				//��ʱ
				timerctl.timer[i].flags = TIMER_FLAGS_ALLOC;
				fifo8_put(timerctl.timer[i].fifo, timerctl.timer[i].data);
			}else{
				//δ��ʱ
				if (timerctl.next > timerctl.timer[i].timeout)
				{
					timerctl.next = timerctl.timer[i].timeout;
				}
			}	
		}
	}*/

	int i, j;
	//��IRQ-0�źŽ������˵���Ϣ֪ͨPIC
	io_out8(PIC0_OCW2, 0x60);
	//�ж�һ�ξͼ���һ��  ʹ��count���м�ʱ
	timerctl.count++;
	//û����ʱ
	if (timerctl.next > timerctl.count)
	{
		return;
	}
	for (i = 0; i < timerctl.using; i++)
	{
		//���еĶ�ʱ�������ڻ�У����Բ�ȷ��flags
		if (timerctl.timers[i]->timeout > timerctl.count)
		{
			break;
		}
		//��ʱ
		timerctl.timers[i]->flags = TIMER_FLAGS_ALLOC;
		fifo8_put(timerctl.timers[i]->fifo, timerctl.timers[i]->data);
	}
	//��һ����ʱ����ʱ�������λ
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


