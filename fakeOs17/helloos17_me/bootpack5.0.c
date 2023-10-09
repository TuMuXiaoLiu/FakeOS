//Ҫʹ��sprintg���� �����ڿ�ͷ���� #include

//���Ӻ��������ļ�
#include "bootpack.h"
#include <stdio.h>

void make_window8(unsigned char *buf, int xsize, int ysize, char *title, char act);
void make_wtitle8(unsigned char *buf, int xsize, char *title, char act);
void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l);
void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c);
void console_task(struct SHEET *sheet);


//������� �൱��Java�е�Main����
void HariMain(void){
	//����BOOTINFO�Ķ��󲢸�ֵ ָ�븳ֵ
	struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
	//����������
	struct FIFO32 fifo;
	//���ڴ���������
	char s[40];
	//����buff����
	int fifobuf[128];
	//�������
	int mx, my, i, cursor_x, cursor_c;
	//�����ڴ�����
	unsigned int memtotal;
	struct MOUSE_DEC mdec;
	//�����ڴ��������
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;

	//ͼ�����
	struct SHTCTL *shtctl;
	//�����������
	static char keytable0[0x80] = {
		0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0,   0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0,   0,   'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0,   0,   ']', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0x5c, 0,  0,   0,   0,   0,   0,   0,   0,   0,   0x5c, 0,  0
	};
	static char keytable1[0x80] = {
		0,   0,   '!', 0x22, '#', '$', '%', '&', 0x27, '(', ')', '~', '=', '~', 0,   0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '`', '{', 0,   0,   'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', '+', '*', 0,   0,   '}', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', '<', '>', '?', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   '_', 0,   0,   0,   0,   0,   0,   0,   0,   0,   '|', 0,   0
	};
	int key_to = 0, key_shift = 0;

	//������  ���   ����
	struct SHEET *sht_back, *sht_mouse, *sht_win, *sht_cons;
	//�����建��  ��껺��
	unsigned char *buf_back, buf_mouse[256], *buf_win, *buf_cons;
	//�������������
	struct TASK *task_a, *task_cons;
	//������ʱ������
	struct TIMER *timer;
	

	//��ʼ��GDTIDT
	init_gdtidt();
	//��ʼ��PIC
	init_pic();
	//��� ��ֹ�ж� ��IDT��ʼ��ʱ�����ж϶��ᱻ��ֹ����Ҫ�ڴ˽�����еĽ�ֹ�ж� ��CPU�����ⲿ�豸ָ��
	io_sti();

	//��ʼ��������
	fifo32_init(&fifo, 128, fifobuf, 0);
	//��ʼ����ʱ��
	init_pit();
	//��ʼ��������
	init_keyboard(&fifo, 256);
	enable_mouse(&fifo, 512, &mdec);

	//�޸�PIT��PIC1�ͼ��̵��жϣ�����Ϊ11111000
	io_out8(PIC0_IMR, 0xf8);
	//�޸�PIC��IMR���Ա�������������ж�
	io_out8(PIC1_IMR, 0xef); /* �S��(11101111) */
	
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
	
	task_a = task_init(memman);
	fifo.task = task_a;
	task_run(task_a, 1, 0);
	

	//sht_back
	//��ȡ������ͼ�����
	sht_back = sheet_alloc(shtctl);
	//�����建��
	buf_back = (unsigned char *)memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	//������û��͸��ɫ
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);
	//��ʼ����Ļ
	init_screen8(buf_back, binfo->scrnx, binfo->scrny);


	//sht_cons
	sht_cons = sheet_alloc(shtctl);
	buf_cons = (unsigned char *)memman_alloc_4k(memman, 256 * 165);
	sheet_setbuf(sht_cons, buf_cons, 256, 165, -1);//��͸��ɫ
	make_window8(buf_cons, 256, 165, "console", 0);
	make_textbox8(sht_cons, 8, 28, 240, 128, COL8_000000);
	task_cons = task_alloc();
	task_cons->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 -8;
	task_cons->tss.eip = (int)&console_task;
	task_cons->tss.es = 1 * 8;
	task_cons->tss.cs = 2 * 8;
	task_cons->tss.ss = 1 * 8;
	task_cons->tss.ds = 1 * 8;
	task_cons->tss.fs = 1 * 8;
	task_cons->tss.gs = 1 * 8;
	*((int *)(task_cons->tss.esp + 4)) = (int)sht_cons;
	//level 2, priority 2
	task_run(task_cons, 2, 2);

	//sht_win
	//��ȡ����ͼ�����
	sht_win = sheet_alloc(shtctl);
	//���ڻ���
	buf_win = (unsigned char *)memman_alloc_4k(memman, 160 * 52);
	//����
	sheet_setbuf(sht_win, buf_win, 144, 52, -1);
	//���ô��ں���
	make_window8(buf_win, 144, 52, "task_a", 1);
	make_textbox8(sht_win, 8, 28, 128, 16, COL8_FFFFFF);
	cursor_x = 8;
	cursor_c = COL8_FFFFFF;
	//��ʼ������ʱ��
	timer = timer_alloc();
	timer_init(timer, &fifo, 1);
	timer_settime(timer, 50);


	//sht_mouse
	//��ȡ���ͼ�����
	sht_mouse = sheet_alloc(shtctl);
	//����͸��ɫ��99
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
	//��ʼ�����  ����ɫ��99 
	init_mouse_cursor8(buf_mouse, 99);
	//���㻭�����������
	mx = (binfo->scrnx - 16) / 2; 
	my = (binfo->scrny - 28 - 16) / 2;


	//������λ��
	sheet_slide(sht_back,  0,  0);
	//��ʾ����
	sheet_slide(sht_cons, 32,  4);
	sheet_slide(sht_win,  64, 56);
	//��ʾ���������
	sheet_slide(sht_mouse, mx, my);	

	//���ñ�����ͼ��߶�
	sheet_updown(sht_back,	0);
	//���ô���ͼ��߶�
	sheet_updown(sht_cons,	1);
	sheet_updown(sht_win,	2);
	//�������ͼ��߶�
	sheet_updown(sht_mouse, 3);

	//��ӡ�������
	sprintf(s, "(%3d, %3d)", mx, my);
	putfonts8_asc_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_008484, s, 10);
	//��ӡ �ڴ��С�Լ�����ռ�
	sprintf(s, "memory %dMB  free : %dKB", memtotal / (1024*1024), memman_total(memman) / 1024);
	putfonts8_asc_sht(sht_back, 0, 32, COL8_FFFFFF, COL8_008484, s, 40);


	for (;;) {
		io_cli();
		if (fifo32_status(&fifo) == 0) {
			task_sleep(task_a);
			io_sti();
		} else {
			i = fifo32_get(&fifo);
			io_sti();
			//����
			if (256 <= i && i <= 511){
				sprintf(s, "%02X", i - 256);
				putfonts8_asc_sht(sht_back, 0, 16, COL8_FFFFFF, COL8_008484, s, 2);

				//����������תΪ�ַ�����
				if (i < 0x80 + 256){
					if (key_shift == 0){
						s[0] = keytable0[i - 256];
					} else {
						s[0] = keytable1[i - 256];
					}
				} else {
					s[0] = 0;
				}

				//һ���ַ�
				if (s[0] != 0){
					//���͸�����A
					if (key_to == 0){
						if (cursor_x < 128){
							//��ʾһ���ַ���ǰ��һ�ι��
							s[1] = 0;
							putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, s, 1);
							cursor_x += 8;
						}
					}else{
						//���͸������д���
						fifo32_put(&task_cons->fifo, keytable1[i - 256] + 256);
					}
				}
				//�˸��
				if (i == 256 + 0x0e){
					//���͸�����A
					if (key_to == 0){
						if (cursor_x > 8){
							//�ÿո���ѹ�������� ����һλ���
							putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, " ", 1);
							cursor_x -= 8;
						}
					}else{
						//���͸�������
						fifo32_put(&task_cons->fifo, 8 + 256);
					}
				}
				//TAB��
				if (i == 256 + 0x0f){
					if (key_to == 0){
						key_to = 1;
						make_wtitle8(buf_win, sht_win->bxsize, "task_a", 0);
						make_wtitle8(buf_cons, sht_cons->bxsize, "console", 1);
					} else {
						key_to = 0;
						make_wtitle8(buf_win, sht_win->bxsize, "task_a", 1);
						make_wtitle8(buf_cons, sht_cons->bxsize, "console", 0);
					}
					sheet_refresh(sht_win, 0, 0, sht_win->bxsize, 21);
					sheet_refresh(sht_cons, 0, 0, sht_cons->bxsize, 21);
				}

				//��shift ON
				if (i == 256 + 0x2a){
					key_shift |= 1;
				}
				//��shift ON
				if (i == 256 + 0x38){
					key_shift |= 2;
				}
				//��shift OFF
				if (i == 256 + 0xaa){
					key_shift &= -1;
				}
				//��shift off
				if (i == 256 + 0xb6){
					key_shift &= -2;
				}
				
				//�������ʾ
				boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x +7, 43);
				sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
			} else if (512 <= i && i <= 767) {//���
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
					//ˢ�����ָ��
					sheet_slide(sht_mouse, mx, my);
					//����϶�����
					if (mdec.btn & 0x01 != 0){
						sheet_slide(sht_win, mx-80, my - 8);
					}
				}
			} else if (i <= 1){//��궨ʱ��
				if (i !=0 ){
					timer_init(timer, &fifo, 0);
					cursor_c = COL8_000000;
				} else {//��궨ʱ��
					timer_init(timer, &fifo, 1);
					cursor_c = COL8_FFFFFF;
				}
				timer_settime(timer, 50);
				boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
				sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
			}
		}
	}
}


