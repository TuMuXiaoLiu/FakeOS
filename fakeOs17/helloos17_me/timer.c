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

void init_pit(void) {
	int i;
	struct TIMER *t;
	io_out8(PIT_CTRL, 0x34);
	io_out8(PIT_CNT0, 0x9c);
	io_out8(PIT_CNT0, 0x2e);
	//��������Ϊ0
	timerctl.count  = 0;
	//�����ж�ʱ������Ϊδʹ��
	for (i = 0; i < MAX_TIMER; i++)  {
		timerctl.timers0[i].flags = 0;
	}
	//����һ����ʱ��
	t = timer_alloc();
	t->timeout = 0xffffffff;
	//��Ϊ������
	t->flags = TIMER_FLAGS_USING;
	//ĩβ
	t->next = 0;
	//��Ϊ�������ҽ������һ(�ڱ�),���Ծ�����ǰ��
	timerctl.t0 = t;
	//��Ϊֻ���ڱ���������һ����ʱ������ڱ��ĵ�
	timerctl.next = 0xffffffff;
	return;
}

//���䶨ʱ��
struct TIMER *timer_alloc(void) {
	int i;
	for (i = 0; i < MAX_TIMER; i++) {
		if (timerctl.timers0[i].flags == 0) {	
			//���Ϊ������
			timerctl.timers0[i].flags = TIMER_FLAGS_ALLOC;
			return &timerctl.timers0[i];
		}
	}
	//û�п��ඨʱ��
	return 0;
}

//�ͷŶ�ʱ��
void timer_free(struct TIMER *timer) {
	//���Ϊδʹ��
	timer->flags = 0;
	return;
}

//��ʼ����ʱ��
void timer_init(struct TIMER *timer, struct FIFO32 *fifo, int data) {
	timer->fifo = fifo;
	timer->data = data;
	return;
}

//���ü�ʱ��ʱ��
void timer_settime(struct TIMER *timer, unsigned int timeout) {
	int e;
	struct TIMER *t, *s;
	timer->timeout = timeout + timerctl.count;
	timer->flags = TIMER_FLAGS_USING;
	e = io_load_eflags();
	io_cli();
	t = timerctl.t0;
	//������ǰ��������
	if (timer->timeout <= t->timeout){
		timerctl.t0 = timer;
		//��һ����t
		timer->next = t;
		timerctl.next = timer->timeout;
		io_store_eflags(e);
		return;
	}
	//��Ѱ����λ��
	for (; ; ){
		s = t;
		t = t->next;
		//���뵽s-t֮��
		if (timer->timeout <= t->timeout){
			s->next = timer;
			timer->next = t;
			io_store_eflags(e);
			return;
		}
	}
}

//�жϴ���
void inthandler20(int *esp) {
	struct TIMER *timer;
	char ts = 0;
	//��IRQ-0�źŽ������˵���Ϣ֪ͨPIC
	io_out8(PIC0_OCW2, 0x60);
	//�ж�һ�ξͼ���һ��  ʹ��count���м�ʱ
	timerctl.count++;
	//û����ʱ
	if (timerctl.next > timerctl.count) {
		return;
	}
	//�Ȱ���ǰ��ĵ�ַ��ֵ��timer
	timer = timerctl.t0;
	for (; ; ) {
		//���еĶ�ʱ�������ڻ�У����Բ�ȷ��flags
		if (timer->timeout > timerctl.count) {
			break;
		}
		//��ʱ
		timer->flags = TIMER_FLAGS_ALLOC;
		if (timer != task_timer){
			fifo32_put(timer->fifo, timer->data);
		} else {
			//mt_timer ��ʱ
			ts = 1;
		}
		//��ֵ��һ����ʱ����ַ
		timer = timer->next;
	}
	//����λ
	timerctl.t0 = timer;
	//timerctl.next ���趨
	timerctl.next = timer->timeout;
	if (ts != 0){
		task_switch();
	}
	return;
}


