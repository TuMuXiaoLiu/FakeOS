//Ҫʹ��sprintg���� �����ڿ�ͷ���� #include

//���Ӻ��������ļ�
#include "bootpack.h"
#include <stdio.h>

//������� �൱��Java�е�Main����
void HariMain(void)
{
	//����BOOTINFO�Ķ��󲢸�ֵ ָ�븳ֵ
	struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
	//����������
	struct FIFO32 fifo;
	//���ڴ���������
	char s[40];
	//����buff����
	int fifobuf[128];

	//������ʱ������
	struct TIMER *timer, *timer2, *timer3;
	struct  MOUSE_DEC mdec;
	//�������
	int mx, my, i, count = 0;

	//�����ڴ�����
	unsigned int memtotal;
	//�����ڴ��������
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;

	//ͼ�����
	struct SHTCTL *shtctl;
	//������  ���   ����
	struct SHEET *sht_back, *sht_mouse, *sht_win;
	//�����建��  ��껺��
	unsigned char *buf_back, buf_mouse[256], *buf_win;

	//��ʼ��GDTIDT
	init_gdtidt();
	//��ʼ��PIC
	init_pic();
	//��� ��ֹ�ж� ��IDT��ʼ��ʱ�����ж϶��ᱻ��ֹ����Ҫ�ڴ˽�����еĽ�ֹ�ж� ��CPU�����ⲿ�豸ָ��
	io_sti();

	//��ʼ��������
	fifo32_init(&fifo, 128, fifobuf);
	//��ʼ����ʱ��
	init_pit();
	//��ʼ��������
	init_keyboard(&fifo, 256);
	enable_mouse(&fifo, 512, &mdec);

	//�޸�PIT��PIC1�ͼ��̵��жϣ�����Ϊ11111000
	io_out8(PIC0_IMR, 0xf8);
	//�޸�PIC��IMR���Ա�������������ж�
	io_out8(PIC1_IMR, 0xef); /* �S��(11101111) */
	
	//��ʼ����ʱ�������
	timer = timer_alloc();
	timer_init(timer, &fifo, 10);
	timer_settimer(timer, 1000);
	
	timer2 = timer_alloc();
	timer_init(timer2, &fifo, 3);
	timer_settimer(timer2, 300);

	timer3 = timer_alloc();
	timer_init(timer3, &fifo, 1);
	timer_settimer(timer3, 50);

	//�����ڴ����
	memtotal = memtest( 0x00400000, 0xbfffffff);
	memman_init(memman);
	// 0x00001000 - 0x0009efff
	memman_free(memman, 0x00001000, 0x0009e000);
	memman_free(memman, 0x00400000, memtotal - 0x00400000);

	//�趨��ɫ��
	init_palette();
	//���û����淽�������-> ����ͼ�����
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
	//��ȡ������ͼ�����
	sht_back = sheet_alloc(shtctl);
	//��ȡ���ͼ�����
	sht_mouse = sheet_alloc(shtctl);
	//��ȡ����ͼ�����
	sht_win = sheet_alloc(shtctl);

	//�����建��
	buf_back = (unsigned char *)memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	//���ڻ���
	buf_win = (unsigned char *)memman_alloc_4k(memman, 160 * 52);

	//������û��͸��ɫ
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);
	//����͸��ɫ��99
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
	//����
	sheet_setbuf(sht_win, buf_win, 160, 52, -1);

	//��ʼ����Ļ
	init_screen8(buf_back, binfo->scrnx, binfo->scrny);
	//��ʼ�����  ����ɫ��99 
	init_mouse_cursor8(buf_mouse, 99);
	//���ô��ں���
	make_window8(buf_win, 160, 52, "counter");

	//������λ��
	sheet_slide(sht_back, 0, 0);
	
	//���㻭�����������
	mx = (binfo->scrnx - 16) / 2; 
	my = (binfo->scrny - 28 - 16) / 2;
	//��ʾ���������
	sheet_slide(sht_mouse, mx, my);
	//��ʾ����
	sheet_slide(sht_win, 80, 72);

	//���ñ�����ͼ��߶�
	sheet_updown(sht_back, 0);
	//���ô���ͼ��߶�
	sheet_updown(sht_win, 1);
	//�������ͼ��߶�
	sheet_updown(sht_mouse, 2);

	//��ӡ�������
	sprintf(s, "(%3d, %3d)", mx, my);
	putfonts8_asc_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_008484, s, 10);
	
	//��ӡ �ڴ��С�Լ�����ռ�
	sprintf(s, "memory %dMB  free : %dKB", memtotal / (1024*1024), memman_total(memman) / 1024);
	putfonts8_asc_sht(sht_back, 0, 32, COL8_FFFFFF, COL8_008484, s, 40);
	
	for (;;) {
		count++;
		io_cli();
		if (fifo32_status(&fifo) == 0) {
			//����HLT
			io_sti();
		} else {
			i = fifo32_get(&fifo);
			io_sti();
			if (256 <= i && i <= 511){	//����
				sprintf(s, "%02X", i - 256);
				putfonts8_asc_sht(sht_back, 0, 16, COL8_FFFFFF, COL8_008484, s, 2);
			}else if (512 <= i && i <= 767) {//����
				//���������ֽ����ݶ����ˣ���ʾ����
				if (mouse_decode(&mdec, i - 512) != 0) {
					//��ʾ����ƶ�����
					sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
					if ((mdec.btn & 0x01) != 0) { s[1] = 'L'; }
					if ((mdec.btn & 0x02) != 0) { s[3] = 'R'; }
					if ((mdec.btn & 0x04) != 0) { s[2] = 'C'; }
					putfonts8_asc_sht(sht_back, 32, 16, COL8_FFFFFF, COL8_008484, s, 15);

					mx += mdec.x;
					my += mdec.y;
					//�������ı߽緶Χ
					if (mx < 0) { mx = 0; }
					if (my < 0) { my = 0; }

					if (mx > binfo->scrnx -1) { mx = binfo->scrnx -1; }
					if (my > binfo->scrny -1) { my = binfo->scrny -1; }

					sprintf(s, "(%3d, %3d)", mx, my);
					putfonts8_asc_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_008484, s, 10);
					//ˢ������
					sheet_slide(sht_mouse, mx, my);
				}
			} else if (i == 10) {//10�붨ʱ��
				putfonts8_asc_sht(sht_back, 0, 64, COL8_FFFFFF, COL8_008484, "10[sec]", 7);
				sprintf(s, "%010d", count);
				putfonts8_asc_sht(sht_win, 40, 28, COL8_000000, COL8_C6C6C6, s, 10);
			}else if (i == 3){//3�붨ʱ��
				putfonts8_asc_sht(sht_back, 0, 80, COL8_FFFFFF, COL8_008484, "03[sec]", 20);
				count = 0; /* ��ʼ���� */
			}else if (i == 1){//��궨ʱ��
				timer_init(timer3, &fifo, 0);
				boxfill8(buf_back, binfo->scrnx, COL8_008484, 8, 96, 15, 111);
				timer_settimer(timer3, 50);
				sheet_refresh(sht_back, 8, 96, 16, 112);
			}else if(i == 0){//��궨ʱ��
				timer_init(timer3, &fifo, 1);
				boxfill8(buf_back, binfo->scrnx, COL8_FFFFFF, 8, 96, 15, 111);
				timer_settimer(timer3, 50);
				sheet_refresh(sht_back, 8, 96, 16, 112);
			}
		}
	}
}


