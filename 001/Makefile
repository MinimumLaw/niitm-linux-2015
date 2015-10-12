CC=gcc

all: libHello.so hello

clean:
		rm -f libHello.so hello

run:
		LD_PRELOAD_PATH=`pwd` ./hello

libHello.so: libHello.c libHello.h
		${CC} -o libHello.so -shared libHello.c

hello: main.c libHello.h libHello.so
		${CC} -o hello -L . -llibHello
