CFLAGS = -Wextra
CC = gcc -std=gnu99

all: ppos_core

teste: all
	./teste

queue.o: queue.c queue.h
	$(CC) $(CFLAGS) -g -c $<

ppos_core: ppos_core.c queue.o
	$(CC) $(CFLAGS) -g -o teste $< queue.o pingpong-dispatcher.c

clear:
	rm -f *.o

purge: clear
	rm -f teste
