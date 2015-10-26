# Uncomment next line for f(x)=x - easy integral calculation (N^2/2)
# CFLAGS = -D_TEST_MATH_
LDFLAGS=-pthread -lm
CC=gcc

all: main

run: main
	./main 10 20

main: main.c

clean:
	rm -f main
