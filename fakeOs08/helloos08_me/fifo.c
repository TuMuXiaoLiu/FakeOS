//FIFO ������

#include "bootpack.h"

#define FLAGS_OVERRUN		0x0001

//��ʼ��������
void fifo8_init(struct FIFO8 *fifo, int size, unsigned char *buf)
{
	fifo->size = size;
	fifo->buf = buf;
	//��������С
	fifo->free = size;
	fifo->flags = 0;
	//��һ������д���λ��
	fifo->p = 0;
	//��һ�����ݶ�����λ��
	fifo->q = 0;
	return;
}

//��FIFO�������ݲ�����
int fifo8_put(struct FIFO8 *fifo, unsigned char data)
{	
	if (fifo->free == 0)
	{
		//û�пռ��� ���
		fifo->flags |= FLAGS_OVERRUN;
		return -1;
	}
	//��������
	fifo->buf[fifo->p] = data;
	//д��λ��++
	fifo->p++;
	//���д��λ�ñ��� ��д��λ����Ϊ0
	if (fifo->p == fifo->size)
	{
		fifo->p = 0;
	}
	fifo->free--;
	return 0;
}

//��FIFO��ȡ����
int fifo8_get(struct FIFO8 *fifo)
{
	int data;
	//������Ϊ��
	if (fifo->free == fifo->size)
	{
		return -1;
	}
	//ȡ����
	data = fifo->buf[fifo->q];
	//����λ��++
	fifo->q++;
	//����λ�õ�ͷ����Ϊ0
	if (fifo->q == fifo->size)
	{
		fifo->q = 0;
	}
	fifo->free++;
	return data;
}

//�鿴��������������
int fifo8_status(struct FIFO8 *fifo)
{
	return fifo->size - fifo->free;
}

