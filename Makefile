CC = gcc
CFLAGS = -Wall -g -O2
ALLEG = `allegro-config --libs`
CONF = -Iconf

.PHONY: all clean

all: main

clean:
	rm -f bin/*
	rm -f build/*

################
# Object files #
################

build/main.o: src/main.c
	$(CC) $(CONF) -c src/main.c -o build/main.o


################
# Executables  #
################

main: build/main.o
	$(CC) $(ALLEG) build/main.o -o bin/main


