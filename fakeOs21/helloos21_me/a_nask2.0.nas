[FORMAT "WCOFF"]				;生成对象文件模式
[INSTRSET "i486p"]				;表示使用486兼容指令集
[BITS 32]					;生成32位模式机器语言
[FILE "a_nask.nas"]				;源文件名信息


GLOBAL	_api_putchar

[SECTION .text]

_api_putchar:
		MOV	EDX, 1
		MOV	AX, [ESP + 4]
		INT	0x40
		RET
