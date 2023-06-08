# 定义内核源码的目录
KERN_DIR ?=/home/liefyuan/Liefyuan/bingpi-v3s/linux-zero-5.2.y
# 定义当前目录
PWD        :=$(shell pwd)
# 要生成的内核模块
obj-m +=misc_dev.o

all:
	make -C $(KERN_DIR) M=$(PWD) modules

clean:
	rm -rf *.order *o *.symvers *.mod.c *.mod *.ko
