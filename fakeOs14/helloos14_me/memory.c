//内存管理相关

#include "bootpack.h"

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

//按4KB的大小进行分配和释放 0x1000正好时4KB
//分配
unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size)
{
	unsigned int a;
	size = (size + 0xfff) & 0xfffff000;
	a = memman_alloc(man, size);
	return a;
}
//释放
int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
	int i;
	size = (size + 0xfff) & 0xfffff000;
	i = memman_free(man, addr, size);
	return i;
}
