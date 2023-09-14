Day 11	制作窗口

一、 鼠标显示问题	1.0
	1、bootpack.c 	第145-146行代码
	2、sheet.c		增加第158-162行代码

二、shtctl 的省略 2.0
	1、bootpack.h 	增加第203-204行
	2、sheet.c		增加第25-26行
	3、sheet.c		修改sheet_updown参数,增加第73行
	4、bootpack.h	修改sheet_updown参数
	5、sheet.c		修改sheet_refresh参数,修改第149行入参
	6、bootpack.h	修改sheet_refresh参数
	7、sheet.c		修改sheet_slide参数,修改第209、210行入参
	8、bootpack.h	修改sheet_slide参数
	9、sheet.c		修改sheet_free参数,修改第221行入参
	10、bootpack.h	修改sheet_free参数
	11、bootpack.c	修改上述2、5、7、9行函数的实参

三、显示窗口 3.0
	1、bootpack.c		增加函数 make_window8(), 第184-234行
	2、bootpack.c		增加显示窗口的函数，第28、29、71、76、83、90-93、104、109

四、高速计数器 4.0
	1、bootpack.c		增加第 18行、130-137行，修改第78、85、92、143行

五、消除闪烁 4.0
	1、sheet.c			增加sheet_refreshsub()形参h0,修改第184行for循环
	2、sheet.c			修改sheet_refresh(),第166行增加参数
	3、sheet.c			修改sheet_slide(),第230、231行
	4、sheet.c			修改sheet_updown(),第114、129、156行
	5、bootpack.h		SHTCLT类体增加 map 指针 216行
	6、sheet.c			修改sheet_init(),增加第17-23行
	7、sheet.c			增加sheet_refreshmap()函数，第253-296行
	8、sheet.c			增加sheet_refreshsub()形参h1,增加175行map赋值和sid，及208-212行
	9、sheet.c			修改sheet_refreshsub()，增加第166行实参(h1位置)
	10、sheet.c			修改sheet_slide(),增加第232-233的sheet_refreshmap()调用以及增加第234-235行的sheet_refreshsub()实参(h1位置)
	11、sheet.c			增加sheet_updown()中第113、128、154的sheet_refreshmap()调用，以及第114、129、156行的sheet_refreshsub() 实参增加

以上
	
						敬往事一杯酒
						再爱也不回头
