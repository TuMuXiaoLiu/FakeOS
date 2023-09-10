//Ҫʹ��sprintg���� �����ڿ�ͷ���� #include

//��Ӻ��������ļ�
#include "bootpack.h"
#include <stdio.h>

//�����ⲿ���� ������
extern struct FIFO8 keyfifo, mousefifo;
void init_keyboard(void);

//���������
struct MOUSE_DEC
{
	//buf ���ڴ������ phase ���ڼ�¼��ȡ�˶�������
	unsigned char buf[3], phase;
	//x,y ���ڴ�������ƶ���Ϣ�� btn ���ڴ�����İ�����Ϣ
	int x, y, btn;
};

//��꼤��
void enable_mouse(struct MOUSE_DEC *mdec);
//������
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat);


//������� �൱��Java�е�Main����
void HariMain(void)
{
	//����BOOTINFO�Ķ��󲢸�ֵ ָ�븳ֵ
	struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
	//���ڴ��������� ��� �Լ������̵Ļ�����
	char s[40], mcursor[256], keybuf[32], mousebuf[128];

	//��ʼ��GDTIDT
	init_gdtidt();
	//��ʼ��PIC
	init_pic();
	//��� ��ֹ�ж� ��IDT��ʼ��ʱ�����ж϶��ᱻ��ֹ����Ҫ�ڴ˽�����еĽ�ֹ�ж� ��CPU�����ⲿ�豸ָ��
	io_sti();


	//�����ⲿ�豸 ������
	//���û�����
	fifo8_init(&keyfifo, 32, keybuf);
	fifo8_init(&mousefifo, 128, mousebuf);
	//�޸�PIC��IMR���Ա�������������̵��ж�
	io_out8(PIC0_IMR, 0xf9); /* �S��(11111001) */
	io_out8(PIC1_IMR, 0xef); /* �S��(11101111) */
	//��ʼ������
	init_keyboard();

	//�趨��ɫ��
	init_palette();
	//���û����淽�������
	init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);
	

	//��ʾ���
	int mx, my;
	//���㻭�����������
	mx = (binfo->scrnx - 16) / 2; 
	my = (binfo->scrny - 28 - 16) / 2;
	//��ʾ���������
	init_mouse_cursor8(mcursor, COL8_008484);
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
	//��ӡ�������
	sprintf(s, "(%d, %d)", mx, my);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);


	struct  MOUSE_DEC mdec;
	enable_mouse(&mdec);

	//
	int i;
	for (;;) {
		io_cli();
		if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0) {
			io_stihlt();
		} else {
			if (fifo8_status(&keyfifo) != 0) {
				i = fifo8_get(&keyfifo);
				io_sti();
				sprintf(s, "%02X", i);
				boxfill8(binfo->vram, binfo->scrnx, COL8_008484,  0, 16, 15, 31);
				putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
			} else if (fifo8_status(&mousefifo) != 0) {
				i = fifo8_get(&mousefifo);
				io_sti();
				//���������ֽ����ݶ����ˣ���ʾ����
				
				if (mouse_decode(&mdec, i) != 0)
				{
					//��ʾ����ƶ�����
					sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
					if ((mdec.btn & 0x01) != 0)
					{
						s[1] = 'L';
					}
	
					if ((mdec.btn & 0x02) != 0)
					{
						s[3] = 'R';
					}

					if ((mdec.btn & 0x04) != 0)
					{
						s[2] = 'C';
					}
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 32, 16, 32+15*8-1, 31);
					putfonts8_asc(binfo->vram, binfo->scrnx, 31, 16, COL8_FFFFFF, s);
					//���ָ����ƶ�
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, mx, my, mx+15, my+15);

					mx += mdec.x;
					my += mdec.y;
					//�������ı߽緶Χ
					if (mx < 0)
					{
						mx = 0;
					}
					if (my < 0)
					{
						my = 0;
					}
					if (mx > binfo->scrnx -16)
					{
						mx = binfo->scrnx -16;
					}
					if (my > binfo->scrny -16)
					{
						my = binfo->scrny -16;
					}
					sprintf(s, "(%3d, %3d)", mx, my);
					//��������
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 0, 79, 15);
					//��ʾ����
					putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
					//����ƶ�������ʾ
					putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
				}
			}
		}
	}
}


#define PORT_KEYDAT				0x0060
#define PORT_KEYSTA				0x0064
#define PORT_KEYCMD				0x0064
#define KEYSTA_SEND_NOTREADY	0x02
#define KEYCMD_WRITE_MODE		0x60
#define KBC_MODE				0x47

void wait_KBC_sendready(void)
{
	/* �ȴ����̿��Ƶ�·׼����� */
	for (;;) {
		if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
			break;
		}
	}
	return;
}

void init_keyboard(void)
{
	/* ��ʼ�����̿��Ƶ�· */
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE);
	return;
}

#define KEYCMD_SENDTO_MOUSE		0xd4
#define MOUSECMD_ENABLE			0xf4

struct MOUSE_DEC *mdec;

void enable_mouse(struct MOUSE_DEC *mdec)
{
	 //������� 
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
	//˳���Ļ������̿������᷵��ACK(0xfa)
	return; 
}

//������
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat){
		//���뵽�ȴ����� 0xfa ��״̬
		if (mdec->phase == 0)
		{
			if (dat == 0xfa)
			{
				mdec->phase = 1;
			}
			return 0;
		}

		//�ȴ����ĵ�һ���ֽ�
		if (mdec->phase == 1)
		{
			//�����һ���ֽ���ȷ
			if ((dat & 0xc8) == 0x08)
			{
				mdec->buf[0] = dat;
				mdec->phase = 2;
			}
			
			return 0;
		}

		//�ȴ����ĵڶ����ֽ�
		if (mdec->phase == 2)
		{
			mdec->buf[1] = dat;
			mdec->phase = 3;
			return  0;
		}

		//�ȴ����ĵ������ֽ�
		if (mdec->phase == 3)
		{	
			//��ȡ����������
			mdec->buf[2] = dat;
			mdec->phase = 1;
			mdec->btn = mdec->buf[0] & 0x07;
			mdec->x = mdec->buf[1];
			mdec->y = mdec->buf[2];

			if ((mdec->buf[0] & 0x10) != 0)
			{
				mdec->x |= 0xffffff00;
			}

			if ((mdec->buf[0] & 0x20) != 0)
			{
				mdec->y |= 0xffffff00; 
			}
			//����y�����뻭������෴
			mdec->y = - mdec->y;
			//��ȡ��� ����1
			return 1;
		}
		
		//һ������²���ִ�е���һ��
		return -1;
}









