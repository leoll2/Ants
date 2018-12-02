CC = gcc
CFLAGS = -Wall -g -O2
ALLEG = `allegro-config --libs`
CONF = -I.
PTHREAD = -lpthread -lrt -lm

.PHONY: all clean

all: main

clean:
	rm -f bin/*
	rm -f build/*


################
# Object files #
################

build/ant.o: src/ant.c src/ant.h
	$(CC) -c $(CONF) src/ant.c -o build/ant.o

build/field.o: src/field.c src/field.h
	$(CC) -c $(CONF) src/field.c -o build/field.o

build/main.o: src/main.c
	$(CC) -c $(CONF) src/main.c -o build/main.o

build/multimedia.o: src/multimedia.c src/multimedia.h
	$(CC) -c $(CONF) src/multimedia.c -o build/multimedia.o

build/rt_thread.o: src/rt_thread.c src/rt_thread.h
	$(CC) -c $(CONF) src/rt_thread.c -o build/rt_thread.o


################
# Executables  #
################

main: build/main.o build/ant.o build/field.o build/multimedia.o build/rt_thread.o
	$(CC) \
	build/main.o \
	build/ant.o \
	build/field.o \
	build/multimedia.o \
	build/rt_thread.o \
	-o bin/main $(ALLEG) $(PTHREAD) 


