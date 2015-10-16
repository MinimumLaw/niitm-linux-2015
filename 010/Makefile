all: tie-tie

run: tie-tie
	ls | ./tie-tie a out.txt && echo -n Result is `cat out.txt`

tie-tie: main
	mv main tie-tie

clean:
	rm -f main tie-tie out.txt

main: main.c
