Day 17	命令行窗口

一、闲置任务	参考2.0
	1、mtask.c				增加函数 task_idle() 第196-201行，修改 task_init() 函数，第11行、第37-46行
	2、bootpack.c			修改 sht_win_b,将最后一行注释。 第117行

二、创建命令行窗口 2.0
	1、bootpack.c			注释/删除 sht_win_b 相关 第10、44、46、48、100-118、172-174、183-185行
	2、bootpack.c			添加函数 console_task() 第385-419行
	3、bootpack.c			添加第120-137、175、186行

三、切换窗口输入 3.0
	1、bootpack.c			拆分原 make_window8() 函数 第264-326行
	2、bootpack.c			增加第43行、203-215行

四、实现字符输入 4.0
	1、bootpack.h				向TASK中添加fifo属性
	2、bootpack.c				增加189-220行
	3、bootpack.c				修改console_task()函数

五、符号输入 5.0
	1、bootpack.c				第35-55行
	2、bootpack.c				第201-226、256-271行

六、大小写字母 和 对锁定键的支持
	1、bootpack.c				第49、160-164行
	2、bootpack.c				第12、18、19、49、60、131-132、135-140、213-234行

以上
							鸟穿浮云云不惊，
							沙沉流水水尚清。	
	