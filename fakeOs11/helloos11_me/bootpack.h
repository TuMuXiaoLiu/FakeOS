//存放了所有的函数声明、颜色的定义等

/* asmhead.nas */
//用于设定初始值的类体		0x0ff0-0x0fff
struct BOOTINFO
{
	char cyls; /* 启动区读磁盘读到此为止 */
	char leds; /* 启动时键盘的LED的状态 */
	char vmode; /* 显卡模式为多少位彩色 */
	char reserve;
	short scrnx, scrny; /* 画面分辨率 */
	char *vram;
};
#define ADR_BOOTINFO	0x00000ff0


/* naskfunc.nas */
/*告诉C编译器，有一个函数在别的文件里 自己找一下
函数声明不用{}，只用;意为 函数在别的文件中，需要找一下
即便是写在同一个文件中 也需要在此处声明*/
void io_hlt(void);
void io_cli(void);
void io_sti(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);
//将GDT加载到GDTR寄存器中
void load_gdtr(int limit, int addr);
//将IDT加载到IDRT寄存器中
void load_idtr(int limit, int addr);
//鼠标键盘
void asm_inthandler21(void);
void asm_inthandler27(void);
void asm_inthandler2c(void);


//fifo.c 缓冲类
struct FIFO8
{
	unsigned char *buf;
	int p, q, size, free, flags;
};
//初始化缓冲区
void fifo8_init(struct FIFO8 *fifo, int size, unsigned char *buf);
//存数据
int fifo8_put(struct FIFO8 *fifo, unsigned char data);
//读数据
int fifo8_get(struct FIFO8 *fifo);
//查看缓冲区状态
int fifo8_status(struct FIFO8 *fifo);


/* graphic.c 画画用*/
//设置颜色
void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
//设定界面内容的位置
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);
//初始化界面
void init_screen8(char *vram, int x, int y);
//显示字符
void putfont8(char *vram, int xsize, int x, int y, char c, char *font);
//用于链接字符集并从中选出对应的字符
void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s);
//勾勒鼠标样式
void init_mouse_cursor8(char *mouse, char bc);
//打印鼠标
void putblock8_8(char *vram, int vxsize, int pxsize, int pysize, int px0, int py0, char *buf, int bxsize);

//颜色设定
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


/* dsctbl.c */
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

//指定各个的内存地址区域
#define ADR_IDT			0x0026f800
#define LIMIT_IDT		0x000007ff
#define ADR_GDT			0x00270000
#define LIMIT_GDT		0x0000ffff
#define ADR_BOTPAK		0x00280000
#define LIMIT_BOTPAK	0x0007ffff
#define AR_DATA32_RW	0x4092
#define AR_CODE32_ER	0x409a
#define AR_INTGATE32	0x008e


//int.c    pic相关
//初始化PIC
void init_pic(void);
void inthandler27(int *esp);
#define PIC0_ICW1		0x0020
#define PIC0_OCW2		0x0020
#define PIC0_IMR		0x0021
#define PIC0_ICW2		0x0021
#define PIC0_ICW3		0x0021
#define PIC0_ICW4		0x0021
#define PIC1_ICW1		0x00a0
#define PIC1_OCW2		0x00a0
#define PIC1_IMR		0x00a1
#define PIC1_ICW2		0x00a1
#define PIC1_ICW3		0x00a1
#define PIC1_ICW4		0x00a1

//键盘中断
void inthandler21(int *esp);
void wait_KBC_sendready(void);
void init_keyboard(void);
extern struct FIFO8 keyfifo;
#define PORT_KEYDAT				0x0060
#define PORT_KEYCMD				0x0064

//鼠标中断
//声明鼠标类
struct MOUSE_DEC
{
	//buf 用于存放数据 phase 用于记录读取了多少数据
	unsigned char buf[3], phase;
	//x,y 用于存放鼠标的移动信息， btn 用于存放鼠标的按键信息
	int x, y, btn;
};
void inthandler2c(int *esp);
void enable_mouse(struct MOUSE_DEC *mdec);
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat);
extern struct FIFO8 mousefifo;


//内存管理 memory.c
//定义内存可用条数 约32KB
#define MEMMAN_FREES	4090
#define MEMMAN_ADDR		0x003c0000
//定义内存可用信息类
struct FREEINFO
{
	unsigned int addr, size;
};
//定义内存管理类
struct MEMMAN
{
	int frees, maxfrees, lostsize, losts;
	struct FREEINFO free[MEMMAN_FREES];
};
//检测内粗大小
unsigned int memtest(unsigned int start, unsigned int end);
//初始化内存管理
void memman_init(struct MEMMAN *man);
//计算可用内存大小
unsigned int memman_total(struct MEMMAN *man);
//分配内存
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size);
//释放内存
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size);
//分配4k内存
unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size);
//释放4K内存
int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size);


//图层管理相关  sheet.c
//定义图层类
struct SHEET
{
	//图层缓冲区
	unsigned char *buf;
	//大小		col_inv 透明色
	int bxsize, bysize, vx0, vy0, col_inv, height, flags;
	//图层管理类对象
	struct SHTCTL *ctl;
};
//最大图层数量
#define MAX_SHEETS		256
//定义图层管理类	
//varm、xsize、ysize、top为 4字节*4 = 16字节 
//*sheets	为 4字节*256 = 1024字节	
//sheets	为 32字节*256 = 8192字节
//合计为9232字节
struct SHTCTL
{
	//vram 的地址  以及 图层map，刷新时之刷新对应的图层
	unsigned char *vram, *map;
	//画面大小 和 高度
	int xsize, ysize, top;
	//用于存放256个vram地址
	struct SHEET *sheets[MAX_SHEETS];
	//存放256个vram对象
	struct SHEET sheets0[MAX_SHEETS];
};
//初始化图层管理
struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize);
//获取新生成的未使用图层
struct  SHEET *sheet_alloc(struct SHTCTL *ctl);
//设定图层的缓冲区大小和透明色
void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv);
//设定底板高度
void sheet_updown(struct SHEET *sht, int height);
//刷新画面
void sheet_refresh( struct SHEET *sht, int bx0, int by0, int bx1, int by1);
//局部刷新
void sheet_refreshsub(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0, int h1);
//左右移动图层
void sheet_slide( struct SHEET *sht, int vx0, int vy0);
//释放已使用的图层内存函数
void sheet_free(struct SHEET *sht);

