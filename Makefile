CC=gcc
BINDEPS=src/color.h src/colormode.h src/imode.h src/fmode.h src/hexmode.h src/submode.h
FLAGS=-O3

main: calc

calc: $(BINDEPS)
	$(CC) -Wall $(FLAGS) -lm src/main.c -o calc

install: calc
	cp ./calc /usr/bin/calc

init:
	mkdir build
