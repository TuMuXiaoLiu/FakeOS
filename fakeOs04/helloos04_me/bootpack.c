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
//void write_mem8(int addr, int data);

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

void HariMain(void){

	/*����һ������i i��32λ����*/
	int i;	
	//����P��ʹ��Byte���͵�ַ
	//char *p;

	//�趨��ɫ��
	init_palette();

	//����ַ��ֵ��ȥ
	//char *p = (char *)0xa0000;

	/*ʹ���·���C�������
	for(i = 0xa0000;i <= 0xaffff; i++){
		//��Ч�� MOV BYTE[i],15
		//write_mem8(i,15);
		//����Ļ���ִ���ɫ��������
		//write_mem8(i,i&0x0f);
	}
	for(i = 0xa0000;i <= 0xaffff;i++){
		//�����ַ
		//p = i;
		//*p=i & 0x0f;
		*((char *)i) = i & 0x0f;
	}*/
	
	/*����������
	//����ַ��ֵ��ȥ
	//char *p = (char *)0xa0000;
	boxfill8(p, 320, COL8_FF0000, 20, 20, 120, 120);
	boxfill8(p, 320, COL8_00FF00, 70, 50, 170, 150);
	boxfill8(p, 320, COL8_0000FF, 120, 80, 220, 180);
	*/

	//�� ��ʼ����
	char *vram = (char *) 0xa0000;/* ��ַ������ֵ */
	int xsize = 320;
	int ysize = 200;
	/* ���� 0xa0000 + x + y * 320 �������� 8*/
	boxfill8(vram, xsize, COL8_008484,  0,         0,          xsize -  1, ysize - 29);
	boxfill8(vram, xsize, COL8_C6C6C6,  0,         ysize - 28, xsize -  1, ysize - 28);
	boxfill8(vram, xsize, COL8_FFFFFF,  0,         ysize - 27, xsize -  1, ysize - 27);
	boxfill8(vram, xsize, COL8_C6C6C6,  0,         ysize - 26, xsize -  1, ysize -  1);

	boxfill8(vram, xsize, COL8_FFFFFF,  3,         ysize - 24, 59,         ysize - 24);
	boxfill8(vram, xsize, COL8_FFFFFF,  2,         ysize - 24,  2,         ysize -  4);
	boxfill8(vram, xsize, COL8_848484,  3,         ysize -  4, 59,         ysize -  4);
	boxfill8(vram, xsize, COL8_848484, 59,         ysize - 23, 59,         ysize -  5);
	boxfill8(vram, xsize, COL8_000000,  2,         ysize -  3, 59,         ysize -  3);
	boxfill8(vram, xsize, COL8_000000, 60,         ysize - 24, 60,         ysize -  3);

	boxfill8(vram, xsize, COL8_848484, xsize - 47, ysize - 24, xsize -  4, ysize - 24);
	boxfill8(vram, xsize, COL8_848484, xsize - 47, ysize - 23, xsize - 47, ysize -  4);
	boxfill8(vram, xsize, COL8_FFFFFF, xsize - 47, ysize -  3, xsize -  4, ysize -  3);
	boxfill8(vram, xsize, COL8_FFFFFF, xsize -  3, ysize - 24, xsize -  3, ysize -  3);

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
	};
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