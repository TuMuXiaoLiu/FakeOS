//���̿��ƴ���
#include "bootpack.h"

//�������̻������
struct FIFO32 *keyfifo;
//???
int keydata0;

//�����ж�
void inthandler21(int *esp)
{
	int data;
	// ֪ͨPIC IRQ-01 �Ѿ�������� 
	io_out8(PIC0_OCW2, 0x61);	
	data = io_in8(PORT_KEYDAT);
	fifo32_put(keyfifo, data + keydata0);
	return;
}

#define PORT_KEYSTA				0x0064
#define KEYSTA_SEND_NOTREADY	0x02
#define KEYCMD_WRITE_MODE		0x60
#define KBC_MODE				0x47

void wait_KBC_sendready(void)
{
	/* �ȴ����̿��Ƶ�·׼����� */
	for (;;) {
		if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
			break;
		}
	}
	return;
}


//��ʼ������
void init_keyboard(struct FIFO32 *fifo, int data0)
{
	//��FIFO����������Ϣ���浽ȫ�ֱ�����
	keyfifo = fifo;
	keydata0 = data0;
	/* ��ʼ�����̿��Ƶ�· */
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE);
	return;
}