//GDT IDT descriptor table ��ϵ����

#include "bootpack.h"

//��ʼ��GDT IDT
void init_gdtidt(void){
	//GDT����
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)ADR_GDT;
	//IDT����
	struct GATE_DESCRIPTOR *idt = (struct GATE_DESCRIPTOR *)ADR_IDT;
	int i;

	//GDT �ĳ�ʼ��
	for (i = 0;i < LIMIT_GDT / 8; i++){
		//�����е� �� ������limit ��ַ base ����Ȩ�� access ����Ϊ0
		set_segmdesc(gdt + i, 0, 0, 0);
	}
	//���öκ�1������Ϊ4G ��ַ��0  ������Ϊ4092
	set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, AR_DATA32_RW);
	//���öκ�2������Ϊ512KB ��ַ��280000 ������Ϊ 409a
	set_segmdesc(gdt + 2, LIMIT_BOTPAK, ADR_BOTPAK, AR_CODE32_ER);
	//ͨ�������μĴ�����ֵ
	load_gdtr(LIMIT_BOTPAK, ADR_GDT);

	//IDT��ʼ��
	for (i = 0; i < LIMIT_IDT / 8; i++){
		//�����жϺŵ��������
		set_gatedesc(idt + i, 0, 0, 0);
	}
	//ͨ�������жϼĴ�����ֵ
	load_idtr(LIMIT_IDT, ADR_IDT);

	//ע�ᶨʱ���ж�
	set_gatedesc(idt + 0x0d, (int)asm_inthandler0d, 2*8, AR_INTGATE32);
	set_gatedesc(idt + 0x20, (int)asm_inthandler20, 2*8, AR_INTGATE32);
	//ע������жϺ�
	set_gatedesc(idt + 0x21, (int)asm_inthandler21, 2*8, AR_INTGATE32);
	//ע������ж�
	set_gatedesc(idt + 0x27, (int)asm_inthandler27, 2*8, AR_INTGATE32);
	//ע������ж�
	set_gatedesc(idt + 0x2c, (int)asm_inthandler2c, 2*8, AR_INTGATE32);
	//ע���Զ��������ж�
	set_gatedesc(idt + 0x40, (int)asm_hrb_api,		2*8, AR_INTGATE32 + 0x60);

	return;
}

/*limit �ε��ֽ���  base ��ַ   access_right/ar ����Ȩ��
ar�ĵͰ�λ���
00000000(0x00):δʹ�õļ�¼��(descriptor table)
10010010(0x92):ϵͳר�ã��ɶ�д�ĶΣ�����ִ��
10011010(0x9a):ϵͳר�ã���ִ�еĶΣ��ɶ�����д
11110010(0xf2):Ӧ�ó����ã��ɶ�д�ĶΣ�����ִ��
11111010(0xfa):Ӧ�ó����ã���ִ�еĶΣ��ɶ�����д
*/
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar){
	//������޴���
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

//IDT �жϺ�ע���
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar){
	gd->offset_low   = offset & 0xffff;
	gd->selector     = selector;
	gd->dw_count     = (ar >> 8) & 0xff;
	gd->access_right = ar & 0xff;
	gd->offset_high  = (offset >> 16) & 0xffff;
	return;
}

