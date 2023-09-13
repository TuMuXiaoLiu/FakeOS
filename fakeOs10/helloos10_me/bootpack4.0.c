//Ҫʹ��sprintg���� �����ڿ�ͷ���� #include

//��Ӻ��������ļ�
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
	//�������
	int mx, my, i;


	//�����ڴ�����
	unsigned int memtotal;
	//�����ڴ�������
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;


	//ͼ�����
	struct SHTCTL *shtctl;
	//������  ���
	struct SHEET *sht_back, *sht_mouse;
	//�����建��  ��껺��
	unsigned char *buf_back, buf_mouse[256];


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


	//��ʼ������
	init_keyboard();
	enable_mouse(&mdec);
	//�����ڴ����
	memtotal = memtest( 0x00400000, 0xbfffffff);
	memman_init(memman);
	// 0x00001000 - 0x0009efff
	memman_free(memman, 0x00001000, 0x0009e000);
	memman_free(memman, 0x00400000, memtotal - 0x00400000);


	//�趨��ɫ��
	init_palette();
	//���û����淽�������-> ����ͼ�����
	//init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
	//��ȡ������ͼ�����
	sht_back = sheet_alloc(shtctl);
	//��ȡ���ͼ�����
	sht_mouse = sheet_alloc(shtctl);
	//�����建��
	buf_back = (unsigned char *)memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	//������û��͸��ɫ
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);
	//����͸��ɫ��99
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
	//��ʼ����Ļ
	init_screen8(buf_back, binfo->scrnx, binfo->scrny);
	//��ʼ�����  ����ɫ��99   ���ԭ87�д���
	init_mouse_cursor8(buf_mouse, 99);
	//������λ��
	sheet_slide(shtctl, sht_back, 0, 0);
	
	//���㻭�����������
	mx = (binfo->scrnx - 16) / 2; 
	my = (binfo->scrny - 28 - 16) / 2;
	//��ʾ���������
	//init_mouse_cursor8(mcursor, COL8_008484);
	sheet_slide(shtctl, sht_mouse, mx, my);
	//���ñ�����ͼ��߶�
	sheet_updown(shtctl, sht_back, 0);
	//�������ͼ��߶�
	sheet_updown(shtctl, sht_mouse, 1);

	//��ӡ�������
	sprintf(s, "(%d, %d)", mx, my);
	//
	putfonts8_asc(buf_back, binfo->scrnx, 0, 0, COL8_FFFFFF, s);

	
	//��ӡ �ڴ��С�Լ�����ռ�
	sprintf(s, "memory %dMB  free : %dKB", memtotal / (1024*1024), memman_total(memman) / 1024);
	putfonts8_asc(buf_back, binfo->scrnx, 0, 32, COL8_FFFFFF, s);
	//ˢ������
	sheet_refresh(shtctl, sht_back, 0, 0, binfo->scrnx, 48);
	

	for (;;) 
	{
		io_cli();
		if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0) 
		{
			io_stihlt();
		} else 
		{
			if (fifo8_status(&keyfifo) != 0) {
				i = fifo8_get(&keyfifo);
				io_sti();
				sprintf(s, "%02X", i);
				boxfill8(buf_back, binfo->scrnx, COL8_008484,  0, 16, 15, 31);
				putfonts8_asc(buf_back, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
				//ˢ������
				sheet_refresh(shtctl, sht_back, 0, 16, 16, 32);
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
					boxfill8(buf_back, binfo->scrnx, COL8_008484, 32, 16, 32+15*8-1, 31);
					putfonts8_asc(buf_back, binfo->scrnx, 31, 16, COL8_FFFFFF, s);
					//���ָ����ƶ�
					//ˢ������
					sheet_refresh(shtctl, sht_back, 32, 16, 32+15*8, 32);

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
					boxfill8(buf_back, binfo->scrnx, COL8_008484, 0, 0, 79, 15);
					//��ʾ����
					putfonts8_asc(buf_back, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
					//����ƶ�������ʾ
					//ˢ������
					sheet_refresh(shtctl, sht_back, 0, 0, 80, 16);
					sheet_slide(shtctl, sht_mouse, mx, my);
				}
			}
		}
	}
}


