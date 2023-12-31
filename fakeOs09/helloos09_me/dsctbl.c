//GDT IDT descriptor table 关系处理

#include "bootpack.h"

//初始化GDT IDT
void init_gdtidt(void)
{
	//GDT对象
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)ADR_GDT;
	//IDT对象
	struct GATE_DESCRIPTOR *idt = (struct GATE_DESCRIPTOR *)ADR_IDT;
	int i;

	//GDT 的初始化
	for (i = 0;i < LIMIT_GDT / 8; i++)
	{
		//将所有的 段 的上限limit 基址 base 访问权限 access 都设为0
		set_segmdesc(gdt + i, 0, 0, 0);
	}
	//设置段号1的上限为4G 基址是0  段属性为4092
	set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, AR_DATA32_RW);
	//设置段号2的上限为512KB 基址是280000 段属性为 409a
	set_segmdesc(gdt + 2, LIMIT_BOTPAK, ADR_BOTPAK, AR_CODE32_ER);
	//通过汇编给段寄存器赋值
	load_gdtr(LIMIT_BOTPAK, ADR_GDT);

	//IDT初始化
	for (i = 0; i < LIMIT_IDT / 8; i++)
	{
		//设置中断号的相关数据
		set_gatedesc(idt + i, 0, 0, 0);
	}
	//通过汇编给中断寄存器赋值
	load_idtr(LIMIT_IDT, ADR_IDT);

	//注册键盘中断号
	set_gatedesc(idt + 0x21, (int)asm_inthandler21, 2*8, AR_INTGATE32);
	//注册键盘中断
	set_gatedesc(idt + 0x27, (int) asm_inthandler27, 2 * 8, AR_INTGATE32);
	//注册鼠标中断
	set_gatedesc(idt + 0x2c, (int)asm_inthandler2c, 2*8, AR_INTGATE32);
	return;
}

/*limit 段的字节数  base 基址   access_right/ar 访问权限
ar的低八位简介
00000000(0x00):未使用的记录表(descriptor table)
10010010(0x92):系统专用，可读写的段，不可执行
10011010(0x9a):系统专用，可执行的段，可读不可写
11110010(0xf2):应用程序用，可读写的段，不可执行
11111010(0xfa):应用程序用，可执行的段，可读不可写
*/
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar)
{
	//如果上限大于
	if (limit > 0xfffff) {
		ar |= 0x8000; /* G_bit = 1 */
		limit /= 0x1000;
	}
	sd->limit_low    = limit & 0xffff;
	sd->base_low     = base & 0xffff;
	sd->base_mid     = (base >> 16) & 0xff;
	sd->access_right = ar & 0xff;
	sd->limit_high   = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);
	sd->base_high    = (base >> 24) & 0xff;
	return;
}

//IDT 中断号注册表
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar)
{
	gd->offset_low   = offset & 0xffff;
	gd->selector     = selector;
	gd->dw_count     = (ar >> 8) & 0xff;
	gd->access_right = ar & 0xff;
	gd->offset_high  = (offset >> 16) & 0xffff;
	return;
}

