[FORMAT "WCOFF"]				;���ɶ����ļ�ģʽ
[INSTRSET "i486p"]				;��ʾʹ��486����ָ�
[BITS 32]					;����32λģʽ��������
[FILE "a_nask.nas"]				;Դ�ļ�����Ϣ


GLOBAL	_api_putchar

[SECTION .text]

_api_putchar:
		MOV	EDX, 1
		MOV	AX, [ESP + 4]
		INT	0x40
		RET
