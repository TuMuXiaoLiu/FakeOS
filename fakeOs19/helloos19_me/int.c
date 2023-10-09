//PIC���

#include "bootpack.h"

//��ʼ��PIC
void init_pic(void)
{
	//��ֹ��PIC�����ж�
	io_out8(PIC0_IMR, 0xff);
	//��ֹ��PIC�����ж�
	io_out8(PIC1_IMR, 0xff);

	//��PIC
	//���ش���ģʽ��edge trigger mode��
	io_out8(PIC0_ICW1, 0x11);
	//IRQ0-7 �� INT20-27 ����
	io_out8(PIC0_ICW2, 0x20);
	//PIC1 �� IRQ2 ����
	io_out8(PIC0_ICW3, 1 << 2);
	//�޻�����ģʽ
	io_out8(PIC0_ICW4, 0x01);

	//��PIC
	//���ش���ģʽ��edge trigger mode��
	io_out8(PIC1_ICW1, 0x11);
	//IRQ8-15 �� INT28-2f ����
	io_out8(PIC1_ICW2, 0x28);
	//PIC1 �� IRQ2 ����
	io_out8(PIC1_ICW3, 2);
	//�޻�����ģʽ
	io_out8(PIC1_ICW4, 0x01);

	//11111011 PIC1����ȫ����ֹ
	io_out8(PIC0_IMR, 0xfb);
	//11111111 ��ֹ�����ж�
	io_out8(PIC1_IMR, 0xff);

	return;
}

#define PORT_KEYDAT		0x0060

/*PIC0�жϵĲ���������
 ����ж���Athlon64X2��ͨ��оƬ���ṩ�ı�����ֻ��ִ��һ�� 
 ����ж�ֻ�ǽ��գ���ִ���κβ��� 
 Ϊʲô������
	��  ��Ϊ����жϿ����ǵ������������ġ�ֻ�Ǵ���һЩ��Ҫ�������*/
void inthandler27(int *esp)
{
	//֪ͨPIC��IRQ-07
	io_out8(PIC0_OCW2, 0x67);
	return;
}
