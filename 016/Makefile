
all: serv

run: serv
	./serv 8888 ./htdocs

clean:
	rm -f main serv

serv: main
	mv main serv

main: main.c