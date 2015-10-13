CC=gcc

all: libHello.so hello

clean:
		rm -f libHello.so hello

run:
		./hello

libHello.so: libHello.c libHello.h
		${CC} -o libHello.so -shared libHello.c

hello: main
		mv main hello
