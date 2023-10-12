;naskfunc
;TAB=4

[FORMAT "WCOFF"]			;����Ŀ���ģʽ
[INSTRSET "i486p"]
[BITS 32]				;����32λģʽ�õĻ�������

;����Ŀ���ļ�����Ϣ
[FILE "naskfunc.nas"]			;Դ�ļ�������Ϣ
;�����а����ĺ����� ��ǰ����Ҫ����_������C�������ӣ�ͬʱʹ��GLOBAL����
GLOBAL	_io_hlt, _io_cli, _io_sti, _io_stihlt
GLOBAL	_io_in8,  _io_in16,  _io_in32
GLOBAL	_io_out8, _io_out16, _io_out32
GLOBAL	_io_load_eflags, _io_store_eflags
GLOBAL	_load_gdtr, _load_idtr
GLOBAL	_load_cr0, _store_cr0
GLOBAL	_load_tr
GLOBAL	_asm_inthandler20, _asm_inthandler21
GLOBAL	_asm_inthandler27, _asm_inthandler2c
GLOBAL	_asm_inthandler0d
GLOBAL	_memtest_sub
GLOBAL	_farjmp, _farcall
GLOBAL	_asm_hrb_api, _start_app
EXTERN	_inthandler20, _inthandler21
EXTERN	_inthandler27, _inthandler2c
EXTERN	_inthandler0d
EXTERN	_hrb_api

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

_asm_inthandler0d:
		STI
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler0d
		CMP		EAX,0		
		JNE		end_app		
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		ADD		ESP,4			; INT 0x0d��Ҫ���
		IRETD

_asm_inthandler20:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler20
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

_asm_inthandler21:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler21
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

_asm_inthandler27:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler27
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

_asm_inthandler2c:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler2c
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

_load_cr0:
		MOV	EAX,CR0
		RET

_store_cr0:
		MOV	EAX,[ESP+4]
		MOV	CR0,EAX
		RET

_load_tr:
		LTR	[ESP+4]
		RET

;�ڴ��鴦��
_memtest_sub:
		PUSH	EDI
		PUSH	ESI
		PUSH	EBX
		MOV	ESI,0xaa55aa55		;pat0 = 0xaa55aa55
		MOV	EDI,0x55aa55aa		;pat1 = 0x55aa55aa
		MOV	EAX,[ESP+12+4]		;i = start

mts_loop:
		MOV	EBX,EAX
		ADD	EBX,0xffc		;p = i + 0xffc
		MOV	EDX,[EBX]		;old = *p
		MOV	[EBX],ESI		;*p = pat0
		XOR	DWORD [EBX],0xffffffff	;*p^0xffffffff
		CMP	EDI,[EBX]		;if(*p != pat1) goto mts_fin
		JNE	mts_fin
		XOR	DWORD [EBX],0xffffffff	;*p^0xffffffff
		CMP	ESI,[EBX]		;if(*p != pat1) goto mts_fin
		JNE	mts_fin
		MOV	[EBX],EDX		;*p = old
		ADD	EAX,0x1000		;i += 0x1000
		CMP	EAX,[ESP+12+8]		;if(i <= end) goto mts_loop
		JBE	mts_loop
		POP	EBX
		POP	ESI
		POP	EDI
		RET

mts_fin:
		MOV	[EBX],EDX		;*p = old
		POP	EBX
		POP	ESI
		POP	EDI
		RET

_farjmp:
	JMP	FAR[ESP+4]
	RET

_farcall:		; void farcall(int eip, int cs);
	CALL		FAR [ESP+4]     ; eip, cs
	RET

_asm_hrb_api:
		STI
		PUSH	DS
		PUSH	ES
		PUSHAD			; ���ڱ����PUSH
		PUSHAD			; ������hrb_api��ֵ��PUSH
		MOV		AX,SS
		MOV			DS,AX ; ������ϵͳ�öε�ַ����DS��ES
		MOV		ES,AX
		CALL	_hrb_api
		CMP			EAX,0 ; ��EAX��Ϊ0ʱ�������
		JNE		end_app
		ADD		ESP,32
		POPAD
		POP		ES
		POP		DS
		IRETD			;��������Զ�ִ��STI

end_app:
;	EAXΪtss.esp0�ĵ�ַ
		MOV		ESP,[EAX]
		POPAD
		RET			; ����cmd_app

_start_app:		; void start_app(int eip, int cs, int esp, int ds, int *tss_esp0);
		PUSHAD								; ��32λ�Ĵ�����ֵȫ����������
		MOV			EAX,[ESP+36]	; Ӧ�ó�����EIP
		MOV			ECX,[ESP+40]	; Ӧ�ó�����CS
		MOV			EDX,[ESP+44]	; Ӧ�ó�����ESP
		MOV			EBX,[ESP+48]	; Ӧ�ó�����DS/SS
		MOV			EBP,[ESP+52]	; tss.esp0�ĵ�ַ
		MOV			[EBP ],ESP		; �������ϵͳ��ESP
		MOV			[EBP+4],SS		; �������ϵͳ��SS
		MOV		ES,BX
		MOV		DS,BX
		MOV		FS,BX
		MOV		GS,BX
; �������ջ��������RETF��ת��Ӧ�ó���
		OR			ECX,3 				; ��Ӧ�ó����öκź�3����OR����
		OR      EBX,3 				; ��Ӧ�ó����öκź�3����OR����
		PUSH		EBX						; Ӧ�ó����SS
		PUSH		EDX						; Ӧ�ó����ESP
		PUSH		ECX						; Ӧ�ó����CS
		PUSH		EAX						; Ӧ�ó����EIP
		RETF
; Ӧ�ó�������󲻻�ص�����
