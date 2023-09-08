;naskfunc
;TAB=4

[FORMAT "WCOFF"]			;����Ŀ���ģʽ
[INSTRSET "i486p"]
[BITS 32]				;����32λģʽ�õĻ�������

;����Ŀ���ļ�����Ϣ
[FILE "naskfunc.nas"]			;Դ�ļ�������Ϣ
;�����а����ĺ����� ��ǰ����Ҫ����_������C�������ӣ�ͬʱʹ��GLOBAL����
GLOBAL _io_hlt, _io_cli, _io_sti; _io_stihlt
GLOBAL _io_in8, _io_in16, _io_in32
GLOBAL _io_out8, _io_out16, _io_out32
GLOBAL _io_load_eflags, _io_store_eflags
GLOBAL _load_gdtr, _load_idtr
GLOBAL	_asm_inthandler21, _asm_inthandler2c
EXTERN	_inthandler21, _inthandler2c

;������ʵ�ʵĺ���
[SECTION .text]				;Ŀ���ļ���д����Щ����д����
;void io_hlt(void)
_io_hlt:				
		HLT			;���������
		RET			;��Ϊ��������

;void io_cli(void)
_io_cli:				
		CLI
		RET

;void io_sti(void)
_io_sti:				
		STI
		RET

;void io_stihlt(void)
_io_stihlt:	
		STI
		HLT
		RET

;int io_in8(int port)
_io_in8:
		MOV	EDX,[ESP+4]		;port
		MOV	EAX,0
		IN	AL,DX
		RET

;int io_in16(int port)
_io_in16:
		MOV	EDX,[ESP+4]		;port
		MOV	EAX,0
		IN	AX,DX
		RET

;int io_in32(int port)
_io_in32:	
		MOV	EDX,[ESP+4]
		MOV	EAX,0
		IN	AX,DX
		RET

;void io_out8(int port, int data)
_io_out8:
		MOV	EDX,[ESP+4]		;port
		MOV	AL,[ESP+8]		;data
		OUT	DX,AL
		RET

;void io_out16
_io_out16:
		MOV	EDX,[ESP+4]		;port
		MOV	AL,[ESP+8]		;data
		OUT	DX,AL
		RET

;void io_out32
_io_out32:
		MOV	EDX,[ESP+4]
		MOV	AL,[ESP+8]
		OUT	DX,AL
		RET

;int io_load_eflags(void)
_io_load_eflags:
		PUSHFD				;ָPUSH EFLAGS
		POP	EAX
		RET

;void io_store_eflags(int eflags)
_io_store_eflags:
		MOV	EAX,[ESP+4]
		PUSH	EAX
		POPFD				;ָ POP EFLAGS 
		RET

;void load_gdtr(int limit, int addr);
_load_gdtr:
		MOV	AX,[ESP+4]		;limit
		MOV	[ESP+6],AX
		LGDT	[ESP+6]
		RET

;void load_idtr(int limit, int addr);
_load_idtr:
		MOV	AX,[ESP+4]
		MOV	[ESP+6],AX
		LIDT	[ESP+6]
		RET

EXTERN _inthandler21, _inthandler2c

_asm_inthandler21:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV	EAX,ESP
		PUSH	EAX
		MOV	AX,SS
		MOV	DS,AX
		MOV	ES,AX
		CALL	_inthandler21
		POP	EAX
		POPAD
		POP	DS
		POP	ES
		IRETD

_asm_inthandler2c:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV	EAX,ESP
		PUSH	EAX
		MOV	AX,SS
		MOV	DS,AX
		MOV	ES,AX
		CALL	_inthandler2c
		POP	EAX
		POPAD
		POP	DS
		POP	ES
		IRETD

