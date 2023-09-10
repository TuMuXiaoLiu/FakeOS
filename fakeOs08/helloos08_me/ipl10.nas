; harlbote-ipl
; TAB=4

CYLS	EQU		10				;声明CYLS=10

		ORG		0x7c00			; 指明程序装载地址

; 标准FAT12格式软盘专用的代码 Stand FAT12 format floppy code

		JMP		entry
		DB		0x90
		DB		"HARIBOTE"		; 启动扇区名称（8字节）
		DW		512			; 每个扇区（sector）大小（必须512字节）
		DB		1			; 簇（cluster）大小（必须为1个扇区）
		DW		1			; FAT起始位置（一般为第一个扇区）
		DB		2			; FAT个数（必须为2）
		DW		224			; 根目录大小（一般为224项）
		DW		2880			; 该磁盘大小（必须为2880扇区1440*1024/512）
		DB		0xf0			; 磁盘类型（必须为0xf0）
		DW		9			; FAT的长度（必??9扇区）
		DW		18			; 一个磁道（track）有几个扇区（必须为18）
		DW		2			; 磁头数（必??2）
		DD		0			; 不使用分区，必须是0
		DD		2880			; 重写一次磁盘大小
		DB		0,0,0x29		; 意义不明（固定）
		DD		0xffffffff		; （可能是）卷标号码
		DB		"CCL-OS     "		; 磁盘的名称（必须为11字?，不足填空格）
		DB		"FAT12   "		; 磁盘格式名称（必??8字?，不足填空格）
		RESB		18			; 先空出18字节
					;原先此代码在第8行处，但由于位置错误，在制作img文件时会提示错误：imgout BPB data error

; 程序主体

entry:
		MOV		AX,0			;初始化寄存器
		MOV		SS,AX			;初始化栈寄存器
		MOV		SP,0x7c00		;将栈指针寄存器初始化并给予0x7c00,即引导程序位置
		MOV		DS,AX			;初始化数据寄存器

;读取磁盘
		MOV		AX,0x0820		;指定存放数据的内存起始位置，读进来之后存放在这里
		MOV		ES,Ax			;指定段寄存器
		MOV		CH,0			;柱面0
		MOV		DH,0			;磁头0
		MOV		CL,2			;扇区2

readloop:
		MOV		SI,0			;记录失败次数寄存器

retry:		
		MOV		AH,0x02			;AH=0x02 读入磁盘
		MOV		AL,1			;一个扇区
		MOV		BX,0			;
		MOV		DL,0x00			;A驱动器
		INT		0x13			;调用磁盘BIOS
		JNC		next			;没有出错则跳转至fin
		ADD		SI,1			;往SI加1
		CMP		SI,5			;比较
		JAE		error			;如果5<=SI,则跳转至error
		MOV		AH,0X00			;
		MOV		DL,0X00			;A驱动器
		INT		0x13			;重置驱动器
		JMP		retry

next:
		MOV		AX,ES			;把内存地址后移0x200（512/16 十六进制转换）
		ADD		AX,0x0020		;
		MOV		ES,AX			;ADD ES，0x020，因为没有ADD ES，所以只能通过AX进行
		ADD		CL,1			;往CL加一，即下一个扇区
		CMP		CL,18			;比较CL
		JBE		readloop		;如果CL<=18则跳转回readloop
		MOV		CL,1			;扇区加一
		ADD		DH,1			;磁头加一
		CMP		DH,2			;比较
		JB		readloop		;DH<2跳转回readloop
		MOV		DH,0			;设置磁头为0（正面磁头）
		ADD		CH,1			;扇区加一
		CMP		CH,CYLS			;比较扇区的大小，CYLS值在开头已声明定义
		JB		readloop		;CH>CYLS跳转至readloop

;读取完毕，跳转至 haribote.sys 执行
		MOV		[0x0ff0],CH
		JMP		0xc200

error:		
		MOV		SI,msg

putloop:
		MOV		AL,[SI]
		ADD		SI,1			; 给SI加1
		CMP		AL,0			;比较AL，如果为0即读不到数据便跳转
		JE		fin
		MOV		AH,0x0e			; 显示一个文字
		MOV		BX,15			; 指定字符颜色
		INT		0x10			; 调用显卡BIOS
		JMP		putloop
fin:
		HLT					; 让CPU停止，等待指令
		JMP		fin			; 无限循环

msg:
		DB		0x0a, 0x0a		; 换行两次
		DB		"load error_ccl"
		DB		0x0a			; 换行
		DB		0

		RESB		0x7dfe-$		; 填写0x00直到0x001fe

		DB		0x55, 0xaa
