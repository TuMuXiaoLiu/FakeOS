//图层管理

#include "bootpack.h"

//初始化图层管理
struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize)
{
	struct SHTCTL *ctl;
	int i;
	//sizeof(类名) 可以自动计算出所需对象的字节数量(内存大小)
	ctl = (struct SHTCTL *)memman_alloc_4k(memman, sizeof(struct SHTCTL));
	if (ctl == 0)
	{
		goto err;
	}
	ctl->vram = vram;
	ctl->xsize = xsize;
	ctl->ysize = ysize;
	//一个sheet都没有  
	ctl->top = -1;
	for (i = 0; i < MAX_SHEETS; i++)
	{
		//标记为未使用
		ctl->sheets0[i].flags = 0;
	}
	err:
		return ctl;
}

#define SHEET_USE			1
//获取新生成的未使用图层
struct  SHEET *sheet_alloc(struct SHTCTL *ctl)
{
	struct SHEET *sht;
	int i;
	//遍历所有的图层对象
	for (i = 0; i < MAX_SHEETS; i++)
	{
		//判断是否已使用
		if (ctl->sheets0[i].flags == 0)
		{
			//获取该未使用图层的地址
			sht = &ctl->sheets0[i];
			//标记为已使用
			sht->flags = SHEET_USE;
			//隐藏图层
			sht->height = -1;
			return sht;
		}
	}
	///所有图层都在使用中
	return 0;
}

//设定图层的缓冲区大小和透明色
void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv)
{
	//缓冲区
	sht->buf = buf;
	//大小
	sht->bxsize = xsize;
	sht->bysize = ysize;
	//透明色
	sht->col_inv = col_inv;
	return;
}

//设定底板高度
void sheet_updown(struct SHTCTL *ctl, struct SHEET *sht, int height)
{
	//存储设置前的高度
	int h, old = sht->height;
	//如果指定的高度过高或者过低，则进行修正
	if (height > ctl->top+1)
	{
		height = ctl->top + 1;
	}
	if (height < -1)
	{
		height = -1;
	}
	//设定高度
	sht->height = height;
	//由于高度的变化需要对所有的图层进行重新排列
	//比以前低
	if (old > height)
	{
		//如果不是隐藏
		if (height > 0)
		{
			//把中间的往上提
			for (h = old; h < height; h--)
			{
				ctl->sheets[h] = ctl->sheets[h-1];
				ctl->sheets[h]->height = h;
			}
			ctl->sheets[height] = sht;
		}else{
			//隐藏
			if (ctl->top > old)
			{
				//把上面的降下来
				for (h = old; h < ctl->top; h++)
				{
					ctl->sheets[h] = ctl->sheets[h+1];
					ctl->sheets[h]->height = h;
				}
			}
			//由于图层减少 所以最上面的图层高度下降
			ctl->top--;
		}
	}else if (old < height)//比以前高
	{
		if (old >= 0)
		{
			////把中间的拉下来
			for (h = old; h < height; h++)
			{
				ctl->sheets[h] = ctl->sheets[h+1];
				ctl->sheets[h] = h;
			}
			ctl->sheets[height] = sht;
		}else{//由隐藏转为显示状态
			//将已经在上面的提上来
			for (h = ctl->top; h >= height; h--)
			{
				ctl->sheets[h+1] = ctl->sheets[h];
				ctl->sheets[h+1]->height = h+1; 
			}
			ctl->sheets[height] = sht;
			//由于已显示的图层新增一个，故最上面的图层高度增加
			ctl->top++;
		}
	}
	//按新图层重新绘制画面
	sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize);
	return;
}

//刷新画面
void sheet_refresh(struct SHTCTL *ctl, struct SHEET *sht, int bx0, int by0, int bx1, int by1)
{	
	//如果正在显示，则按新图层刷新画面
	if (sht->height >= 0)
	{
		sheet_refreshsub(ctl, sht->vx0 + bx0, sht->vy0 + by0, sht->vx0 + bx1, sht->vy0 + by1);
	}
	return;
}

//局部刷新
void sheet_refreshsub(struct SHTCTL * ctl, int vx0, int vy0, int vx1, int vy1)
{
	int h, bx, by, vx, vy, bx0, by0, bx1, by1;
	unsigned char *buf, c, *vram = ctl->vram;
	struct SHEET *sht;
	for (h = 0; h <= ctl->top; h++)
	{
		sht = ctl->sheets[h];
		buf = sht->buf;
		//使用 vx0 - vy1, 对 bx0-by1 进行倒推
		bx0 = vx0 - sht->vx0;
		by0 = vy0 - sht->vy0;
		bx1 = vx1 - sht->vx0;
		by1 = vy1 - sht->vy0;

		if (bx0 < 0){ bx0 = 0; }
		if (by0 < 0){ by0 = 0; }
		if (bx1 > sht->bxsize){ bx1 = sht->bxsize; }
		if (by1 > sht->bysize){ by1 = sht->bysize; }

		for (by = by0; by < by1; by++)
		{
			vy = sht->vy0 + by;
			for (bx = bx0; bx < bx1; bx++)
			{
				vx = sht->vx0 + bx;
				c = buf[by * sht->bxsize + bx];
				if (c != sht->col_inv)
				{
					vram[vy * ctl->xsize + vx] = c;
				}
			}
		}
	}
	return;
}

//左右移动图层
void sheet_slide(struct SHTCTL *ctl, struct SHEET *sht, int vx0, int vy0)
{
	int old_vx0 = sht->vx0, old_vy0 = sht->vy0;
	sht->vx0 = vx0;
	sht->vy0 = vy0;
	//如果此图层正在显示则即刻刷新
	if (sht->height >= 0)
	{
		//sheet_refresh(ctl);
		sheet_refreshsub(ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize);
		sheet_refreshsub(ctl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize);
	}
	return;
}

//释放已使用的图层内存函数
void sheet_free(struct SHTCTL *ctl, struct SHEET *sht)
{
	//如果还在显示状态 则设为隐藏
	if (sht->height >= 0)
	{
		sheet_updown(ctl, sht, -1);
	}
	//标记为未使用状态
	sht->flags = 0;
	return;
}
