//要使用sprintg函数 必须在开头加上 #include

//添加函数引用文件
#include "bootpack.h"
#include <stdio.h>

//定义内存可用条数 约32KB
#define MEMMAN_FREES	4090
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

unsigned int memtest(unsigned int start, unsigned int end);
//初始化内存管理
void memman_init(struct MEMMAN *man);
//计算可用内存大小
unsigned int memman_total(struct MEMMAN *man);
//分配内存
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size);
//释放内存
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size);

#define MEMMAN_ADDR		0x003c0000

//程序入口 相当于Java中的Main函数
void HariMain(void)
{
	//声明BOOTINFO的对象并赋值 指针赋值
	struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
	//用于存放输出内容 鼠标 以及鼠标键盘的缓冲区
	char s[40], mcursor[256], keybuf[32], mousebuf[128];
	struct  MOUSE_DEC mdec;

	//声明内存总数
	unsigned int memtotal;
	//声明内存管理对象
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;


	//初始化GDTIDT
	init_gdtidt();
	//初始化PIC
	init_pic();
	//解除 禁止中断 在IDT初始化时所有中断都会被禁止，需要在此解除所有的禁止中断 让CPU接受外部设备指令
	io_sti();


	//接受外部设备 鼠标键盘
	//设置缓冲区
	fifo8_init(&keyfifo, 32, keybuf);
	fifo8_init(&mousefifo, 128, mousebuf);
	//修改PIC的IMR，以便接受来自鼠标键盘的中断
	io_out8(PIC0_IMR, 0xf9); /* 許可(11111001) */
	io_out8(PIC1_IMR, 0xef); /* 許可(11101111) */


	//设定调色板
	init_palette();
	//调用画界面方法并入参
	init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);


	//初始化键盘
	init_keyboard();
	enable_mouse(&mdec);
	//进行内存管理
	memtotal = memtest( 0x00400000, 0xbfffffff);
	memman_init(memman);
	// 0x00001000 - 0x0009efff
	memman_free(memman, 0x00001000, 0x0009e000);
	memman_free(memman, 0x00400000, memtotal - 0x00400000);

	//显示鼠标
	int mx, my;
	//计算画面的中心坐标
	mx = (binfo->scrnx - 16) / 2; 
	my = (binfo->scrny - 28 - 16) / 2;
	//显示并勾勒鼠标
	init_mouse_cursor8(mcursor, COL8_008484);
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
	//打印鼠标坐标
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
				//鼠标的三个字节数据都齐了，显示出来
				
				if (mouse_decode(&mdec, i) != 0)
				{
					//显示鼠标移动数据
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
					//鼠标指针的移动
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, mx, my, mx+15, my+15);

					mx += mdec.x;
					my += mdec.y;
					//控制鼠标的边界范围
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
					//隐藏坐标
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 0, 79, 15);
					//显示坐标
					putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
					//鼠标移动动作显示
					putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
				}
			}
		}
	}
}



//将CPU缓存设置成OFF
#define EFLAGS_AC_BIT		0x00040000
#define CR0_CACHE_DISABLE	0x60000000

//检测内存大小
unsigned int memtest(unsigned int start, unsigned int end)
{
	char flg486 = 0;
	unsigned int eflg, cr0, i;

	//确认CPU时386还是486以上的
	eflg = io_load_eflags();
	//AC-bit = 1
	eflg |= EFLAGS_AC_BIT;
	io_store_eflags(eflg);
	eflg = io_load_eflags();
	//如果时386，即使设定AC=1，AC的值还是会自动回到0
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
		//禁止缓存
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

//初始化内存管理
void memman_init(struct MEMMAN *man)
{
	//设定初始参数
	//可用内存数量
	man->frees = 0;
	//用于观察可用情况 最大值
	man->maxfrees = 0;
	//释放失败的内存大小总和
	man->lostsize = 0;
	//释放失败次数
	man->losts = 0;
	return;
}

//计算可用内存大小
unsigned int memman_total(struct MEMMAN *man)
{
	unsigned int i, t = 0;
	for (i = 0; i< man->frees; i++)
	{
		t += man->free[i].size;
	}
	return t;
}

//分配内存
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size)
{
	unsigned int i, a;
	for (i = 0; i < man->frees; i++)
	{
		//找到足够大的内存
		if (man->free[i].size >= size)
		{
			a = man->free[i].addr;
			man->free[i].addr += size;
			man->free[i].size -= size;
			//如果free[i]变成了0，则减掉一条可用内存数量
			if (man->free[i].size == 0)
			{
				man->frees--;
				for (; i < man->frees; i++)
				{
					//代入结构体?
					man->free[i] = man->free[i+1];
				}
			}
			return a;
		}
	}
	//没有可用空间
	return 0;
}

//释放内存
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
	int i, j;
	//为了便于归纳整理内存，将free[]按照addr的顺序排列
	//所以现决定应该放在哪里
	for (i = 0; i < man->frees; i++)
	{
		//定位 
		if (man->free[i].addr > addr)
		{
			break;
		}
	}
	//free[i-1].addr < addr < free[i].addr
	if (i > 0)
	{
		//前面有可用内存
		if (man->free[i-1].addr + man->free[i-1].size == addr)
		{
			//可以与前面的内存归纳到一起
			man->free[i -1].size += size;
			if (i < man->frees)
			{					//后面也有
				if (addr + size == man->free[i].addr)
				{
					man->free[i - 1].size += man->free[i].size;
					//man->free[i]删除 free[i]变成0后归纳到前面去
					man->frees--;
					for (; i < man->frees; i++)
					{
						man->free[i] = man->free[i+1];
					}
				}
			}
			//成功完成
			return 0;
		}
	}
	//不能与前面的可用空间归纳到一起
	if (i < man->frees)
	{
		//后面还有 
		if (addr + size == man->free[i].addr)
		{
			//可以和后面的归纳到一起
			man->free[i].addr = addr;
			man->free[i].size += size;
			//成功完成
			return 0;
		}
	}
	//既不能与前面的也不能与后面的归纳到一起
	if (man->frees < MEMMAN_FREES)
	{
		//free[i]之后的向后移动 腾出一点可用空间
		for (j = man->frees; j > i; j--)
		{
			man->free[j] = man->free[j - 1];
		}
		man->frees++;
		if (man->maxfrees < man->frees)
		{
			//更新最大值
			man->maxfrees = man->frees;
		}
		man->free[i].addr = addr;
		man->free[i].size = size;
		//成功完成
		return 0;
	}
	//不能往后移
	man->losts++;
	man->lostsize += size;
	//失败
	return -1;	
}
