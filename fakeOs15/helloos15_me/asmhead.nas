;haribote-os boot asm
;TAB=4

[INSTRSET "i486p"]


VBEMODE	EQU		0x105			; 1024 x  768 x 8bit ��ɫ
; ��ʾģʽ
;	0x100 :  640 x  400 x 8bit ��ɫ
;	0x101 :  640 x  480 x 8bit ��ɫ
;	0x103 :  800 x  600 x 8bit ��ɫ
;	0x105 : 1024 x  768 x 8bit ��ɫ
;	0x107 : 1280 x 1024 x 8bit ��ɫ

BOTPAK	EQU		0x00280000		;����bootpack
DSKCAC	EQU		0x00100000		;���̻����λ��
DSKCAC0	EQU		0x00008000		;���̻����λ�ã�ʵģʽ��

;BOOT_INFO���
CYLS	EQU		0x0ff0			;����������λ��
LEDS	EQU		0x0ff1			;
VMODE	EQU		0x0ff2			;������ɫ����Ϣ
SCRNX	EQU		0x0ff4			;�ֱ���X
SCRNY	EQU		0x0ff6			;�ֱ���Y
VRAM	EQU		0x0ff8			;ͼ�񻺳�������ʼ��ַ

ORG	0xc200					;����Ҫ��װ�صĵ�ַ

; ȷ��VBE�Ƿ����
		MOV		AX,0x9000
		MOV		ES,AX
		MOV		DI,0
		MOV		AX,0x4f00
		INT		0x10
		CMP		AX,0x004f
		JNE		scrn320

; ���VBE�İ汾
		MOV		AX,[ES:DI+4]
		CMP		AX,0x0200
		JB		scrn320			; if (AX < 0x0200) goto scrn320

; ȡ�û���ģʽ��Ϣ
		MOV		CX,VBEMODE
		MOV		AX,0x4f01
		INT		0x10
		CMP		AX,0x004f
		JNE		scrn320

; ����ģʽ��Ϣ��ȷ��
		CMP		BYTE [ES:DI+0x19],8		;��ɫ������Ϊ8
		JNE		scrn320
		CMP		BYTE [ES:DI+0x1b],4		;��ɫ��ָ����������Ϊ4(4�ǵ�ɫ��ģʽ)
		JNE		scrn320
		MOV		AX,[ES:DI+0x00]			;ģʽ����bit7����1�Ͳ��ܼ���0x4000
		AND		AX,0x0080
		JZ		scrn320				; ģʽ���Ե�bit7��0�����Է���

;	��������
		MOV		BX,VBEMODE+0x4000
		MOV		AX,0x4f02
		INT		0x10
		MOV		BYTE [VMODE],8			; ��Ļ��ģʽ���ο�C���Ե����ã�
		MOV		AX,[ES:DI+0x12]
		MOV		[SCRNX],AX
		MOV		AX,[ES:DI+0x14]
		MOV		[SCRNY],AX
		MOV		EAX,[ES:DI+0x28]		;VRAM�ĵ�ַ
		MOV		[VRAM],EAX
		JMP		keystatus

scrn320:
		MOV		AL,0x13				; VGAͼ��320x200x8bit��ɫ
		MOV		AH,0x00
		INT		0x10
		MOV		BYTE [VMODE],8			; ���»���ģʽ���ο�C���ԣ�
		MOV		WORD [SCRNX],320
		MOV		WORD [SCRNY],200
		MOV		DWORD [VRAM],0x000a0000

;ͨ��BIOS��ȡָʾ��״̬
keystatus:
	MOV		AH,0x02			;
	INT		0x16			;BIOS-keyboard
	MOV		[LEDS],AL		;

;��ֹPIC���������ж�
;AT���ݻ��Ĺ涨��PIC��ʼ��
;Ȼ��֮ǰ��CLI�����κ�����͹���
;PIC��ͬ����ʼ��
	MOV		AL,0xff			;
	OUT		0x21,AL			;
	NOP					;����ִ��out����
	OUT		0xa1,AL			;
	CLI					;��һ���ж�CPU

;��CPU֧��1M�����ڴ桢����A20GATE
	CALL		waitkbdout
	MOV		AL,0xd1
	OUT		0x64,AL
	CALL		waitkbdout
	MOV		AL,0xdf			;enable A20
	OUT		0x60,AL			
	CALL		waitkbdout

;����ģʽת��
[INSTRSET "i486p"]				;˵��ʹ��486ָ��
	LGDT		[GDTR0]			;������ʱGDT
	MOV		EAX,CR0			;
	AND		EAX,0x7fffffff		;ʹ��bit31(���÷�ҳ)
	OR		EAX,0x00000001		;bit0��1ת��(����ģʽ����)
	MOV		CR0,EAX
	JMP		pipelineflush

pipelineflush:
	MOV		AX,1*8			;д32bit�Ķ�
	MOV		DS,AX
	MOV		ES,AX
	MOV		FS,AX
	MOV		GS,AX
	MOV		SS,AX

;bootpack����
	MOV		ESI,bootpack		;Դ
	MOV		EDI,BOTPAK		;Ŀ��
	MOV		ECX,512*1024/4
	CALL		memcpy

;�����������
;����������ʼ
	MOV		ESI,0x7c00		;Դ
	MOV		EDI,DSKCAC		;Ŀ��
	MOV		ECX,512/4
	CALL		memcpy

;ʣ��Ĳ���
	MOV		ESI,DSKCAC0+512		;Դ
	MOV		EDI,DSKCAC+512		;Ŀ��
	MOV		ECX,0
	MOV		CL,BYTE[CYLS]
	IMUL		ECX,512*18*2/4		;����4�õ��ֽ���
	SUB		ECX,512/4		;IPLƫ����
	CALL		memcpy

;��asmhead���ʣ���bootpack����
;bootpack����
	MOV		EBX,BOTPAK
	MOV		ECX,[EBX+16]
	ADD		ECX,3			;ECX += 3
	SHR		ECX,2			;ECX /= 4
	JZ		skip			;�������
	MOV		ESI,[EBX+20]		;Դ
	ADD		ESI,EBX			
	MOV		EDI,[EBX+12]		;Ŀ��
	CALL		memcpy

skip:
	MOV		ESP,[EBX+12]		;��ջ��ʼ��
	JMP		DWORD 2*8:0x0000001b

waitkbdout:
	IN		AL,0x64
	AND		AL,0x02
	JNZ		waitkbdout		;AND�����Ϊ0��ת��waitkbdout
	RET

memcpy:
	MOV		EAX,[ESI]
	ADD		ESI,4
	MOV		[EDI],EAX
	ADD		EDI,4
	SUB		ECX,1
	JNZ		memcpy			;��������Ϊ0��ת��memcpy
	RET

;memcpy ��ַǰ׺��С
	ALIGNB		16

GDT0:	
	RESB		8				;��ʼֵ
	DW		0xffff,0x0000,0x9200,0x00cf	;д32bitλ�μĴ���
	DW		0xffff,0x0000,0x9a28,0x0047	;��ִ�е��ļ���32bit�Ĵ�����bootpack�ã�

	DW		0

GDTR0:
	DW		8*3-1
	DD		GDT0

	ALIGNB		16

bootpack:


