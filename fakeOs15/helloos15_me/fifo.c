//FIFO 缓冲区

#include "bootpack.h"

#define FLAGS_OVERRUN		0x0001

//初始化缓冲区
void fifo32_init(struct FIFO32 *fifo, int size, int *buf){
	fifo->size = size;
	fifo->buf = buf;
	//缓冲区大小
	fifo->free = size;
	fifo->flags = 0;
	//下一个数据写入的位置
	fifo->p = 0;
	//下一个数据读出的位置
	fifo->q = 0;
	return;
}

//向FIFO传送数据并保存
int fifo32_put(struct FIFO32 *fifo, int data){	
	if (fifo->free == 0){
		//没有空间了 溢出
		fifo->flags |= FLAGS_OVERRUN;
		return -1;
	}
	//存入数据
	fifo->buf[fifo->p] = data;
	//写入位置++
	fifo->p++;
	//如果写入位置饱和 则写入位置置为0
	if (fifo->p == fifo->size){
		fifo->p = 0;
	}
	fifo->free--;
	return 0;
}

//从FIFO获取数据
int fifo32_get(struct FIFO32 *fifo){
	int data;
	//缓冲区为空
	if (fifo->free == fifo->size){
		return -1;
	}
	//取数据
	data = fifo->buf[fifo->q];
	//读出位置++
	fifo->q++;
	//读出位置到头，置为0
	if (fifo->q == fifo->size){
		fifo->q = 0;
	}
	fifo->free++;
	return data;
}

//查看缓冲区内容数量
int fifo32_status(struct FIFO32 *fifo){
	return fifo->size - fifo->free;
}

