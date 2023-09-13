//ͼ�����

#include "bootpack.h"

//��ʼ��ͼ�����
struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize)
{
	struct SHTCTL *ctl;
	int i;
	//sizeof(����) �����Զ���������������ֽ�����(�ڴ��С)
	ctl = (struct SHTCTL *)memman_alloc_4k(memman, sizeof(struct SHTCTL));
	if (ctl == 0)
	{
		goto err;
	}
	ctl->vram = vram;
	ctl->xsize = xsize;
	ctl->ysize = ysize;
	//һ��sheet��û��  
	ctl->top = -1;
	for (i = 0; i < MAX_SHEETS; i++)
	{
		//���Ϊδʹ��
		ctl->sheets0[i].flags = 0;
	}
	err:
		return ctl;
}

#define SHEET_USE			1
//��ȡ�����ɵ�δʹ��ͼ��
struct  SHEET *sheet_alloc(struct SHTCTL *ctl)
{
	struct SHEET *sht;
	int i;
	//�������е�ͼ�����
	for (i = 0; i < MAX_SHEETS; i++)
	{
		//�ж��Ƿ���ʹ��
		if (ctl->sheets0[i].flags == 0)
		{
			//��ȡ��δʹ��ͼ��ĵ�ַ
			sht = &ctl->sheets0[i];
			//���Ϊ��ʹ��
			sht->flags = SHEET_USE;
			//����ͼ��
			sht->height = -1;
			return sht;
		}
	}
	///����ͼ�㶼��ʹ����
	return 0;
}

//�趨ͼ��Ļ�������С��͸��ɫ
void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv)
{
	//������
	sht->buf = buf;
	//��С
	sht->bxsize = xsize;
	sht->bysize = ysize;
	//͸��ɫ
	sht->col_inv = col_inv;
	return;
}

//�趨�װ�߶�
void sheet_updown(struct SHTCTL *ctl, struct SHEET *sht, int height)
{
	//�洢����ǰ�ĸ߶�
	int h, old = sht->height;
	//���ָ���ĸ߶ȹ��߻��߹��ͣ����������
	if (height > ctl->top+1)
	{
		height = ctl->top + 1;
	}
	if (height < -1)
	{
		height = -1;
	}
	//�趨�߶�
	sht->height = height;
	//���ڸ߶ȵı仯��Ҫ�����е�ͼ�������������
	//����ǰ��
	if (old > height)
	{
		//�����������
		if (height > 0)
		{
			//���м��������
			for (h = old; h < height; h--)
			{
				ctl->sheets[h] = ctl->sheets[h-1];
				ctl->sheets[h]->height = h;
			}
			ctl->sheets[height] = sht;
		}else{
			//����
			if (ctl->top > old)
			{
				//������Ľ�����
				for (h = old; h < ctl->top; h++)
				{
					ctl->sheets[h] = ctl->sheets[h+1];
					ctl->sheets[h]->height = h;
				}
			}
			//����ͼ����� �����������ͼ��߶��½�
			ctl->top--;
		}
	}else if (old < height)//����ǰ��
	{
		if (old >= 0)
		{
			////���м��������
			for (h = old; h < height; h++)
			{
				ctl->sheets[h] = ctl->sheets[h+1];
				ctl->sheets[h] = h;
			}
			ctl->sheets[height] = sht;
		}else{//������תΪ��ʾ״̬
			//���Ѿ��������������
			for (h = ctl->top; h >= height; h--)
			{
				ctl->sheets[h+1] = ctl->sheets[h];
				ctl->sheets[h+1]->height = h+1; 
			}
			ctl->sheets[height] = sht;
			//��������ʾ��ͼ������һ�������������ͼ��߶�����
			ctl->top++;
		}
	}
	//����ͼ�����»��ƻ���
	sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize);
	return;
}

//ˢ�»���
void sheet_refresh(struct SHTCTL *ctl, struct SHEET *sht, int bx0, int by0, int bx1, int by1)
{	
	//���������ʾ������ͼ��ˢ�»���
	if (sht->height >= 0)
	{
		sheet_refreshsub(ctl, sht->vx0 + bx0, sht->vy0 + by0, sht->vx0 + bx1, sht->vy0 + by1);
	}
	return;
}

//�ֲ�ˢ��
void sheet_refreshsub(struct SHTCTL * ctl, int vx0, int vy0, int vx1, int vy1)
{
	int h, bx, by, vx, vy, bx0, by0, bx1, by1;
	unsigned char *buf, c, *vram = ctl->vram;
	struct SHEET *sht;
	for (h = 0; h <= ctl->top; h++)
	{
		sht = ctl->sheets[h];
		buf = sht->buf;
		//ʹ�� vx0 - vy1, �� bx0-by1 ���е���
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

//�����ƶ�ͼ��
void sheet_slide(struct SHTCTL *ctl, struct SHEET *sht, int vx0, int vy0)
{
	int old_vx0 = sht->vx0, old_vy0 = sht->vy0;
	sht->vx0 = vx0;
	sht->vy0 = vy0;
	//�����ͼ��������ʾ�򼴿�ˢ��
	if (sht->height >= 0)
	{
		//sheet_refresh(ctl);
		sheet_refreshsub(ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize);
		sheet_refreshsub(ctl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize);
	}
	return;
}

//�ͷ���ʹ�õ�ͼ���ڴ溯��
void sheet_free(struct SHTCTL *ctl, struct SHEET *sht)
{
	//���������ʾ״̬ ����Ϊ����
	if (sht->height >= 0)
	{
		sheet_updown(ctl, sht, -1);
	}
	//���Ϊδʹ��״̬
	sht->flags = 0;
	return;
}
