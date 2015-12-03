obj-m += kbuf.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

load: all
	sudo /sbin/insmod kbuf.ko
	sudo dmesg -c

unload:
	sudo /sbin/rmmod kbuf.ko
	sudo dmesg -c
