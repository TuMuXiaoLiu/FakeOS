//Ҫʹ��sprintg���� �����ڿ�ͷ���� #include

//��Ӻ��������ļ�
#include "bootpack.h"
#include <stdio.h>

//�����ڴ�������� Լ32KB
#define MEMMAN_FREES	4090
//�����ڴ������Ϣ��
struct FREEINFO
{
	unsigned int addr, size;
};

//�����ڴ������
struct MEMMAN
{
	int frees, maxfrees, lostsize, losts;
	struct FREEINFO free[MEMMAN_FREES];
};

unsigned int memtest(unsigned int start, unsigned int end);
//��ʼ���ڴ����
void memman_init(struct MEMMAN *man);
//��������ڴ��С
unsigned int memman_total(struct MEMMAN *man);
//�����ڴ�
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size);
//�ͷ��ڴ�
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size);

#define MEMMAN_ADDR		0x003c0000

//������� �൱��Java�е�Main����
void HariMain(void)
{
	//����BOOTINFO�Ķ��󲢸�ֵ ָ�븳ֵ
	struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
	//���ڴ��������� ��� �Լ������̵Ļ�����
	char s[40], mcursor[256], keybuf[32], mousebuf[128];
	struct  MOUSE_DEC mdec;

	//�����ڴ�����
	unsigned int memtotal;
	//�����ڴ�������
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;


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


	//�趨��ɫ��
	init_palette();
	//���û����淽�������
	init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);


	//��ʼ������
	init_keyboard();
	enable_mouse(&mdec);
	//�����ڴ����
	memtotal = memtest( 0x00400000, 0xbfffffff);
	memman_init(memman);
	// 0x00001000 - 0x0009efff
	memman_free(memman, 0x00001000, 0x0009e000);
	memman_free(memman, 0x00400000, memtotal - 0x00400000);

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

	//
	int i;
	
	sprintf(s, "memory %dMB  free : %dKB", memtotal / (1024*1024), memman_total(memman) / 1024);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 32, COL8_FFFFFF, s);
	
	
	for (;;) 
	{
		io_cli();
		if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0) 
		{
			io_stihlt();
		} else 
		{
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



//��CPU�������ó�OFF
#define EFLAGS_AC_BIT		0x00040000
#define CR0_CACHE_DISABLE	0x60000000

//����ڴ��С
unsigned int memtest(unsigned int start, unsigned int end)
{
	char flg486 = 0;
	unsigned int eflg, cr0, i;

	//ȷ��CPUʱ386����486���ϵ�
	eflg = io_load_eflags();
	//AC-bit = 1
	eflg |= EFLAGS_AC_BIT;
	io_store_eflags(eflg);
	eflg = io_load_eflags();
	//���ʱ386����ʹ�趨AC=1��AC��ֵ���ǻ��Զ��ص�0
	if ((eflg & EFLAGS_AC_BIT) != 0)
	{
		flg486 = 1;
	}
	//AC-bit = 0
	eflg &= ~EFLAGS_AC_BIT;
	io_store_eflags(eflg);

	if (flg486 != 0)
	{
		cr0 = load_cr0();
		//��ֹ����
		cr0 |= CR0_CACHE_DISABLE;
		store_cr0(cr0);
	}

	i = memtest_sub(start, end);
	
	if (flg486 != 0)
	{
		cr0 = load_cr0();
		cr0 &= ~CR0_CACHE_DISABLE;
		store_cr0(cr0);
	}

	return i;
}

//��ʼ���ڴ����
void memman_init(struct MEMMAN *man)
{
	//�趨��ʼ����
	//�����ڴ�����
	man->frees = 0;
	//���ڹ۲������� ���ֵ
	man->maxfrees = 0;
	//�ͷ�ʧ�ܵ��ڴ��С�ܺ�
	man->lostsize = 0;
	//�ͷ�ʧ�ܴ���
	man->losts = 0;
	return;
}

//��������ڴ��С
unsigned int memman_total(struct MEMMAN *man)
{
	unsigned int i, t = 0;
	for (i = 0; i< man->frees; i++)
	{
		t += man->free[i].size;
	}
	return t;
}

//�����ڴ�
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size)
{
	unsigned int i, a;
	for (i = 0; i < man->frees; i++)
	{
		//�ҵ��㹻����ڴ�
		if (man->free[i].size >= size)
		{
			a = man->free[i].addr;
			man->free[i].addr += size;
			man->free[i].size -= size;
			//���free[i]�����0�������һ�������ڴ�����
			if (man->free[i].size == 0)
			{
				man->frees--;
				for (; i < man->frees; i++)
				{
					//����ṹ��?
					man->free[i] = man->free[i+1];
				}
			}
			return a;
		}
	}
	//û�п��ÿռ�
	return 0;
}

//�ͷ��ڴ�
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
	int i, j;
	//Ϊ�˱��ڹ��������ڴ棬��free[]����addr��˳������
	//�����־���Ӧ�÷�������
	for (i = 0; i < man->frees; i++)
	{
		//��λ 
		if (man->free[i].addr > addr)
		{
			break;
		}
	}
	//free[i-1].addr < addr < free[i].addr
	if (i > 0)
	{
		//ǰ���п����ڴ�
		if (man->free[i-1].addr + man->free[i-1].size == addr)
		{
			//������ǰ����ڴ���ɵ�һ��
			man->free[i -1].size += size;
			if (i < man->frees)
			{					//����Ҳ��
				if (addr + size == man->free[i].addr)
				{
					man->free[i - 1].size += man->free[i].size;
					//man->free[i]ɾ�� free[i]���0����ɵ�ǰ��ȥ
					man->frees--;
					for (; i < man->frees; i++)
					{
						man->free[i] = man->free[i+1];
					}
				}
			}
			//�ɹ����
			return 0;
		}
	}
	//������ǰ��Ŀ��ÿռ���ɵ�һ��
	if (i < man->frees)
	{
		//���滹�� 
		if (addr + size == man->free[i].addr)
		{
			//���Ժͺ���Ĺ��ɵ�һ��
			man->free[i].addr = addr;
			man->free[i].size += size;
			//�ɹ����
			return 0;
		}
	}
	//�Ȳ�����ǰ���Ҳ���������Ĺ��ɵ�һ��
	if (man->frees < MEMMAN_FREES)
	{
		//free[i]֮�������ƶ� �ڳ�һ����ÿռ�
		for (j = man->frees; j > i; j--)
		{
			man->free[j] = man->free[j - 1];
		}
		man->frees++;
		if (man->maxfrees < man->frees)
		{
			//�������ֵ
			man->maxfrees = man->frees;
		}
		man->free[i].addr = addr;
		man->free[i].size = size;
		//�ɹ����
		return 0;
	}
	//����������
	man->losts++;
	man->lostsize += size;
	//ʧ��
	return -1;	
}
