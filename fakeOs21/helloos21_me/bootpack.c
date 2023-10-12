//��Ӻ��������ļ�
#include "bootpack.h"
#include <stdio.h>

#define	KEYCMD_LED		0xed

void HariMain(void){//������� �൱��Java�е�Main����
	struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;//����BOOTINFO�Ķ��󲢸�ֵ ָ�븳ֵ
	struct SHTCTL *shtctl;//ͼ�����
	char s[40];//���ڴ���������
	struct FIFO32 fifo, keycmd;//����������
	int fifobuf[128], keycmd_buf[32];//����buff����
	int mx, my, i, cursor_x, cursor_c;//�������
	unsigned int memtotal;//�����ڴ�����
	struct MOUSE_DEC mdec;
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;//�����ڴ�������
	unsigned char *buf_back, buf_mouse[256], *buf_win, *buf_cons;//�����建��  ��껺���
	struct SHEET *sht_back, *sht_mouse, *sht_win, *sht_cons;//������  ���   ����
	struct TASK *task_a, *task_cons;//�������������
	struct TIMER *timer;//������ʱ������
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
	int key_to = 0, key_shift = 0, key_leds = (binfo->leds >> 4)& 7, keycmd_wait = -1;
	
	init_gdtidt();//��ʼ��GDTIDT
	init_pic();//��ʼ��PIC
	io_sti();//��� ��ֹ�ж� ��IDT��ʼ��ʱ�����ж϶��ᱻ��ֹ����Ҫ�ڴ˽�����еĽ�ֹ�ж� ��CPU�����ⲿ�豸ָ��
	fifo32_init(&fifo, 128, fifobuf, 0);//��ʼ��������
	init_pit();//��ʼ����ʱ��
	init_keyboard(&fifo, 256);//��ʼ��������
	enable_mouse(&fifo, 512, &mdec);
	io_out8(PIC0_IMR, 0xf8);//�޸�PIT��PIC1�ͼ��̵��жϣ����Ϊ11111000
	io_out8(PIC1_IMR, 0xef); //�޸�PIC��IMR���Ա�������������ж�/* �S��(11101111) */
	fifo32_init(&keycmd, 32, keycmd_buf, 0);
	memtotal = memtest( 0x00400000, 0xbfffffff);
	memman_init(memman);
	memman_free(memman, 0x00001000, 0x0009e000);
	memman_free(memman, 0x00400000, memtotal - 0x00400000);
	init_palette();	//�趨��ɫ��
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);//���û����淽�������-> ����ͼ�����
	task_a = task_init(memman);
	fifo.task = task_a;
	task_run(task_a, 1, 2);
	//sht_back
	sht_back = sheet_alloc(shtctl);//��ȡ������ͼ�����
	buf_back = (unsigned char *)memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);//�����建��
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);//������û��͸��ɫ
	init_screen8(buf_back, binfo->scrnx, binfo->scrny);//��ʼ����Ļ
	//sht_cons
	sht_cons = sheet_alloc(shtctl);
	buf_cons = (unsigned char *)memman_alloc_4k(memman, 256 * 165);
	sheet_setbuf(sht_cons, buf_cons, 256, 165, -1);//��͸��ɫ
	make_window8(buf_cons, 256, 165, "console", 0);
	make_textbox8(sht_cons, 8, 28, 240, 128, COL8_000000);
	task_cons = task_alloc();
	task_cons->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 12;
	task_cons->tss.eip = (int)&console_task;
	task_cons->tss.es = 1 * 8;
	task_cons->tss.cs = 2 * 8;
	task_cons->tss.ss = 1 * 8;
	task_cons->tss.ds = 1 * 8;
	task_cons->tss.fs = 1 * 8;
	task_cons->tss.gs = 1 * 8;
	*((int *)(task_cons->tss.esp + 4)) = (int)sht_cons;
	*((int *)(task_cons->tss.esp + 8)) = memtotal;
	task_run(task_cons, 2, 2);	//level 2, priority 2
	//sht_win
	sht_win = sheet_alloc(shtctl);//��ȡ����ͼ�����
	buf_win = (unsigned char *)memman_alloc_4k(memman, 160 * 52);//���ڻ���
	sheet_setbuf(sht_win, buf_win, 144, 52, -1);//����
	make_window8(buf_win, 144, 52, "task_a", 1);//���ô��ں���
	make_textbox8(sht_win, 8, 28, 128, 16, COL8_FFFFFF);
	cursor_x = 8;
	cursor_c = COL8_FFFFFF;
	timer = timer_alloc();//��ʼ������ʱ��
	timer_init(timer, &fifo, 1);
	timer_settime(timer, 50);
	//sht_mouse
	sht_mouse = sheet_alloc(shtctl);//��ȡ���ͼ�����
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);//����͸��ɫ��99
	init_mouse_cursor8(buf_mouse, 99);//��ʼ�����  ����ɫ��99 
	mx = (binfo->scrnx - 16) / 2; 
	my = (binfo->scrny - 28 - 16) / 2;

	sheet_slide(sht_back,  0,  0);//������λ��
	sheet_slide(sht_cons, 32,  4);//��ʾ����
	sheet_slide(sht_win,  64, 56);
	sheet_slide(sht_mouse, mx, my);	//��ʾ���������
	sheet_updown(sht_back,	0);//���ñ�����ͼ��߶�
	sheet_updown(sht_cons,	1);//���ô���ͼ��߶�
	sheet_updown(sht_win,	2);
	sheet_updown(sht_mouse, 3);//�������ͼ��߶�
	fifo32_put(&keycmd, KEYCMD_LED);//��ֹ�ͼ��̵�ǰ״̬��ͻ����һ��ʼ�ͽ�������
	fifo32_put(&keycmd, key_leds);

	for (;;) {
		if (fifo32_status(&keycmd) > 0 && keycmd_wait < 0){//�����������̿��������͵����ݣ�������
			keycmd_wait = fifo32_get(&keycmd);
			wait_KBC_sendready();
			io_out8(PORT_KEYDAT, keycmd_wait);
		}
		io_cli();
		if (fifo32_status(&fifo) == 0) {
			task_sleep(task_a);
			io_sti();
		} else {
			i = fifo32_get(&fifo);
			io_sti();
			if (256 <= i && i <= 511){//����
				//����������תΪ�ַ�����
				if (i < 0x80 + 256){if (key_shift == 0){ s[0] = keytable0[i - 256]; } else { s[0] = keytable1[i - 256]; } } else { s[0] = 0; }
				//��������ַ�ΪӢ����ĸʱ
				if ('A' <= s[0] && s[0] <= 'Z'){ if (((key_leds & 4) == 0 && key_shift == 0) || ((key_leds & 4) != 0 && key_shift != 0)){ s[0] += 0x20; } }//����д��ĸתΪСд��ĸ
				if (s[0] != 0){//һ���ַ�
					if (key_to == 0){//���͸�����A
						if (cursor_x < 128){
							//��ʾһ���ַ���ǰ��һ�ι��
							s[1] = 0;
							putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, s, 1);
							cursor_x += 8;
						}
					} else { fifo32_put(&task_cons->fifo, s[0] + 256); }//���͸������д���
				}
				if (i == 256 + 0x0e){//�˸��
					if (key_to == 0){//���͸�����A
						if (cursor_x > 8){//�ÿո���ѹ�������� ����һλ���
							putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, " ", 1);
							cursor_x -= 8;
						}
					} else { fifo32_put(&task_cons->fifo, 8 + 256); }//���͸�������
				}
				//�س���  �����������
				if (i == 256 + 0x1c){ if (key_to != 0){ fifo32_put(&task_cons->fifo, 10 + 256); } }
				//TAB��
				if (i == 256 + 0x0f){
					if (key_to == 0){
						key_to = 1;
						make_wtitle8(buf_win, sht_win->bxsize, "task_a", 0);
						make_wtitle8(buf_cons, sht_cons->bxsize, "console", 1);
						cursor_c = -1;//����ʾ���
						boxfill8(sht_win->buf, sht_win->bxsize, COL8_FFFFFF, cursor_x, 28, cursor_x + 7, 43);
						fifo32_put(&task_cons->fifo, 2);//�����д��ڹ����˸
					} else {
						key_to = 0;
						make_wtitle8(buf_win, sht_win->bxsize, "task_a", 1);
						make_wtitle8(buf_cons, sht_cons->bxsize, "console", 0);
						cursor_c = COL8_000000;//��ʾ���
						fifo32_put(&task_cons->fifo, 3);//�����д��ڹ����˸��
					}
					sheet_refresh(sht_win, 0, 0, sht_win->bxsize, 21);
					sheet_refresh(sht_cons, 0, 0, sht_cons->bxsize, 21);
				}
				
				if (i == 256 + 0x2a){ key_shift |= 1;}//��shift ON
				if (i == 256 + 0x36){ key_shift |= 2;}//��shift ON
				if (i == 256 + 0xaa){ key_shift &= ~1;}//��shift OFF
				if (i == 256 + 0xb6){ key_shift &= ~2;}//��shift off
				
				if (i == 256 + 0x3a){//CapsLock
					key_leds ^= 4;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				if (i == 256 + 0x45){	//NumLock
					key_leds ^= 2;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				if (i == 256 + 0x46){//ScrollLock
					key_leds ^=1;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				if (i == 256 + 0xfa){ keycmd_wait = -1;}//���̳ɹ����յ�����
				if (i == 256 + 0xfe){ //����û�гɹ����յ�����
					wait_KBC_sendready();
					io_out8(PORT_KEYDAT, keycmd_wait);
				}
				if (cursor_c >= 0){ boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x +7, 43); }//�������ʾ 
				sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);//ˢ��
			} else if (512 <= i && i <= 767) {//���
				if (mouse_decode(&mdec, i - 512) != 0) {//���������ֽ����ݶ����ˣ���ʾ����
					mx += mdec.x;
					my += mdec.y;
					//�������ı߽緶Χ
					if (mx < 0) { mx = 0; }
					if (my < 0) { my = 0; }
					if (mx > binfo->scrnx -1) { mx = binfo->scrnx -1; }
					if (my > binfo->scrny -1) { my = binfo->scrny -1; }
					sheet_slide(sht_mouse, mx, my);//ˢ�����ָ��
					if ((mdec.btn & 0x01) != 0){ sheet_slide(sht_win, mx-80, my - 8); }//����϶�����
				}
			} else if (i <= 1){//��궨ʱ��
				if (i !=0 ){
					timer_init(timer, &fifo, 0);
					if (cursor_c >= 0){ cursor_c = COL8_000000; }
				} else {//��궨ʱ��
					timer_init(timer, &fifo, 1);
					if (cursor_c >= 0){ cursor_c = COL8_FFFFFF; }
				}
				timer_settime(timer, 50);
				if (cursor_c >= 0){
					boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
					sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
				}
			}
		}
	}
}
