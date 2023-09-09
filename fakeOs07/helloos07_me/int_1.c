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

//���Ϊ���̶˿�
#define PORT_KEYDAT		0x0060

//��Ӽ����ж�
void inthandler21(int *esp)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	unsigned char data, s[4];
	//֪ͨPIC IRQ-01 �Ѿ��������
	io_out8(PIC0_OCW2, 0x61);
	data = io_in8(PORT_KEYDAT);

	sprintf(s, "%02X", data);
	boxfill8(binfo->vram, binfo->scrnx, COL8_000000, 0, 16, 15, 31);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);

	return;
}

//�������ж�
void inthandler2c(int *esp)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	boxfill8(binfo->vram, binfo->scrnx, COL8_000000, 0, 0, 32 * 8 - 1, 15);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, "INT 2C (IRQ-12) : mouse");
	for (;;) {
		io_hlt();
	}
}
