Day 12	定时器 

一、使用定时器 1.0
	1、time.c			创建此文件并在makefile文件首行添加对应的obj
	2、time.c			增加第1-21行
	3、bootpack.h		增加第242-243行
	4、bootpack.c		增加第49-53行		
	5、time.c			添加中断函数 inthandler20（新增函数名进bootpack.h 之后不再表述）
	6、naskfunc.nas		添加_asm_inthandler20	以及头列表，bootpack.h 别忘了
	7、dsctbl.c			增加定时器的中断 第37行
	
二、计量时间 2.0
	1、bootpack.h		增加定时器控制器类 声明外部引用对象 第248-254行
	2、time.c			增加定时器控制器对象，并进行计数 第16、24、34行
	3、bootpack.c		修改屏幕显示参数 第137行

三、超时功能(定时呼叫) 3.0
	1、bootpack.h		增加第252行
	2、time.c			修改 init_pit()、inthandler20() 函数，增加settimer()函数
	3、bootpack.c		增加/修改 第14、16、53、56、64、67、154、205-211行
	PS 53、56等行注意先后执行顺序

四、多位定时器 4.0
	1、bootpack.h		增加254-261，268行
	2、time.c			增加第16、18行,修改 init_pit()、settime()->timer_settime()、inthandler20(),增加 timer_alloc()、timer_free()、timer_init()
	3、bootpack.c		修改第14、16、18、60-74、160、217-239行
	
五、优化中断inthandler20
	此次优化将参数中的三个优化一次性处理完了，故……按照参考书看吧。对应的地方做了一次、二次注释。

以上
						若能避开猛烈的狂喜
						自然不会有伤痛来袭