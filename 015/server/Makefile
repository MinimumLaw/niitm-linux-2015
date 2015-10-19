CPPFLAGS=-I${PWD}/../include
LDFLAGS=-lpthread
all: server

clean:
	rm -f server main

server: main
	mv main server

main: main.c llist.c client.c