//Ҫʹ��sprintg���� �����ڿ�ͷ���� #include

//���Ӻ��������ļ�
#include "bootpack.h"
#include <stdio.h>

//������� �൱��Java�е�Main����
void HariMain(void)
{
	//����BOOTINFO�Ķ��󲢸�ֵ ָ�븳ֵ
	struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
	//���ڴ��������� ��� �Լ������̵Ļ�����
	char s[40], mcursor[256], keybuf[32], mousebuf[128];
	struct  MOUSE_DEC mdec;


	//��ʼ��GDTIDT
	init_gdtidt();
	//��ʼ��PIC
	init_pic();
	//��� ��ֹ�ж� ��IDT��ʼ��ʱ�����ж϶��ᱻ��ֹ����Ҫ�ڴ˽�����еĽ�ֹ�ж� ��CPU�����ⲿ�豸ָ��
	io_sti();


	//�����ⲿ�豸 ������
	//���û�����
	fifo8_init(&keyfifo, 32, keybuf);
	fifo8_init(&mousefifo, 128, mousebuf);
	//�޸�PIC��IMR���Ա�������������̵��ж�
	io_out8(PIC0_IMR, 0xf9); /* �S��(11111001) */
	io_out8(PIC1_IMR, 0xef); /* �S��(11101111) */


	//�趨��ɫ��
	init_palette();
	//���û����淽�������
	init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);


	//��ʼ������
	init_keyboard();
	enable_mouse(&mdec);


	//��ʾ���
	int mx, my;
	//���㻭�����������
	mx = (binfo->scrnx - 16) / 2; 
	my = (binfo->scrny - 28 - 16) / 2;
	//��ʾ���������
	init_mouse_cursor8(mcursor, COL8_008484);
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
	//��ӡ�������
	sprintf(s, "(%d, %d)", mx, my);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
	

	//
	int i;
	for (;;) {
		io_cli();
		if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0) {
			io_stihlt();
		} else {
			if (fifo8_status(&keyfifo) != 0) {
				i = fifo8_get(&keyfifo);
				io_sti();
				sprintf(s, "%02X", i);
				boxfill8(binfo->vram, binfo->scrnx, COL8_008484,  0, 16, 15, 31);
				putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
			} else if (fifo8_status(&mousefifo) != 0) {
				i = fifo8_get(&mousefifo);
				io_sti();
				//���������ֽ����ݶ����ˣ���ʾ����
				
				if (mouse_decode(&mdec, i) != 0)
				{
					//��ʾ����ƶ�����
					sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
					if ((mdec.btn & 0x01) != 0)
					{
						s[1] = 'L';
					}
	
					if ((mdec.btn & 0x02) != 0)
					{
						s[3] = 'R';
					}

					if ((mdec.btn & 0x04) != 0)
					{
						s[2] = 'C';
					}
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 32, 16, 32+15*8-1, 31);
					putfonts8_asc(binfo->vram, binfo->scrnx, 31, 16, COL8_FFFFFF, s);
					//���ָ����ƶ�
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, mx, my, mx+15, my+15);

					mx += mdec.x;
					my += mdec.y;
					//�������ı߽緶Χ
					if (mx < 0)
					{
						mx = 0;
					}
					if (my < 0)
					{
						my = 0;
					}
					if (mx > binfo->scrnx -16)
					{
						mx = binfo->scrnx -16;
					}
					if (my > binfo->scrny -16)
					{
						my = binfo->scrny -16;
					}
					sprintf(s, "(%3d, %3d)", mx, my);
					//��������
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 0, 79, 15);
					//��ʾ����
					putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
					//����ƶ�������ʾ
					putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
				}
			}
		}
	}
}