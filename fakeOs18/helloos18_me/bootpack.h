//��������еĺ�����������ɫ�Ķ����

/* asmhead.nas */
//�����趨��ʼֵ������		0x0ff0-0x0fff
struct BOOTINFO{
	char cyls; /* �����������̶�����Ϊֹ */
	char leds; /* ����ʱ���̵�LED��״̬ */
	char vmode; /* �Կ�ģʽΪ����λ��ɫ */
	char reserve;
	short scrnx, scrny; /* ����ֱ��� */
	char *vram;
};
#define ADR_BOOTINFO	0x00000ff0
#define	ADR_DISKIMG		0x00100000


/* naskfunc.nas */
/*����C����������һ�������ڱ���ļ��� �Լ���һ��
������������{}��ֻ��;��Ϊ �����ڱ���ļ��У���Ҫ��һ��
������д��ͬһ���ļ��� Ҳ��Ҫ�ڴ˴�����*/
void io_hlt(void);
void io_cli(void);
void io_sti(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);
//��GDT���ص�GDTR�Ĵ�����
void load_gdtr(int limit, int addr);
//��IDT���ص�IDRT�Ĵ�����
void load_idtr(int limit, int addr);
int load_cr0(void);
void store_cr0(int cr0);
void load_tr(int tr);
//��ʱ��
void asm_inthandler20(void);
//������
void asm_inthandler21(void);
void asm_inthandler27(void);
void asm_inthandler2c(void);
void farjmp(int eip, int cs);


//fifo.c ������
struct FIFO32{
	int *buf;
	int p, q, size, free, flags;
	struct TASK *task;
};

//��ʼ��������
void fifo32_init(struct FIFO32 *fifo, int size, int *buf, struct TASK *task);
//������
int fifo32_put(struct FIFO32 *fifo, int data);
//������
int fifo32_get(struct FIFO32 *fifo);
//�鿴������״̬
int fifo32_status(struct FIFO32 *fifo);


/* graphic.c ������*/
//������ɫ
void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
//�趨�������ݵ�λ��
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);
//��ʼ������
void init_screen8(char *vram, int x, int y);
//��ʾ�ַ�
void putfont8(char *vram, int xsize, int x, int y, char c, char *font);
//���������ַ���������ѡ����Ӧ���ַ�
void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s);
//���������ʽ
void init_mouse_cursor8(char *mouse, char bc);
//��ӡ���
void putblock8_8(char *vram, int vxsize, int pxsize, int pysize, int px0, int py0, char *buf, int bxsize);

//��ɫ�趨
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
//GDT���α��������� limit ����(�ε��ֽ���-1) base ��ַ  access ����Ȩ��
struct SEGMENT_DESCRIPTOR{
	//
	short limit_low, base_low;
	char base_mid, access_right;
	char limit_high, base_high;
};

//IDT���жϼ�¼���������� 
struct GATE_DESCRIPTOR{
	short offset_low, selector;
	char dw_count, access_right;
	short offset_high;
};

//��ʼ��GDTIDT
void init_gdtidt(void);
//limit �ε��ֽ���  base ��ַ   ar ����Ȩ��
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
//IDT
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);

//ָ���������ڴ��ַ����
#define ADR_IDT			0x0026f800
#define LIMIT_IDT		0x000007ff
#define ADR_GDT			0x00270000
#define LIMIT_GDT		0x0000ffff
#define ADR_BOTPAK		0x00280000
#define LIMIT_BOTPAK	0x0007ffff
#define AR_DATA32_RW	0x4092
#define AR_CODE32_ER	0x409a
#define AR_TSS32		0x0089
#define AR_INTGATE32	0x008e


//int.c    pic���
//��ʼ��PIC
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

//�����ж�
void inthandler21(int *esp);
void wait_KBC_sendready(void);
void init_keyboard(struct FIFO32 *fifo, int data0);
#define PORT_KEYDAT				0x0060
#define PORT_KEYCMD				0x0064

//����ж�
//���������
struct MOUSE_DEC{
	//buf ���ڴ������ phase ���ڼ�¼��ȡ�˶�������
	unsigned char buf[3], phase;
	//x,y ���ڴ�������ƶ���Ϣ�� btn ���ڴ�����İ�����Ϣ
	int x, y, btn;
};

void inthandler2c(int *esp);
void enable_mouse(struct FIFO32 *fifo, int data0, struct MOUSE_DEC *mdec);
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat);


//�ڴ���� memory.c
//�����ڴ�������� Լ32KB
#define MEMMAN_FREES	4090
#define MEMMAN_ADDR		0x003c0000

//�����ڴ������Ϣ��
struct FREEINFO{
	unsigned int addr, size;
};

//�����ڴ������
struct MEMMAN{
	int frees, maxfrees, lostsize, losts;
	struct FREEINFO free[MEMMAN_FREES];
};

