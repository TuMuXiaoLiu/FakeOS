//要使用sprintg函数 必须在开头加上 #include
//后面需要加上内容
#include <stdio.h>


/*告诉C编译器，有一个函数在别的文件里 自己找一下
函数声明不用{}，只用;意为 函数在别的文件中，需要找一下
即便是写在同一个文件中 也需要在此处声明*/
void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);

//设置颜色
void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
//设定界面内容的位置
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);
//初始化界面
void init_screen(char *vram, int x, int y);
//显示字符
void putfont8(char *vram, int xsize, int x, int y, char c, char *font);
//用于链接字符集并从中选出对应的字符
void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s);
//勾勒鼠标样式
void init_mouse_cursor8(char *mouse, char bc);
//打印鼠标
void putblock8_8(char *vram, int vxsize, int pxsize, int pysize, int px0, int py0, char *buf, int bxsize);





#define COL8_000000		0
#define COL8_FF0000		1
#define COL8_00FF00		2
#define COL8_FFFF00		3
#define COL8_0000FF		4
#define COL8_FF00FF		5
#define COL8_00FFFF		6
#define COL8_FFFFFF		7
#define COL8_C6C6C6		8
#define COL8_840000		9
#define COL8_008400		10
#define COL8_848400		11
#define COL8_000084		12
#define COL8_840084		13
#define COL8_008484		14
#define COL8_848484		15

//用于设定初始值的类体
struct BOOTINFO
{
	char cyls, leds, vmode, reserve;
	short scrnx, scrny;
	char *vram;
};

//GDT（段表）的类体 limit 上限(段的字节数-1) base 基址  access 访问权限
struct SEGMENT_DESCRIPTOR
{
	//
	short limit_low, base_low;
	char base_mid, access_right;
	char limit_high, base_high;
};

//IDT（中断记录表）的类体 
struct GATE_DESCRIPTOR
{
	short offset_low, selector;
	char dw_count, access_right;
	short offset_high;
};



//初始化GDTIDT
void init_gdtidt(void);
//limit 段的字节数  base 基址   ar 访问权限
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
//IDT
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);
//将GDT加载到GDTR寄存器中
void load_gdtr(int limit, int addr);
//将IDT加载到IDRT寄存器中
void load_idtr(int limit, int addr);


//程序入口 相当于Java中的Main函数
void HariMain(void)
{
	//声明BOOTINFO的对象并赋值 指针赋值
	struct BOOTINFO *binfo = (struct BOOTINFO *)0xff0;

	//设定调色板
	init_palette();

	//调用画界面方法并入参
	init_screen(binfo->vram, binfo->scrnx, binfo->scrny);
	
	//显示文字
	//putfonts8_asc(binfo->vram, binfo->scrnx,  8,  8, COL8_FFFFFF, "ABC 123");
	//putfonts8_asc(binfo->vram, binfo->scrnx, 31, 31, COL8_000000, "CCL OS.");
	//putfonts8_asc(binfo->vram, binfo->scrnx, 30, 30, COL8_FFFFFF, "CCL OS.");

	//声明变量 s,用于输出变量的值
	//char s[40];
	//在屏幕上显示变量值
	//%d 为格式的一种 将内容转换成十进制
	//sprintf(s, "scrnx = %d", binfo->scrnx);
	//putfonts8_asc(binfo->vram, binfo->scrnx, 16, 64, COL8_FFFFFF, s);

	//显示鼠标
	int mx, my;
	//用于存放鼠标
	char mcursor[256];
	//计算画面的中心坐标
	mx = (binfo->scrnx - 16) / 2; 
	my = (binfo->scrny - 28 - 16) / 2;
	init_mouse_cursor8(mcursor, COL8_008484);
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);

	for(;;){
		io_hlt();	
	}
}

