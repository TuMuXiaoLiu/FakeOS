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

//������ʱ������������
struct TIMERCTL timerctl;

void init_pit(void)
{
	io_out8(PIT_CTRL, 0x34);
	io_out8(PIT_CNT0, 0x9c);
	io_out8(PIT_CNT0, 0x2e);
	//��������Ϊ0
	timerctl.count  = 0;
	//��ʱ���������Ϊ0
	timerctl.timeout = 0;
	return;
}

//�жϴ���
void inthandler20(int *esp)
{
	//��IRQ-0�źŽ������˵���Ϣ֪ͨPIC
	io_out8(PIC0_OCW2, 0x60);
	//�ж�һ�ξͼ���һ��
	timerctl.count++;
	//����Ѿ��趨�˳�ʱ
	if (timerctl.timeout > 0)
	{
		timerctl.timeout--;

		//��ʱʱ�䵽
		if (timerctl.timeout == 0)
		{
			//��Ҫ��������ݷ��뻺�� 
			fifo8_put(timerctl.fifo, timerctl.data);
		}
	}
	return;
}

//���ü�ʱ��ʱ��
void settimer(unsigned int timeout, struct FIFO8 *fifo, unsigned char data)
{
	int eflags;
	eflags = io_load_eflags;
	//���ж�
	io_cli;
	timerctl.timeout = timeout;
	timerctl.fifo = fifo;
	timerctl.data = data;
	//�ָ��ж�
	io_store_eflags(eflags);
	return;
}