//����ڴִ�С
unsigned int memtest(unsigned int start, unsigned int end);
//��ʼ���ڴ����
void memman_init(struct MEMMAN *man);
//��������ڴ��С
unsigned int memman_total(struct MEMMAN *man);
//�����ڴ�
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size);
//�ͷ��ڴ�
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size);
//����4k�ڴ�
unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size);
//�ͷ�4K�ڴ�
int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size);


//ͼ��������  sheet.c
//����ͼ����
struct SHEET{
	//ͼ�㻺����
	unsigned char *buf;
	//��С		col_inv ͸��ɫ
	int bxsize, bysize, vx0, vy0, col_inv, height, flags;
	//ͼ����������
	struct SHTCTL *ctl;
};
//���ͼ������
#define MAX_SHEETS		256
//����ͼ�������	
//varm��xsize��ysize��topΪ 4�ֽ�*4 = 16�ֽ� 
//*sheets	Ϊ 4�ֽ�*256 = 1024�ֽ�	
//sheets	Ϊ 32�ֽ�*256 = 8192�ֽ�
//�ϼ�Ϊ9232�ֽ�
struct SHTCTL{
	//vram �ĵ�ַ  �Լ� ͼ��map��ˢ��ʱ֮ˢ�¶�Ӧ��ͼ��
	unsigned char *vram, *map;
	//�����С �� �߶�
	int xsize, ysize, top;
	//���ڴ��256��vram��ַ
	struct SHEET *sheets[MAX_SHEETS];
	//���256��vram����
	struct SHEET sheets0[MAX_SHEETS];
};

//��ʼ��ͼ�����
struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize);
//��ȡ�����ɵ�δʹ��ͼ��
struct  SHEET *sheet_alloc(struct SHTCTL *ctl);
//�趨ͼ��Ļ�������С��͸��ɫ
void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv);
//�趨�װ�߶�
void sheet_updown(struct SHEET *sht, int height);
//ˢ�»���
void sheet_refresh( struct SHEET *sht, int bx0, int by0, int bx1, int by1);
//�ֲ�ˢ��
void sheet_refreshsub(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0, int h1);
//�����ƶ�ͼ��
void sheet_slide( struct SHEET *sht, int vx0, int vy0);
//�ͷ���ʹ�õ�ͼ���ڴ溯��
void sheet_free(struct SHEET *sht);


//��ʱ����� time.c
//��ʱ����������
#define		MAX_TIMER		500
//������ʱ������
struct TIMER{
	struct TIMER *next;
	unsigned int timeout, flags;
	struct FIFO32 *fifo;
	int data;
};

//������ʱ����������
struct TIMERCTL{
	//������
	unsigned int count, next;
	struct TIMER *t0;
	struct TIMER timers0[MAX_TIMER];
};

//�����ⲿ���õĶ�ʱ�������������
extern struct TIMERCTL timerctl;
//��ʼ����ʱ��
void init_pit(void);
//���䶨ʱ��
struct TIMER *timer_alloc(void);
//�ͷŶ�ʱ��
void timer_free(struct TIMER *timer);
//��ʼ����ʱ��
void timer_init(struct TIMER *timer, struct FIFO32 *fifo, int data);
//���ü�ʱ��ʱ���
void timer_settime(struct TIMER *timer, unsigned int timeout);
//�ж�
void inthandler20(int *esp);

//mtask.c
//�����������
#define MAX_TASKS		1000
//�����GDT�ļ��ſ�ʼ�����TSS
#define	TASK_GDT0		3
#define	MAX_TASK_LV		100
#define	MAX_TASKLEVELS	10

struct TSS32{
	//���ڴ���������������Ϣ
	int backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3;
	//32λ�Ĵ���
	int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
	//16λ�Ĵ���
	int es, cs, ss, ds, fs, gs;
	//�����������
	int ldtr, iomap;
};

struct TASK{
	//sel ���GDT���
	int sel, flags;
	//�������ȼ�
	int level, priority;
	//FIFO
	struct FIFO32 fifo;
	struct TSS32 tss;
};

struct TASKLEVEL{
	//�������е���������
	int running;
	//��ǵ�ǰ���е�����
	int now;
	struct TASK *tasks[MAX_TASK_LV];
};

struct TASKCTL{
	//���ڻ�е�level
	int now_lv;
	//���´������л�ʱ�Ƿ���Ҫ�ı�level
	int lv_change;
	struct TASKLEVEL level[MAX_TASKLEVELS];
	struct TASK tasks0[MAX_TASKS];
};
extern struct TIMER *task_timer;
struct TASK *task_init(struct MEMMAN *memman);
struct TASK *task_alloc(void);
void task_switch(void);
void task_run(struct TASK *task, int level, int priority);
void task_sleep(struct TASK *task);
void task_idle(void);
struct TASK *task_now(void);