//�������ڵĺ���
void make_window8(unsigned char *buf, int xsize, int ysize, char *title, char act){
	boxfill8(buf, xsize, COL8_C6C6C6,		  0,	  	 0, xsize - 1,		   0);
	boxfill8(buf, xsize, COL8_FFFFFF,		  1,		 1, xsize - 2,	 	   1);
	boxfill8(buf, xsize, COL8_C6C6C6,		  0,		 0,		    0, ysize - 1);
	boxfill8(buf, xsize, COL8_FFFFFF,		  1,		 1,		    1, ysize - 2);
	boxfill8(buf, xsize, COL8_848484, xsize - 2,		 1, xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, xsize - 1,		 0, xsize - 1, ysize - 1);
	boxfill8(buf, xsize, COL8_C6C6C6,		  2,		 2, xsize - 3, ysize - 3);
	boxfill8(buf, xsize, COL8_848484,		  1, ysize - 2, xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000,		  0, ysize - 1, xsize - 1, ysize - 1);
	make_wtitle8(buf, xsize, title, act);
	return;
}

//���⺯��
void make_wtitle8(unsigned char *buf, int xsize, char *title, char act){
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
	char c, tc, tbc;
	if (act != 0){
		tc = COL8_FFFFFF;
		tbc = COL8_000084;
	} else {
		tc = COL8_C6C6C6;
		tbc = COL8_848484;
	}
	boxfill8(buf, xsize, tbc, 3, 3, xsize - 4, 20);
	putfonts8_asc(buf, xsize, 24, 4, tc, title);

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
	boxfill8(sht->buf, sht->bxsize, b, x, y, x + l * 8 - 1, y + 15);
	putfonts8_asc(sht->buf, sht->bxsize, x, y, c, s);
	sheet_refresh(sht, x, y, x + l * 8, y + 16);
	return;
}

