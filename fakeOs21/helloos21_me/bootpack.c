//添加函数引用文件
#include "bootpack.h"
#include <stdio.h>

#define	KEYCMD_LED		0xed

void HariMain(void){//程序入口 相当于Java中的Main函数
	struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;//声明BOOTINFO的对象并赋值 指针赋值
	struct SHTCTL *shtctl;//图层管理
	char s[40];//用于存放输出内容
	struct FIFO32 fifo, keycmd;//声明缓冲流
	int fifobuf[128], keycmd_buf[32];//缓冲buff数组
	int mx, my, i, cursor_x, cursor_c;//鼠标坐标
	unsigned int memtotal;//声明内存总数
	struct MOUSE_DEC mdec;
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;//声明内存管理对象
	unsigned char *buf_back, buf_mouse[256], *buf_win, *buf_cons;//背景板缓冲  鼠标缓冲等
	struct SHEET *sht_back, *sht_mouse, *sht_win, *sht_cons;//背景板  鼠标   窗口
	struct TASK *task_a, *task_cons;//声明多任务对象
	struct TIMER *timer;//声明计时器对象
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
	
	init_gdtidt();//初始化GDTIDT
	init_pic();//初始化PIC
	io_sti();//解除 禁止中断 在IDT初始化时所有中断都会被禁止，需要在此解除所有的禁止中断 让CPU接受外部设备指令
	fifo32_init(&fifo, 128, fifobuf, 0);//初始化缓冲区
	init_pit();//初始化定时器
	init_keyboard(&fifo, 256);//初始化鼠标键盘
	enable_mouse(&fifo, 512, &mdec);
	io_out8(PIC0_IMR, 0xf8);//修改PIT和PIC1和键盘的中断，许可为11111000
	io_out8(PIC1_IMR, 0xef); //修改PIC的IMR，以便接受来自鼠标的中断/* 許可(11101111) */
	fifo32_init(&keycmd, 32, keycmd_buf, 0);
	memtotal = memtest( 0x00400000, 0xbfffffff);
	memman_init(memman);
	memman_free(memman, 0x00001000, 0x0009e000);
	memman_free(memman, 0x00400000, memtotal - 0x00400000);
	init_palette();	//设定调色板
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);//调用画界面方法并入参-> 启动图层管理
	task_a = task_init(memman);
	fifo.task = task_a;
	task_run(task_a, 1, 2);
	//sht_back
	sht_back = sheet_alloc(shtctl);//获取背景板图层对象
	buf_back = (unsigned char *)memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);//背景板缓冲
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);//背景板没有透明色
	init_screen8(buf_back, binfo->scrnx, binfo->scrny);//初始化屏幕
	//sht_cons
	sht_cons = sheet_alloc(shtctl);
	buf_cons = (unsigned char *)memman_alloc_4k(memman, 256 * 165);
	sheet_setbuf(sht_cons, buf_cons, 256, 165, -1);//无透明色
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
	sht_win = sheet_alloc(shtctl);//获取窗口图层对象
	buf_win = (unsigned char *)memman_alloc_4k(memman, 160 * 52);//窗口缓冲
	sheet_setbuf(sht_win, buf_win, 144, 52, -1);//窗口
	make_window8(buf_win, 144, 52, "task_a", 1);//调用窗口函数
	make_textbox8(sht_win, 8, 28, 128, 16, COL8_FFFFFF);
	cursor_x = 8;
	cursor_c = COL8_FFFFFF;
	timer = timer_alloc();//初始化光标计时器
	timer_init(timer, &fifo, 1);
	timer_settime(timer, 50);
	//sht_mouse
	sht_mouse = sheet_alloc(shtctl);//获取鼠标图层对象
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);//鼠标的透明色号99
	init_mouse_cursor8(buf_mouse, 99);//初始化鼠标  背景色号99 
	mx = (binfo->scrnx - 16) / 2; 
	my = (binfo->scrny - 28 - 16) / 2;

	sheet_slide(sht_back,  0,  0);//背景板位置
	sheet_slide(sht_cons, 32,  4);//显示窗口
	sheet_slide(sht_win,  64, 56);
	sheet_slide(sht_mouse, mx, my);	//显示并勾勒鼠标
	sheet_updown(sht_back,	0);//设置背景板图层高度
	sheet_updown(sht_cons,	1);//设置窗口图层高度
	sheet_updown(sht_win,	2);
	sheet_updown(sht_mouse, 3);//设置鼠标图层高度
	fifo32_put(&keycmd, KEYCMD_LED);//防止和键盘当前状态冲突，在一开始就进行设置
	fifo32_put(&keycmd, key_leds);

	for (;;) {
		if (fifo32_status(&keycmd) > 0 && keycmd_wait < 0){//如果存在向键盘控制器发送的数据，则发送它
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
			if (256 <= i && i <= 511){//键盘
				//将按键编码转为字符编码
				if (i < 0x80 + 256){if (key_shift == 0){ s[0] = keytable0[i - 256]; } else { s[0] = keytable1[i - 256]; } } else { s[0] = 0; }
				//当输入的字符为英文字母时
				if ('A' <= s[0] && s[0] <= 'Z'){ if (((key_leds & 4) == 0 && key_shift == 0) || ((key_leds & 4) != 0 && key_shift != 0)){ s[0] += 0x20; } }//将大写字母转为小写字母
				if (s[0] != 0){//一般字符
					if (key_to == 0){//发送给任务A
						if (cursor_x < 128){
							//显示一个字符就前移一次光标
							s[1] = 0;
							putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, s, 1);
							cursor_x += 8;
						}
					} else { fifo32_put(&task_cons->fifo, s[0] + 256); }//发送给命令行窗口
				}
				if (i == 256 + 0x0e){//退格键
					if (key_to == 0){//发送给任务A
						if (cursor_x > 8){//用空格键把光标消除后， 后移一位光标
							putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, " ", 1);
							cursor_x -= 8;
						}
					} else { fifo32_put(&task_cons->fifo, 8 + 256); }//发送给命令行
				}
				//回车键  发送至命令窗口
				if (i == 256 + 0x1c){ if (key_to != 0){ fifo32_put(&task_cons->fifo, 10 + 256); } }
				//TAB键
				if (i == 256 + 0x0f){
					if (key_to == 0){
						key_to = 1;
						make_wtitle8(buf_win, sht_win->bxsize, "task_a", 0);
						make_wtitle8(buf_cons, sht_cons->bxsize, "console", 1);
						cursor_c = -1;//不显示光标
						boxfill8(sht_win->buf, sht_win->bxsize, COL8_FFFFFF, cursor_x, 28, cursor_x + 7, 43);
						fifo32_put(&task_cons->fifo, 2);//命令行窗口光标闪烁
					} else {
						key_to = 0;
						make_wtitle8(buf_win, sht_win->bxsize, "task_a", 1);
						make_wtitle8(buf_cons, sht_cons->bxsize, "console", 0);
						cursor_c = COL8_000000;//显示光标
						fifo32_put(&task_cons->fifo, 3);//命令行窗口光标闪烁关
					}
					sheet_refresh(sht_win, 0, 0, sht_win->bxsize, 21);
					sheet_refresh(sht_cons, 0, 0, sht_cons->bxsize, 21);
				}
				
				if (i == 256 + 0x2a){ key_shift |= 1;}//左shift ON
				if (i == 256 + 0x36){ key_shift |= 2;}//右shift ON
				if (i == 256 + 0xaa){ key_shift &= ~1;}//左shift OFF
				if (i == 256 + 0xb6){ key_shift &= ~2;}//右shift off
				
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
				if (i == 256 + 0xfa){ keycmd_wait = -1;}//键盘成功接收到数据
				if (i == 256 + 0xfe){ //键盘没有成功接收到数据
					wait_KBC_sendready();
					io_out8(PORT_KEYDAT, keycmd_wait);
				}
				if (cursor_c >= 0){ boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x +7, 43); }//光标再显示 
				sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);//刷新
			} else if (512 <= i && i <= 767) {//鼠标
				if (mouse_decode(&mdec, i - 512) != 0) {//鼠标的三个字节数据都齐了，显示出来
					mx += mdec.x;
					my += mdec.y;
					//控制鼠标的边界范围
					if (mx < 0) { mx = 0; }
					if (my < 0) { my = 0; }
					if (mx > binfo->scrnx -1) { mx = binfo->scrnx -1; }
					if (my > binfo->scrny -1) { my = binfo->scrny -1; }
					sheet_slide(sht_mouse, mx, my);//刷新鼠标指针
					if ((mdec.btn & 0x01) != 0){ sheet_slide(sht_win, mx-80, my - 8); }//鼠标拖动窗口
				}
			} else if (i <= 1){//光标定时器
				if (i !=0 ){
					timer_init(timer, &fifo, 0);
					if (cursor_c >= 0){ cursor_c = COL8_000000; }
				} else {//光标定时器
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
