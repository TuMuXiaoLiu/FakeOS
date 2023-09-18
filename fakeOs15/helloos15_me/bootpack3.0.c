//要使用sprintg函数 必须在开头加上 #include

//添加函数引用文件
#include "bootpack.h"
#include <stdio.h>

//void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c);
struct TSS32{
	//用于存放任务设置相关信息
	int backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3;
	//32位寄存器
	int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
	//16位寄存器
	int es, cs, ss, ds, fs, gs;
	//任务设置相关
	int ldtr, iomap;
};

void task_b_main(struct SHEET *sht_back);

//程序入口 相当于Java中的Main函数
void HariMain(void){
	//声明BOOTINFO的对象并赋值 指针赋值
	struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
	//声明缓冲流
	struct FIFO32 fifo;
	//用于存放输出内容
	char s[40];
	//缓冲buff数组
	int fifobuf[128];

	//声明计时器对象
	struct TIMER *timer, *timer2, *timer3, *timer_ts;
	struct  MOUSE_DEC mdec;
	//鼠标坐标
	int mx, my, i, cursor_x, cursor_c, task_b_esp;

	//声明内存总数
	unsigned int memtotal;
	//声明内存管理对象
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;

	//图层管理
	struct SHTCTL *shtctl;
	//背景板  鼠标   窗口
	struct SHEET *sht_back, *sht_mouse, *sht_win;
	//背景板缓冲  鼠标缓冲
	unsigned char *buf_back, buf_mouse[256], *buf_win;
	//定义键盘输入
	static	char keytable[0x54] = {
		0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0, 0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0, 0, 'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0, 0, ']', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', ',', '.', '/', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.'
	};
	
	//声明多任务对象
	struct TSS32 tss_a, tss_b;
	//GDT对象
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)ADR_GDT;


	//初始化GDTIDT
	init_gdtidt();
	//初始化PIC
	init_pic();
	//解除 禁止中断 在IDT初始化时所有中断都会被禁止，需要在此解除所有的禁止中断 让CPU接受外部设备指令
	io_sti();

	//初始化缓冲区
	fifo32_init(&fifo, 128, fifobuf);
	//初始化定时器
	init_pit();
	//初始化鼠标键盘
	init_keyboard(&fifo, 256);
	enable_mouse(&fifo, 512, &mdec);

	//修改PIT和PIC1和键盘的中断，许可为11111000
	io_out8(PIC0_IMR, 0xf8);
	//修改PIC的IMR，以便接受来自鼠标的中断
	io_out8(PIC1_IMR, 0xef); /* 許可(11101111) */
	
	//初始化计时器缓冲等
	timer = timer_alloc();
	timer_init(timer, &fifo, 10);
	timer_settime(timer, 1000);
	
	timer2 = timer_alloc();
	timer_init(timer2, &fifo, 3);
	timer_settime(timer2, 300);

	timer3 = timer_alloc();
	timer_init(timer3, &fifo, 1);
	timer_settime(timer3, 50);

	timer_ts = timer_alloc();
	timer_init(timer_ts, &fifo, 2);
	timer_settime(timer_ts, 2);

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
	//获取背景板图层对象
	sht_back = sheet_alloc(shtctl);
	//获取鼠标图层对象
	sht_mouse = sheet_alloc(shtctl);
	//获取窗口图层对象
	sht_win = sheet_alloc(shtctl);

	//背景板缓冲
	buf_back = (unsigned char *)memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	//窗口缓冲
	buf_win = (unsigned char *)memman_alloc_4k(memman, 160 * 52);

	//背景板没有透明色
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);
	//鼠标的透明色号99
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
	//窗口
	sheet_setbuf(sht_win, buf_win, 160, 52, -1);

	//初始化屏幕
	init_screen8(buf_back, binfo->scrnx, binfo->scrny);
	//初始化鼠标  背景色号99 
	init_mouse_cursor8(buf_mouse, 99);
	//调用窗口函数
	make_window8(buf_win, 160, 52, "window");

	make_textbox8(sht_win, 8, 28, 144, 16, COL8_FFFFFF);
	cursor_x = 8;
	cursor_c = COL8_FFFFFF;

	//背景板位置
	sheet_slide(sht_back, 0, 0);
	
	//计算画面的中心坐标
	mx = (binfo->scrnx - 16) / 2; 
	my = (binfo->scrny - 28 - 16) / 2;
	//显示并勾勒鼠标
	sheet_slide(sht_mouse, mx, my);
	//显示窗口
	sheet_slide(sht_win, 80, 72);

	//设置背景板图层高度
	sheet_updown(sht_back, 0);
	//设置窗口图层高度
	sheet_updown(sht_win, 1);
	//设置鼠标图层高度
	sheet_updown(sht_mouse, 2);

	//打印鼠标坐标
	sprintf(s, "(%3d, %3d)", mx, my);
	putfonts8_asc_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_008484, s, 10);
	
	//打印 内存大小以及空余空间
	sprintf(s, "memory %dMB  free : %dKB", memtotal / (1024*1024), memman_total(memman) / 1024);
	putfonts8_asc_sht(sht_back, 0, 32, COL8_FFFFFF, COL8_008484, s, 40);

	//多任务处理
	tss_a.ldtr = 0;
	tss_a.iomap = 0x40000000;
	tss_b.ldtr = 0;
	tss_b.iomap = 0x40000000;
	//注册进GDT
	set_segmdesc(gdt + 3, 103, (int)&tss_a, AR_TSS32);
	set_segmdesc(gdt + 4, 103, (int)&tss_b, AR_TSS32);
	load_tr(3 * 8);
	task_b_esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 -8;
	tss_b.eip = (int)&task_b_main;
	tss_b.eflags = 0x00000202;//IF = 1
	tss_b.eax = 0;
	tss_b.ecx = 0;
	tss_b.edx = 0;
	tss_b.ebx = 0;
	tss_b.esp = task_b_esp;
	tss_b.ebp = 0;
	tss_b.esi = 0;
	tss_b.edi = 0;
	tss_b.es = 1 * 8;
	tss_b.cs = 2 * 8;
	tss_b.ss = 1 * 8;
	tss_b.ds = 1 * 8;
	tss_b.fs = 1 * 8;
	tss_b.gs = 1 * 8;

	
	for (;;) {
		io_cli();
		if (fifo32_status(&fifo) == 0) {
			io_stihlt();
		} else {
			i = fifo32_get(&fifo);
			io_sti();
			if (i == 2){
				farjmp(0, 4 * 8);
			}else if (256 <= i && i <= 511){	//键盘
				sprintf(s, "%02X", i - 256);
				putfonts8_asc_sht(sht_back, 0, 16, COL8_FFFFFF, COL8_008484, s, 2);
				if (i < 0x54 + 256){
					if (keytable[i - 256] != 0 && cursor_x < 144){
						//显示一个字符就前移一次光标
						s[0] = keytable[i - 256];
						s[1] = 0;
						putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, s, 1);
						cursor_x += 8;
					}
				}
				//退格键
				if (i == 256 + 0x0e && cursor_x > 8){
					//用空格键把光标消除后， 后移一位光标
					putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, " ", 1);
					cursor_x -= 8;
				}
				//光标再显示
				boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x +7, 43);
				sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
			}else if (512 <= i && i <= 767) {//鼠标
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
			} else if (i == 10) {//10秒定时器
				putfonts8_asc_sht(sht_back, 0, 64, COL8_FFFFFF, COL8_008484, "10[sec]", 7);
			}else if (i == 3){//3秒定时器
				putfonts8_asc_sht(sht_back, 0, 80, COL8_FFFFFF, COL8_008484, "03[sec]", 6);
			}else if (i <= 1){//光标定时器
				if (i !=0 ){
					timer_init(timer3, &fifo, 0);
					cursor_c = COL8_000000;
				} else {//光标定时器
					timer_init(timer3, &fifo, 1);
					cursor_c = COL8_FFFFFF;
				}
				timer_settime(timer3, 50);
				boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x+8, 44);
				sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
			}
		}
	}
}


//创建窗口的函数
void make_window8(unsigned char *buf, int xsize, int ysize, char *title){
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


//重写输入函数  xy 显示位置的坐标  c  字符颜色(color)  b  背景颜色(back color)  s  字符串(string)  l  字符串长度(length)
void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l){
	boxfill8(sht->buf, sht->bxsize, b, x, y, x+1*8-1, y+15);
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
	boxfill8(sht->buf, sht->bxsize, c, x0 - 1, y0 - 1, x1 + 0, y1 + 0);
	return;
}

void task_b_main(struct SHEET *sht_back){
	struct FIFO32 fifo;
	struct TIMER *timer_ts;
	int i, fifobuf[128];

	fifo32_init(&fifo, 128, fifobuf);
	timer_ts = timer_alloc();
	timer_init(timer_ts, &fifo, 1);
	timer_settime(timer_ts, 2);

	for (; ; ){
		io_cli();
		if (fifo32_status(&fifo) == 0){
			io_stihlt();
		} else {
			i = fifo32_get(&fifo);
			io_sti();
			if (i == 1){
				//返回任务A
				farjmp(0, 3 * 8);
				timer_settime(timer_ts, 2);
			}
		}
	}
}
