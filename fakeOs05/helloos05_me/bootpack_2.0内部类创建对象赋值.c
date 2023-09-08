/*����C����������һ�������ڱ���ļ��� �Լ���һ��
������������{}��ֻ��;��Ϊ �����ڱ���ļ��У���Ҫ��һ��
������д��ͬһ���ļ��� Ҳ��Ҫ�ڴ˴�����*/
void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);

//������ɫ
void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
//������
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

//�ڲ��� �����趨��ʼֵ������
struct BOOTINFO
{
	char cyls, leds, vmode, reserve;
	short scrnx, scrny;
	char *vram;
};

//������� �൱��Java�е�Main����
void HariMain(void){
	//����
	char *vram;
	//����Ŀ��
	int xsize, ysize;
	//����BOOTINFO�Ķ��� ָ�븳ֵ
	struct BOOTInfO *binfo;

	//�趨��ɫ��
	init_palette();

	//BOOTINFO����ֵ
	binfo = (struct BOOTINFO *)0x0ffx;
	xsixe = (*binfo).scrnx;
	ysize = (*binfo).scrny;
	vram = (*binfo).vram;


	for(;;){
		io_hlt();	
	}
}

void init_palette(void){
	//C�����е�static char ���ֻ���������ݣ��൱�ڻ���е�DBָ��
	static unsigned char table_rgb[16 * 3] = {
		0x00, 0x00, 0x00,	/*  0:�� */
		0xff, 0x00, 0x00,	/*  1:���� */
		0x00, 0xff, 0x00,	/*  2:���� */
		0xff, 0xff, 0x00,	/*  3:���� */
		0x00, 0x00, 0xff,	/*  4:���� */
		0xff, 0x00, 0xff,	/*  5:���� */
		0x00, 0xff, 0xff,	/*  6:ǳ���� */
		0xff, 0xff, 0xff,	/*  7:�� */
		0xc6, 0xc6, 0xc6,	/*  8:���� */
		0x84, 0x00, 0x00,	/*  9:���� */
		0x00, 0x84, 0x00,	/* 10:���� */
		0x84, 0x84, 0x00,	/* 11:���� */
		0x00, 0x00, 0x84,	/* 12:���� */
		0x84, 0x00, 0x84,	/* 13:���� */
		0x00, 0x84, 0x84,	/* 14:ǳ���� */
		0x84, 0x84, 0x84	/* 15:���� */
	};//�˴��ֺ��Ƕ�������Ľ����ֺ� ����Ҫ��
	set_palette(0, 15, table_rgb);
}

void set_palette(int start, int end, unsigned char *rgb){
	int i, eflags;
	//��¼�ж���ɱ�ʶ��ֵ
	eflags = io_load_eflags();
	//���ж���ɱ�ʶ��Ϊ0����ֹ�ж�
	io_cli();
	io_out8(0x03c8, start);
	for(i = start;i <= end; i++){
		io_out8(0x03c9, rgb[0] / 4);
		io_out8(0x03c9, rgb[1] / 4);
		io_out8(0x03c9, rgb[2] / 4);
		rgb += 3;
	}
	//��ԭ�ж���ɱ�ʶ
	io_store_eflags(eflags);
}

//������
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