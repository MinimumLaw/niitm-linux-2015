CPPFLAGS+=-I${PWD}/../include
LDFLAGS=-pthread
all: server

clean:
	rm -f server main

server: main
	mv main server

main: main.c llist.c client.c