void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c){
	int x1 = x0 + sx, y1 = y0 + sy;
	boxfill8(sht->buf, sht->bxsize, COL8_848484, x0 - 2, y0 - 3, x1 + 1, y0 - 3);
	boxfill8(sht->buf, sht->bxsize, COL8_848484, x0 - 3, y0 - 3, x0 - 3, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, COL8_FFFFFF, x0 - 3, y1 + 2, x1 + 1, y1 + 2);
	boxfill8(sht->buf, sht->bxsize, COL8_FFFFFF, x1 + 2, y0 - 3, x1 + 2, y1 + 2);
	boxfill8(sht->buf, sht->bxsize, COL8_000000, x0 - 1, y0 - 2, x1 + 0, y0 - 2);
	boxfill8(sht->buf, sht->bxsize, COL8_000000, x0 - 2, y0 - 2, x0 - 2, y1 + 0);
	boxfill8(sht->buf, sht->bxsize, COL8_C6C6C6, x0 - 2, y1 + 1, x1 + 0, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, COL8_C6C6C6, x1 + 1, y0 - 2, x1 + 1, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, c,			 x0 - 1, y0 - 1, x1 + 0, y1 + 0);
	return;
}

void task_b_main(struct SHEET *sht_win_b){
	struct FIFO32 fifo;
	struct TIMER *timer_1s;
	int i, fifobuf[128], count = 0, count0 = 0;
	char s[12];

	fifo32_init(&fifo, 128, fifobuf, 0);
	timer_1s = timer_alloc();
	timer_init(timer_1s, &fifo, 100);
	timer_settime(timer_1s, 100);

	for (; ; ){
		count++;
		io_cli();
		if (fifo32_status(&fifo) == 0){
			io_sti();
		} else {
			i = fifo32_get(&fifo);
			io_sti();
			if ( i == 100){
				sprintf(s, "%11d", count- count0);
				putfonts8_asc_sht(sht_win_b, 24, 28, COL8_000000, COL8_C6C6C6, s, 11);
				count0 = count;
				timer_settime(timer_1s, 100);
			}
		}
	}
}

