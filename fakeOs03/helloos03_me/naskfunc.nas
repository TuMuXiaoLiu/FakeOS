;naskfunc
;TAB=4

[FORMAT "WCOFF"]		;����Ŀ���ģʽ
[BITS 32]			;����32λģʽ�õĻ�������

;����Ŀ���ļ�����Ϣ
[FILE "naskfunc.nas"]		;Դ�ļ�������Ϣ
GLOBAL _io_hlt			;�����а����ĺ����� ��ǰ����Ҫ����_������C�������ӣ�ͬʱʹ��GLOBAL����

;������ʵ�ʵĺ���
[SECTION .text]			;Ŀ���ļ���д����Щ����д����
_io_hlt:			;void io_hlt(void)
		HLT		;���������
		RET		;��Ϊ��������