CC = gcc
CFLAGS = -Wall -g -O2
ALLEG = `allegro-config --libs`
CONF = -Iconf
PTHREAD = -lpthread -lrt

.PHONY: all clean

all: main

clean:
	rm -f bin/*
	rm -f build/*


################
# Object files #
################

build/ant.o: src/ant.c
	$(CC) $(CONF) -c src/ant.c -o build/ant.o

build/main.o: src/main.c
	$(CC) $(CONF) -c src/main.c -o build/main.o

build/multimedia.o: src/multimedia.c
	$(CC) $(CONF) -c src/multimedia.c -o build/multimedia.o


################
# Executables  #
################

main: build/main.o build/ant.o build/multimedia.o
	$(CC) $(ALLEG) $(PTHREAD) \
	build/main.o \
	build/ant.o \
	build/multimedia.o \
	-o bin/main
