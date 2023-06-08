KERN_DIR = /home/liefyuan/Liefyuan/bingpi-v3s/linux-zero-5.2.y
all:
	make -C $(KERN_DIR) M=$(shell pwd) modules
clean:
	rm -rf *.order *o *.symvers *.mod.c *.mod *.ko
obj-m += dht11_dev.o