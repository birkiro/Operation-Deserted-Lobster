obj-m += i2cdev.o

CROSS = arm-linux-gnueabi-

KDIR = /home/b4me/linux-dev/KERNEL

PWD := $(shell pwd)

all:
	make ARCH=arm -C $(KDIR) M=$(PWD) CROSS_COMPILE=$(CROSS) modules
	scp i2cdev.ko root@192.168.7.2:/home/root
clean:
	make -C $(KDIR) M=$(PWD) CROSS_COMPILE=$(CROSS) clean
