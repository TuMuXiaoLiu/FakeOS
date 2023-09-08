//��������еĺ�����������ɫ�Ķ����

#define ADR_BOOTINFO	0x00000ff0
/* asmhead.nas */
//�����趨��ʼֵ������
struct BOOTINFO
{
	char cyls, leds, vmode, reserve;
	short scrnx, scrny;
	char *vram;
};

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
//������
void asm_inthandler21(void);
void asm_inthandler2c(void);

/* graphic.c */
//������ɫ
void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
//�趨�������ݵ�λ��
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);
//��ʼ������
void init_screen(char *vram, int x, int y);
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
struct SEGMENT_DESCRIPTOR
{
	//
	short limit_low, base_low;
	char base_mid, access_right;
	char limit_high, base_high;
};

//IDT���жϼ�¼���������� 
struct GATE_DESCRIPTOR
{
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
//��GDT���ص�GDTR�Ĵ�����
void load_gdtr(int limit, int addr);
//��IDT���ص�IDRT�Ĵ�����
void load_idtr(int limit, int addr);
#define ADR_IDT			0x0026f800
#define LIMIT_IDT		0x000007ff
#define ADR_GDT			0x00270000
#define LIMIT_GDT		0x0000ffff
#define ADR_BOTPAK		0x00280000
#define LIMIT_BOTPAK	0x0007ffff
#define AR_DATA32_RW	0x4092
#define AR_CODE32_ER	0x409a
#define AR_INTGATE32	0x008e

//int.c pic���
//��ʼ��PIC
void init_pic(void);
//�����ж�
void inthandler21(int *esp);
//����ж�
void inthandler2c(int *esp);

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