void init_palette(void)
{
	//C语言中的static char 语句只能用于数据，相当于汇编中的DB指令
	static unsigned char table_rgb[16 * 3] = 
	{
		0x00, 0x00, 0x00,	/*  0:黑 */
		0xff, 0x00, 0x00,	/*  1:梁红 */
		0x00, 0xff, 0x00,	/*  2:亮绿 */
		0xff, 0xff, 0x00,	/*  3:亮黄 */
		0x00, 0x00, 0xff,	/*  4:亮蓝 */
		0xff, 0x00, 0xff,	/*  5:亮紫 */
		0x00, 0xff, 0xff,	/*  6:浅亮蓝 */
		0xff, 0xff, 0xff,	/*  7:白 */
		0xc6, 0xc6, 0xc6,	/*  8:亮灰 */
		0x84, 0x00, 0x00,	/*  9:暗红 */
		0x00, 0x84, 0x00,	/* 10:暗绿 */
		0x84, 0x84, 0x00,	/* 11:暗黄 */
		0x00, 0x00, 0x84,	/* 12:暗青 */
		0x84, 0x00, 0x84,	/* 13:暗紫 */
		0x00, 0x84, 0x84,	/* 14:浅暗蓝 */
		0x84, 0x84, 0x84	/* 15:暗灰 */
	};//此处分号是定义数组的结束分号 必须要加
	set_palette(0, 15, table_rgb);
}

void set_palette(int start, int end, unsigned char *rgb)
{
	int i, eflags;
	//记录中断许可标识的值
	eflags = io_load_eflags();
	//将中断许可标识置为0，禁止中断
	io_cli();
	io_out8(0x03c8, start);
	for(i = start;i <= end; i++){
		io_out8(0x03c9, rgb[0] / 4);
		io_out8(0x03c9, rgb[1] / 4);
		io_out8(0x03c9, rgb[2] / 4);
		rgb += 3;
	}
	//复原中断许可标识
	io_store_eflags(eflags);
	return;
}

//画界面
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1)
{
	int x, y;
	for (y = y0;y <= y1; y++)
	{
		for (x = x0; x <= x1; x++)
		{
			vram[y * xsize + x] = c;
		}
	}
	return;
}

//初始化屏幕
void init_screen(char *vram, int x, int y)
{
	boxfill8(vram, x, COL8_008484,  0, 0,      x -  1, y - 29);
	boxfill8(vram, x, COL8_C6C6C6,  0, y - 28, x -  1, y - 28);
	boxfill8(vram, x, COL8_FFFFFF,  0, y - 27, x -  1, y - 27);
	boxfill8(vram, x, COL8_C6C6C6,  0, y - 26, x -  1, y -  1);

	boxfill8(vram, x, COL8_FFFFFF,  3, y - 24, 59, y - 24);
	boxfill8(vram, x, COL8_FFFFFF,  2, y - 24,  2, y -  4);
	boxfill8(vram, x, COL8_848484,  3, y -  4, 59, y -  4);
	boxfill8(vram, x, COL8_848484, 59, y - 23, 59, y -  5);
	boxfill8(vram, x, COL8_000000,  2, y -  3, 59, y -  3);
	boxfill8(vram, x, COL8_000000, 60, y - 24, 60, y -  3);

	boxfill8(vram, x, COL8_848484, x - 47, y - 24, x -  4, y - 24);
	boxfill8(vram, x, COL8_848484, x - 47, y - 23, x - 47, y -  4);
	boxfill8(vram, x, COL8_FFFFFF, x - 47, y -  3, x -  4, y -  3);
	boxfill8(vram, x, COL8_FFFFFF, x -  3, y - 24, x -  3, y -  3);
	return;
}

//打印文字
void putfont8(char *vram, int xsize, int x, int y, char c, char *font)
{
	int i;
	char *p, d /* data */;
	for (i = 0; i < 16; i++) {
		p = vram + (y + i) * xsize + x;
		d = font[i];
		if ((d & 0x80) != 0) { p[0] = c; }
		if ((d & 0x40) != 0) { p[1] = c; }
		if ((d & 0x20) != 0) { p[2] = c; }
		if ((d & 0x10) != 0) { p[3] = c; }
		if ((d & 0x08) != 0) { p[4] = c; }
		if ((d & 0x04) != 0) { p[5] = c; }
		if ((d & 0x02) != 0) { p[6] = c; }
		if ((d & 0x01) != 0) { p[7] = c; }
	}
	return;
}

