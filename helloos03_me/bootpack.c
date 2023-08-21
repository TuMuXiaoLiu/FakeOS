/*告诉C编译器，有一个函数在别的文件里 自己找一下*/

void io_hlt(void);/*函数声明不用{}，只用;意为 函数在别的文件中，需要找一下*/


void HariMain(void){

	fin:
		io_hlt();
		goto fin;
}