//�����д���
void console_task(struct SHEET *sheet){
	struct TIMER *timer;
	struct TASK *task = task_now();
	int i, fifobuf[128], cursor_x = 16, cursor_c = COL8_000000;
	char s[2];

	fifo32_init(&task->fifo, 128, fifobuf, task);
	timer = timer_alloc();
	timer_init(timer, &task->fifo, 1);
	timer_settime(timer, 50);

	//��ʾ��ʾ��
	putfonts8_asc_sht(sheet, 8, 28, COL8_FFFFFF, COL8_000000, ">", 1);

	for (; ; ){
		io_cli();
		if (fifo32_status(&task->fifo) == 0){
			task_sleep(task);
			io_sti();
		} else {
			i = fifo32_get(&task->fifo);
			io_sti();
			//����ö�ʱ��
			if (i <= 1){
				if (i != 0){
					timer_init(timer, &task->fifo, 0);
					cursor_c = COL8_FFFFFF;
				} else {
					timer_init(timer, &task->fifo, 1);
					cursor_c = COL8_000000;
				}
				timer_settime(timer, 50);
			}
			//��������
			if (256 <=  i && i <= 511){
				//�˸��
				if (i == 8 + 256){
					if (cursor_x > 16){
						putfonts8_asc_sht(sheet, cursor_x, 28, COL8_FFFFFF, COL8_000000, " ", 1);
						cursor_x -=8;
					}
				}else{
					//һ���ַ�
					if (cursor_x < 240){
						s[0] = i - 256;
						s[1] = 0;
						putfonts8_asc_sht(sheet, cursor_x, 28, COL8_FFFFFF, COL8_000000, s, 1);
						cursor_x += 8;
					}
				}
			}
			//��ʾ���
			boxfill8(sheet->buf, sheet->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
			sheet_refresh(sheet, cursor_x, 28, cursor_x + 8, 44);			
		}
	}
}

//����ö�ʱ��
void gb_timer(struct SHEET *sheet, struct TIMER *timer, struct FIFO32 fifo){
	int i, cursor_x = 8, cursor_c = COL8_000000;
	i = fifo32_get(&fifo);
	io_sti();
	if (i <= 1){
		if (i != 0){
			timer_init(timer, &fifo, 0);
			cursor_c = COL8_FFFFFF;
		} else {
			timer_init(timer, &fifo, 1);
			cursor_c = COL8_000000;
		}
		timer_settime(timer, 50);
		boxfill8(sheet->buf, sheet->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
		sheet_refresh(sheet, cursor_x, 28, cursor_x + 8, 44);
	}
}