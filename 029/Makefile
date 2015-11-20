obj-m += simple1.o
obj-m += simple2.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

run: all
#	sudo /sbin/rmmod -f simple2.ko
#	sudo /sbin/rmmod -f simple1.ko
	sudo /sbin/insmod simple1.ko
	sudo /sbin/insmod simple2.ko
	sudo dmesg -c
	sudo /sbin/rmmod simple2.ko
	sudo /sbin/rmmod simple1.ko
