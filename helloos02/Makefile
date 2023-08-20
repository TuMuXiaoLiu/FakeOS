
# 默认动作

default :
	../z_tools/make.exe img

# 镜像文件生成

iplccl.bin : iplccl.nas Makefile
	../z_tools/nask.exe iplccl.nas iplccl.bin iplccl.lst

helloosccl.img : iplccl.bin Makefile
	../z_tools/edimg.exe   imgin:../z_tools/fdimg0at.tek wbinimg src:iplccl.bin len:512 from:0 to:0   imgout:helloosccl.img

# 其他指令

asm :
	../z_tools/make.exe -r iplccl.bin

img :
	../z_tools/make.exe -r helloosccl.img

run :
	../z_tools/make.exe img
	copy helloosccl.img ..\z_tools\qemu\fdimage0.bin
	../z_tools/make.exe -C ../z_tools/qemu

install :
	../z_tools/make.exe img
	../z_tools/imgtol.com w a: helloosccl.img

clean :
	-del iplccl.bin
	-del iplccl.lst

src_only :
	../z_tools/make.exe clean
	-del helloosccl.img
