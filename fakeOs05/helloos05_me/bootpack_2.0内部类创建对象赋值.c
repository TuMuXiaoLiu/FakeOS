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
//画界面
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);

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

//内部类 用于设定初始值的类体
struct BOOTINFO
{
	char cyls, leds, vmode, reserve;
	short scrnx, scrny;
	char *vram;
};

//程序入口 相当于Java中的Main函数
void HariMain(void){
	//桌面
	char *vram;
	//界面的宽高
	int xsize, ysize;
	//声明BOOTINFO的对象 指针赋值
	struct BOOTInfO *binfo;

	//设定调色板
	init_palette();

	//BOOTINFO对象赋值
	binfo = (struct BOOTINFO *)0x0ffx;
	xsixe = (*binfo).scrnx;
	ysize = (*binfo).scrny;
	vram = (*binfo).vram;


	for(;;){
		io_hlt();	
	}
}

void init_palette(void){
	//C语言中的static char 语句只能用于数据，相当于汇编中的DB指令
	static unsigned char table_rgb[16 * 3] = {
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

void set_palette(int start, int end, unsigned char *rgb){
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
}

//画界面
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1){
	int x, y;
	for (y = y0;y <= y1; y++)
	{
		for (x = x0; x <= x1; x++)
		{
			vram[y * xsize + x] = c;
		}
	}
}