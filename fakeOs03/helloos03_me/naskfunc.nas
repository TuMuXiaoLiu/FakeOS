;naskfunc
;TAB=4

[FORMAT "WCOFF"]		;制作目标的模式
[BITS 32]			;制作32位模式用的机器语言

;制作目标文件的信息
[FILE "naskfunc.nas"]		;源文件名称信息
GLOBAL _io_hlt			;程序中包含的函数名 且前面需要加上_才能与C语言链接，同时使用GLOBAL声明

;以下是实际的函数
[SECTION .text]			;目标文件中写了这些后再写程序
_io_hlt:			;void io_hlt(void)
		HLT		;计算机休眠
		RET		;意为函数结束