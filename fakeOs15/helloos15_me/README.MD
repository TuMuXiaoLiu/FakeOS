Day 15		多任务

一、任务切换
	1、bootpack.c			创建类TSS32 第8-17行，并声明其对象 第60行
	2、bootpack.c			给 ldtr和iomap 属性赋值 第166-169行
	3、bootpack.c			创建gdt	第62行
	4、bootpack.c			将 tss_a 和 tss_b 注册进 GDT 第171-172行
	5、bootpack.c			给TR寄存器赋值，第173行，相应的naskfunc.nas内加入
	6、naskfunc.nas			添加 taskswitch4 函数，并在bootpack.c中添加调用 第251行
	7、bootpcak.c			初始寄存器赋值第175-190，并创建task_b_main函数 第19行和第346-348行
	8、bootpack.c			定义任务B所用的栈task_b_esp	第36行 并创建第174行

二、任务切换进阶 2.0
	1、bootpack.c			修改 task_b_main() 函数 第346-369行
	2、naskfunc.nas			添加 _taskswitch3 第231-233行

三、任务切换优化 3.0
	1、naskfunc.nas			将 _taskswitch3/4() 函数替换为 _farjmp() 第277-229行
	2、bootpack.c			添加 timer_ts 对象	第33、98-100行
	3、bootpack.c			修改 for循环 ，第204-206行
	4、bootpack.c			修改函数 task_b_main(),第350-374行

四、提高运行速度
	请参考参考书

以上
						有一种战争
						注定单枪匹马