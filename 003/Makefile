CC=gcc

all: libHello.so hello

clean:
		rm -f libHello.so hello

run: all
		./hello

libHello.so: libHello.c libHello.h
		${CC} -o libHello.so -fPIC -shared libHello.c

hello: main.c
		${CC} -o hello main.c -L${PWD} -ldl
