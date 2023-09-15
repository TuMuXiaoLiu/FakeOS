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
	//�����ж�ʱ������Ϊδʹ��
	for (i = 0; i < MAX_TIMER; i++)
	{
		timerctl.timer[i].flags = 0;
	}
	return;
}

//���䶨ʱ��
struct TIMER *timer_alloc(void)
{
	int i;
	for (i = 0; i < MAX_TIMER; i++)
	{
		if (timerctl.timer[i].flags == 0)
		{	
			//���Ϊ������
			timerctl.timer[i].flags = TIMER_FLAGS_ALLOC;
			return &timerctl.timer[i];
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
	timer->timeout = timeout;
	//���Ϊ��ʹ��
	timer->flags = TIMER_FLAGS_USING;
	return;
}

//�жϴ���
void inthandler20(int *esp)
{
	int i;
	//��IRQ-0�źŽ������˵���Ϣ֪ͨPIC
	io_out8(PIC0_OCW2, 0x60);
	//�ж�һ�ξͼ���һ��
	timerctl.count++;
	for (i = 0; i < MAX_TIMER; i++)
	{
		//�жϸü�ʱ���Ƿ���ʹ��״̬
		if (timerctl.timer[i].flags == TIMER_FLAGS_USING)
		{
			//ʱ��ݼ�
			timerctl.timer[i].timeout--;
			//����ö�ʱ����ʱ��ݼ�Ϊ0
			if (timerctl.timer[i].timeout == 0)
			{
				//����ʱ����Ϊ�ѷ��䡢�ȴ��´�ʹ��
				timerctl.timer[i].flags = TIMER_FLAGS_ALLOC;
				//���û��庯������Ӧ���ݴ���
				fifo8_put(timerctl.timer[i].fifo, timerctl.timer[i].data);
			}
		}
	}
	return;
}


