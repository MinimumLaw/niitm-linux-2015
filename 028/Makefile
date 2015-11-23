#CPPFLAGS=-DDEBUG

all: ns_pid

ns_pid: ns_pid.c

clean:
	sudo umount ./proc*
	rm -rf ns_pid ./proc*

run: ns_pid
	sudo ./ns_pid