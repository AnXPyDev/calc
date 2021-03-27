CC=gcc

main: main.o
	$(CC) build/main.o -lm -o build/calc

main.o: src/main.c
	$(CC) -O2 -c src/main.c -o build/main.o

init:
	mkdir build
