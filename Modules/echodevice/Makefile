obj-m += echodevice.o

CROSS = arm-linux-gnueabi-

KDIR = /home/birkiro/beaglebone-dev/kernel

PWD := $(shell pwd)

all:
	make ARCH=arm -C $(KDIR) M=$(PWD) CROSS_COMPILE=$(CROSS) modules
	scp echodevice.ko root@192.168.7.2:/home/root
	make -C $(KDIR) M=$(PWD) CROSS_COMPILE=$(CROSS) clean
clean:
	make -C $(KDIR) M=$(PWD) CROSS_COMPILE=$(CROSS) clean
