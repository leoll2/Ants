CC = gcc
CFLAGS = -Wall -g -O2
CONF = -I.
ALLEG = `allegro-config --libs`
PTHREAD = -lpthread -lrt 
MATH = -lm

.PHONY: all clean

all: bin/main

clean:
	rm -f bin/* build/*
	rmdir bin/ build/


################
# Object files #
################

build/main.o: src/main.c
	mkdir -p build/
	$(CC) -c $(CONF) $< -o $@

build/%.o: src/%.c src/%.h
	mkdir -p build/
	$(CC) -c $(CONF) $< -o $@


################
# Executables  #
################

bin/main: build/main.o build/ant.o build/field.o build/multimedia.o build/rt_thread.o
	mkdir -p bin/
	$(CC) $^ -o $@ $(ALLEG) $(PTHREAD) $(MATH)


