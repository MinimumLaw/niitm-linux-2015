obj-m += vnet.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

load: all
	sudo /sbin/insmod vnet.ko
	sudo dmesg -c

unload:
	sudo /sbin/rmmod vnet.ko
	sudo dmesg -c

test:
	sudo ifconfig vnet0 10.1.1.1/24 up
	ping 10.1.1.3 -c 3
	sudo ifconfig vnet0 down