//�������ڵĺ���
void make_window8(unsigned char *buf, int xsize, int ysize, char *title)
{
	static char closebtn[14][16] = {
		"OOOOOOOOOOOOOOO@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQQQ@@QQQQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"O$$$$$$$$$$$$$$@",
		"@@@@@@@@@@@@@@@@"
	};

	int x, y;
	char c;
	boxfill8(buf, xsize, COL8_C6C6C6, 0, 0, xsize - 1, 0 );
	boxfill8(buf, xsize, COL8_FFFFFF, 1, 1, xsize - 2, 1 );
	boxfill8(buf, xsize, COL8_C6C6C6, 0, 0, 0, ysize - 1);
	boxfill8(buf, xsize, COL8_FFFFFF, 1, 1, 1, ysize - 2);
	boxfill8(buf, xsize, COL8_848484, xsize - 2, 1, xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, xsize - 1, 0, xsize - 1, ysize - 1);
	boxfill8(buf, xsize, COL8_C6C6C6, 2, 2, xsize - 3, ysize - 3);
	boxfill8(buf, xsize, COL8_000084, 3, 3, xsize - 4, 20 );
	boxfill8(buf, xsize, COL8_848484, 1, ysize - 2, xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, 0, ysize - 1, xsize - 1, ysize - 1);
	putfonts8_asc(buf, xsize, 24, 4, COL8_FFFFFF, title);

	for (y = 0; y < 14; y++) {
		for (x = 0; x < 16; x++) {
			c = closebtn[y][x];
			if (c == '@') {
				c = COL8_000000;
			} else if (c == '$') {
				c = COL8_848484;
			} else if (c == 'Q') {
				c = COL8_C6C6C6;
			} else {
				c = COL8_FFFFFF;
			}
			buf[(5 + y) * xsize + (xsize - 21 + x)] = c;
		}
	}
	return;
}


//��д���뺯��  xy ��ʾλ�õ�����  c  �ַ���ɫ(color)  b  ������ɫ(back color)  s  �ַ���(string)  l  �ַ�������(length)
void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l){
	boxfill8(sht->buf, sht->bxsize, b, x, y, x+1*8-1, y+15);
	putfonts8_asc(sht->buf, sht->bxsize, x, y, c, s);
	sheet_refresh(sht, x, y, x + l * 8, y + 16);
	return;
}