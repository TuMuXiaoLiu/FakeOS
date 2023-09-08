;naskfunc
;TAB=4

[FORMAT "WCOFF"]			;制作目标的模式
[INSTRSET "i486p"]
[BITS 32]				;制作32位模式用的机器语言

;制作目标文件的信息
[FILE "naskfunc.nas"]			;源文件名称信息
;程序中包含的函数名 且前面需要加上_才能与C语言链接，同时使用GLOBAL声明
GLOBAL _io_hlt, _io_cli, _io_sti; _io_stihlt
GLOBAL _io_in8, _io_in16, _io_in32
GLOBAL _io_out8, _io_out16, _io_out32
GLOBAL _io_load_eflags, _io_store_eflags
;GLOBAL _write_mem8
GLOBAL _load_gdtr, _load_idtr

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

;void write_mem8(int addr, int data);
_write_mem8:				
		MOV	ECX,[ESP+4]		;[ESP+4]中存放的是地址，将其读入ECX
		MOV	AL,[ESP+8]		;[ESP+8]中存放的是数据，将其读入AL
		MOV	[ECX],AL
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