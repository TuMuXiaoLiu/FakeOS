//�������ֵĴ���

#include "bootpack.h"

void set_palette(int start, int end, unsigned char *rgb)
{
	int i, eflags;
	//��¼�ж����ɱ�ʶ��ֵ
	eflags = io_load_eflags();
	//���ж����ɱ�ʶ��Ϊ0����ֹ�ж�
	io_cli();
	io_out8(0x03c8, start);
	for(i = start;i <= end; i++){
		io_out8(0x03c9, rgb[0] / 4);
		io_out8(0x03c9, rgb[1] / 4);
		io_out8(0x03c9, rgb[2] / 4);
		rgb += 3;
	}
	//��ԭ�ж����ɱ�ʶ
	io_store_eflags(eflags);
	return;
}

void init_palette(void)
{
	//C�����е�static char ���ֻ���������ݣ��൱�ڻ���е�DBָ��
	static unsigned char table_rgb[16 * 3] = 
	{
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

//������
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

//��ʼ����Ļ
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

//��ӡ����
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

//������Ҫ��ӡ������ ���Ӹ������ַ�����ѡȡ��Ӧ��������ʾ
void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s)
{
	//�ⲿ�ַ����ļ�
	extern char hankaku[4096];
	/* C�����У��ַ���������0x00��β */
	for (; *s != 0x00; s++) {
		//������ʾ���ֵķ���
		putfont8(vram, xsize, x, y, c, hankaku + *s * 16);
		x += 8;
	}
	return;
}

//�������
void init_mouse_cursor8(char *mouse, char bc)
{
	//�����������
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
	//���ڻ����
	int x, y;
	//ѭ����ӡ���
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

//��ӡ���
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
