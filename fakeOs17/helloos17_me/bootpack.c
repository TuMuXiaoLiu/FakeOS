//要使用sprintg函数 必须在开头加上 #include

//添加函数引用文件
#include "bootpack.h"
#include <stdio.h>

#define	KEYCMD_LED		0xed

void make_window8(unsigned char *buf, int xsize, int ysize, char *title, char act);
void make_wtitle8(unsigned char *buf, int xsize, char *title, char act);
void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l);
void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c);
void console_task(struct SHEET *sheet);


//程序入口 相当于Java中的Main函数
void HariMain(void){
	//声明BOOTINFO的对象并赋值 指针赋值
	struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
	//声明缓冲流
	struct FIFO32 fifo, keycmd;
	//用于存放输出内容
	char s[40];
	//缓冲buff数组
	int fifobuf[128], keycmd_buf[32];
	//鼠标坐标
	int mx, my, i, cursor_x, cursor_c;
	//声明内存总数
	unsigned int memtotal;
	struct MOUSE_DEC mdec;
	//声明内存管理对象
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;

	//图层管理
	struct SHTCTL *shtctl;
	//定义键盘输入
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

	//背景板  鼠标   窗口
	struct SHEET *sht_back, *sht_mouse, *sht_win, *sht_cons;
	//背景板缓冲  鼠标缓冲
	unsigned char *buf_back, buf_mouse[256], *buf_win, *buf_cons;
	//声明多任务对象
	struct TASK *task_a, *task_cons;
	//声明计时器对象
	struct TIMER *timer;
	

	//初始化GDTIDT
	init_gdtidt();
	//初始化PIC
	init_pic();
	//解除 禁止中断 在IDT初始化时所有中断都会被禁止，需要在此解除所有的禁止中断 让CPU接受外部设备指令
	io_sti();

	//初始化缓冲区
	fifo32_init(&fifo, 128, fifobuf, 0);
	fifo32_init(&keycmd, 32, keycmd_buf, 0);
	//初始化定时器
	init_pit();
	//初始化鼠标键盘
	init_keyboard(&fifo, 256);
	enable_mouse(&fifo, 512, &mdec);

	//修改PIT和PIC1和键盘的中断，许可为11111000
	io_out8(PIC0_IMR, 0xf8);
	//修改PIC的IMR，以便接受来自鼠标的中断
	io_out8(PIC1_IMR, 0xef); /* 許可(11101111) */
	
	//进行内存管理
	memtotal = memtest( 0x00400000, 0xbfffffff);
	memman_init(memman);
	// 0x00001000 - 0x0009efff
	memman_free(memman, 0x00001000, 0x0009e000);
	memman_free(memman, 0x00400000, memtotal - 0x00400000);

	//设定调色板
	init_palette();
	//调用画界面方法并入参-> 启动图层管理
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
	
	task_a = task_init(memman);
	fifo.task = task_a;
	task_run(task_a, 1, 0);
	

	//sht_back
	//获取背景板图层对象
	sht_back = sheet_alloc(shtctl);
	//背景板缓冲
	buf_back = (unsigned char *)memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	//背景板没有透明色
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);
	//初始化屏幕
	init_screen8(buf_back, binfo->scrnx, binfo->scrny);


	//sht_cons
	sht_cons = sheet_alloc(shtctl);
	buf_cons = (unsigned char *)memman_alloc_4k(memman, 256 * 165);
	sheet_setbuf(sht_cons, buf_cons, 256, 165, -1);//无透明色
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
	//获取窗口图层对象
	sht_win = sheet_alloc(shtctl);
	//窗口缓冲
	buf_win = (unsigned char *)memman_alloc_4k(memman, 160 * 52);
	//窗口
	sheet_setbuf(sht_win, buf_win, 144, 52, -1);
	//调用窗口函数
	make_window8(buf_win, 144, 52, "task_a", 1);
	make_textbox8(sht_win, 8, 28, 128, 16, COL8_FFFFFF);
	cursor_x = 8;
	cursor_c = COL8_FFFFFF;
	//初始化光标计时器
	timer = timer_alloc();
	timer_init(timer, &fifo, 1);
	timer_settime(timer, 50);


	//sht_mouse
	//获取鼠标图层对象
	sht_mouse = sheet_alloc(shtctl);
	//鼠标的透明色号99
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
	//初始化鼠标  背景色号99 
	init_mouse_cursor8(buf_mouse, 99);
	//计算画面的中心坐标
	mx = (binfo->scrnx - 16) / 2; 
	my = (binfo->scrny - 28 - 16) / 2;


	//背景板位置
	sheet_slide(sht_back,  0,  0);
	//显示窗口
	sheet_slide(sht_cons, 32,  4);
	sheet_slide(sht_win,  64, 56);
	//显示并勾勒鼠标
	sheet_slide(sht_mouse, mx, my);	

	//设置背景板图层高度
	sheet_updown(sht_back,	0);
	//设置窗口图层高度
	sheet_updown(sht_cons,	1);
	sheet_updown(sht_win,	2);
	//设置鼠标图层高度
	sheet_updown(sht_mouse, 3);

	//打印鼠标坐标
	sprintf(s, "(%3d, %3d)", mx, my);
	putfonts8_asc_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_008484, s, 10);
	//打印 内存大小以及空余空间
	sprintf(s, "memory %dMB  free : %dKB", memtotal / (1024*1024), memman_total(memman) / 1024);
	putfonts8_asc_sht(sht_back, 0, 32, COL8_FFFFFF, COL8_008484, s, 40);
	
	//防止和键盘当前状态冲突，在一开始就进行设置
	fifo32_put(&keycmd, KEYCMD_LED);
	fifo32_put(&keycmd, key_leds);


	for (;;) {

		//如果存在向键盘控制器发送的数据，则发送它
		if (fifo32_status(&keycmd) > 0 && keycmd_wait < 0){
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
			//键盘
			if (256 <= i && i <= 511){
				sprintf(s, "%02X", i - 256);
				putfonts8_asc_sht(sht_back, 0, 16, COL8_FFFFFF, COL8_008484, s, 2);
				//将按键编码转为字符编码
				if (i < 0x80 + 256){
					if (key_shift == 0){
						s[0] = keytable0[i - 256];
					} else {
						s[0] = keytable1[i - 256];
					}
				} else {
					s[0] = 0;
				}
				//当输入的字符为英文字母时
				if ('A' <= s[0] && s[0] <= 'Z'){
					if ( ((key_leds & 4) == 0 && key_shift == 0) || ((key_leds & 4) != 0 && key_shift != 0)){
						//将大写字母转为小写字母
						s[0] += 0x20;
					}
				}
				//一般字符
				if (s[0] != 0){
					//发送给任务A
					if (key_to == 0){
						if (cursor_x < 128){
							//显示一个字符就前移一次光标
							s[1] = 0;
							putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, s, 1);
							cursor_x += 8;
						}
					}else{
						//发送给命令行窗口
						fifo32_put(&task_cons->fifo, keytable1[i - 256] + 256);
					}
				}
				//退格键
				if (i == 256 + 0x0e){
					//发送给任务A
					if (key_to == 0){
						if (cursor_x > 8){
							//用空格键把光标消除后， 后移一位光标
							putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, " ", 1);
							cursor_x -= 8;
						}
					}else{
						//发送给命令行
						fifo32_put(&task_cons->fifo, 8 + 256);
					}
				}
				//TAB键
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
				//左shift ON
				if (i == 256 + 0x2a){ key_shift |= 1;}
				//右shift ON
				if (i == 256 + 0x38){ key_shift |= 2;}
				//左shift OFF
				if (i == 256 + 0xaa){ key_shift &= ~1;}
				//右shift off
				if (i == 256 + 0xb6){ key_shift &= ~2;}
				
				//CapsLock
				if (i == 256 + 0x3a){
					key_leds ^= 4;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				//NumLock
				if (i == 256 + 0x45){
					key_leds ^= 2;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				//ScrollLock
				if (i == 256 + 0x46){
					key_leds ^=1;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				//键盘成功接收到数据
				if (i == 256 + 0xfa){ keycmd_wait = -1;}
				//键盘没有成功接收到数据
				if (i == 256 + 0xfe){ 
					wait_KBC_sendready();
					io_out8(PORT_KEYDAT, keycmd_wait);
				}
				

				//光标再显示
				boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x +7, 43);
				sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
			} else if (512 <= i && i <= 767) {//鼠标
				//鼠标的三个字节数据都齐了，显示出来
				if (mouse_decode(&mdec, i - 512) != 0) {
					//显示鼠标移动数据
					sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
					if ((mdec.btn & 0x01) != 0) { s[1] = 'L'; }
					if ((mdec.btn & 0x02) != 0) { s[3] = 'R'; }
					if ((mdec.btn & 0x04) != 0) { s[2] = 'C'; }
					putfonts8_asc_sht(sht_back, 32, 16, COL8_FFFFFF, COL8_008484, s, 15);

					mx += mdec.x;
					my += mdec.y;
					//控制鼠标的边界范围
					if (mx < 0) { mx = 0; }
					if (my < 0) { my = 0; }

					if (mx > binfo->scrnx -1) { mx = binfo->scrnx -1; }
					if (my > binfo->scrny -1) { my = binfo->scrny -1; }

					sprintf(s, "(%3d, %3d)", mx, my);
					putfonts8_asc_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_008484, s, 10);
					//刷新鼠标指针
					sheet_slide(sht_mouse, mx, my);
					//鼠标拖动窗口
					if (mdec.btn & 0x01 != 0){
						sheet_slide(sht_win, mx-80, my - 8);
					}
				}
			} else if (i <= 1){//光标定时器
				if (i !=0 ){
					timer_init(timer, &fifo, 0);
					cursor_c = COL8_000000;
				} else {//光标定时器
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


//创建窗口的函数
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

//标题函数
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

//重写输入函数  xy 显示位置的坐标  c  字符颜色(color)  b  背景颜色(back color)  s  字符串(string)  l  字符串长度(length)
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

//命令行窗口
void console_task(struct SHEET *sheet){
	struct TIMER *timer;
	struct TASK *task = task_now();
	int i, fifobuf[128], cursor_x = 16, cursor_c = COL8_000000;
	char s[2];

	fifo32_init(&task->fifo, 128, fifobuf, task);
	timer = timer_alloc();
	timer_init(timer, &task->fifo, 1);
	timer_settime(timer, 50);

	//显示提示符
	putfonts8_asc_sht(sheet, 8, 28, COL8_FFFFFF, COL8_000000, ">", 1);

	for (; ; ){
		io_cli();
		if (fifo32_status(&task->fifo) == 0){
			task_sleep(task);
			io_sti();
		} else {
			i = fifo32_get(&task->fifo);
			io_sti();
			//光标用定时器
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
			//键盘数据
			if (256 <=  i && i <= 511){
				//退格键
				if (i == 8 + 256){
					if (cursor_x > 16){
						putfonts8_asc_sht(sheet, cursor_x, 28, COL8_FFFFFF, COL8_000000, " ", 1);
						cursor_x -=8;
					}
				}else{
					//一般字符
					if (cursor_x < 240){
						s[0] = i - 256;
						s[1] = 0;
						putfonts8_asc_sht(sheet, cursor_x, 28, COL8_FFFFFF, COL8_000000, s, 1);
						cursor_x += 8;
					}
				}
			}
			//显示光标
			boxfill8(sheet->buf, sheet->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
			sheet_refresh(sheet, cursor_x, 28, cursor_x + 8, 44);			
		}
	}
}

//光标用定时器
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
