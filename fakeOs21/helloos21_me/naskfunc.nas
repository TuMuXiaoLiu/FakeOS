;naskfunc
;TAB=4

[FORMAT "WCOFF"]			;制作目标的模式
[INSTRSET "i486p"]
[BITS 32]				;制作32位模式用的机器语言

;制作目标文件的信息
[FILE "naskfunc.nas"]			;源文件名称信息
;程序中包含的函数名 且前面需要加上_才能与C语言链接，同时使用GLOBAL声明
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

;以下是实际的函数
[SECTION .text]				;目标文件中写了这些后再写程序
;void io_hlt(void)
_io_hlt:				
		HLT			;计算机休眠
		RET			;意为函数结束

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
		PUSHFD				;指PUSH EFLAGS
		POP	EAX
		RET

;void io_store_eflags(int eflags)
_io_store_eflags:
		MOV	EAX,[ESP+4]
		PUSH	EAX
		POPFD				;指 POP EFLAGS 
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
		ADD		ESP,4			; INT 0x0d需要这句
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

;内存检查处理
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
		PUSHAD			; 用于保存的PUSH
		PUSHAD			; 用于向hrb_api传值的PUSH
		MOV		AX,SS
		MOV			DS,AX ; 将操作系统用段地址存入DS和ES
		MOV		ES,AX
		CALL	_hrb_api
		CMP			EAX,0 ; 当EAX不为0时程序结束
		JNE		end_app
		ADD		ESP,32
		POPAD
		POP		ES
		POP		DS
		IRETD			;此命令会自动执行STI

end_app:
;	EAX为tss.esp0的地址
		MOV		ESP,[EAX]
		POPAD
		RET			; 返回cmd_app

_start_app:		; void start_app(int eip, int cs, int esp, int ds, int *tss_esp0);
		PUSHAD								; 将32位寄存器的值全部保存起来
		MOV			EAX,[ESP+36]	; 应用程序用EIP
		MOV			ECX,[ESP+40]	; 应用程序用CS
		MOV			EDX,[ESP+44]	; 应用程序用ESP
		MOV			EBX,[ESP+48]	; 应用程序用DS/SS
		MOV			EBP,[ESP+52]	; tss.esp0的地址
		MOV			[EBP ],ESP		; 保存操作系统用ESP
		MOV			[EBP+4],SS		; 保存操作系统用SS
		MOV		ES,BX
		MOV		DS,BX
		MOV		FS,BX
		MOV		GS,BX
; 下面调整栈，以免用RETF跳转到应用程序
		OR			ECX,3 				; 将应用程序用段号和3进行OR运算
		OR      EBX,3 				; 将应用程序用段号和3进行OR运算
		PUSH		EBX						; 应用程序的SS
		PUSH		EDX						; 应用程序的ESP
		PUSH		ECX						; 应用程序的CS
		PUSH		EAX						; 应用程序的EIP
		RETF
; 应用程序结束后不会回到这里
