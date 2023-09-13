Day 10	叠加处理

一、 内存管理（内容分割） 1.0
		1.将 bootpack.c 中内存相关的 移动至 menory.c 和 bootpack.h 中
		2.在 makefile 的首行添加 memory.obj

二、以4K为大小进行分配和释放内存 2.0
		1.在 memory.c 中添加 第183-199行代码，实现4K大小内存的分配和释放
		2.将这两个函数添加进 bootpack.h 中

三、图层叠加 3.0
		1.添加 sheet.c 用于管理所有图层，同时在 makefile 的首行添加 sheet.obj
		2.修改 bootpack.h，将 sheet.c 相关添加进来 第195-235行		
		3.修改 bootpack.c 内容过多，不予展示

四、提高处理速度 4.0
		1.修改 sheet.c, 添加 sheet_refreshsub()局部刷新函数
		2.修改 sheet_refresh()函数
		3.修改 sheet_updown()函数
		4.修改 bootpack.c 中的 sheet_refresh() 函数参数

五、提高处理速度 
		修改 sheet.c 中的 sheet_refreshsub() 函数


以上
						我用执着烧死了所有的幼稚和任性, 
						那片荒野慢慢长出了理智,冷漠和清醒