//Ҫʹ��sprintg���� �����ڿ�ͷ���� #include

//��Ӻ��������ļ�
#include "bootpack.h"
#include <stdio.h>


//������� �൱��Java�е�Main����
void HariMain(void)
{
	//����BOOTINFO�Ķ��󲢸�ֵ ָ�븳ֵ
	struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;

	//��ʼ��GDTIDT
	init_gdtidt();
	//��ʼ��PIC
	init_pic();
	//��� ��ֹ�ж� ��IDT��ʼ��ʱ�����ж϶��ᱻ��ֹ����Ҫ�ڴ˽�����еĽ�ֹ�ж� ��CPU�����ⲿ�豸ָ��
	io_sti();

	//�趨��ɫ��
	init_palette();

	//���û����淽�������
	init_screen(binfo->vram, binfo->scrnx, binfo->scrny);
	
	//��ʾ���
	int mx, my;
	//���ڴ��������ݺ����
	char s[40], mcursor[256];
	//���㻭�����������
	mx = (binfo->scrnx - 16) / 2; 
	my = (binfo->scrny - 28 - 16) / 2;
	//��ʾ���������
	init_mouse_cursor8(mcursor, COL8_008484);
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
	//��ӡ�������
	sprintf(s, "(%d, %d)", mx, my);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);

	//�޸�PIC��IMR���Ա�������������̵��ж�
	io_out8(PIC0_IMR, 0xf9); /* �S��(11111001) */
	io_out8(PIC1_IMR, 0xef); /* �S��(11101111) */


	for(;;){
		io_hlt();	
	}
}
