//传入需要打印的文字 并从给定的字符库中选取对应的文字显示
void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s)
{
	//外部字符集文件
	extern char hankaku[4096];
	/* C语言中，字符串都是以0x00结尾 */
	for (; *s != 0x00; s++) {
		//调用显示文字的方法
		putfont8(vram, xsize, x, y, c, hankaku + *s * 16);
		x += 8;
	}
	return;
}

//勾勒鼠标
void init_mouse_cursor8(char *mouse, char bc)
{
	//定义鼠标数组
	static char cursor[16][16] = {
		"**************..",
		"*OOOOOOOOOOO*...",
		"*OOOOOOOOOO*....",
		"*OOOOOOOOO*.....",
		"*OOOOOOOO*......",
		"*OOOOOOO*.......",
		"*OOOOOOO*.......",
		"*OOOOOOOO*......",
		"*OOOO**OOO*.....",
		"*OOO*..*OOO*....",
		"*OO*....*OOO*...",
		"*O*......*OOO*..",
		"**........*OOO*.",
		"*..........*OOO*",
		"............*OO*",
		".............***"
	};
	//用于画鼠标
	int x, y;
	//循环打印鼠标
	for (y = 0; y < 16; y++) {
		for (x = 0; x < 16; x++) {
			if (cursor[y][x] == '*') {
				mouse[y * 16 + x] = COL8_000000;
			}
			if (cursor[y][x] == 'O') {
				mouse[y * 16 + x] = COL8_FFFFFF;
			}
			if (cursor[y][x] == '.') {
				mouse[y * 16 + x] = bc;
			}
		}
	}
	return;
}

//打印鼠标
void putblock8_8(char *vram, int vxsize, int pxsize, int pysize, int px0, int py0, char *buf, int bxsize)
{
	int x, y;
	for(y = 0; y < pysize; y++)
	{
		for (x = 0; x < pxsize; x++)
		{
			vram[(py0 + y) * vxsize + (px0 + x)] = buf[y * bxsize + x];
		}
	}
}

//初始化GDT IDT
void init_gdtidt(void)
{
	//GDT对象
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)0x00270000;
	//IDT对象
	struct GATE_DESCRIPTOR *idt = (struct GATE_DESCRIPTOR *)0x0026f800;
	int i;
	//GDT 的初始化
	for (i = 0;i < 8192; i++)
	{
		//将所有的 段 的上限limit 基址 base 访问权限 access 都设为0
		set_gatedesc(gdt + i, 0, 0, 0);
	}
	//设置段号1的上限为4G 基址是0  段属性为4092
	set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, 0x4092);
	//设置段号2的上限为512KB 基址是280000 段属性为 409a
	set_segmdesc(gdt + 2, 0x0007ffff, 0x00280000, 0x409a);
	//通过汇编给段寄存器赋值
	load_gdtr(0xffff, 0x00270000);

	//IDT初始化
	for (i = 0; i < 256; i++)
	{
		//设置中断号的相关数据
		set_gatedesc(idt + i, 0, 0, 0);
	}
	//通过汇编给中断寄存器赋值
	load_idtr(0x7ff, 0x0026f800);
	return;
}

/*limit 段的字节数  base 基址   ar 访问权限*/
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar)
{
	//如果上限大于
	if (limit > 0xfffff) {
		ar |= 0x8000; /* G_bit = 1 */
		limit /= 0x1000;
	}
	sd->limit_low    = limit & 0xffff;
	sd->base_low     = base & 0xffff;
	sd->base_mid     = (base >> 16) & 0xff;
	sd->access_right = ar & 0xff;
	sd->limit_high   = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);
	sd->base_high    = (base >> 24) & 0xff;
	return;
}

//IDT
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar)
{
	gd->offset_low   = offset & 0xffff;
	gd->selector     = selector;
	gd->dw_count     = (ar >> 8) & 0xff;
	gd->access_right = ar & 0xff;
	gd->offset_high  = (offset >> 16) & 0xffff;
	return;
}
