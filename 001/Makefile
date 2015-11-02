CC=gcc

all: libHello.so hello

clean:
		rm -f libHello.so hello

run: all
		LD_LIBRARY_PATH=${PWD}:${LD_LIBRARY_PATH} ./hello

libHello.so: libHello.c libHello.h
		${CC} -o libHello.so -fPIC -shared libHello.c

hello: main.c libHello.h libHello.so
		${CC} -o hello main.c -L${PWD} -lHello
