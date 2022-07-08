CFLAGS = -Wall
CC = gcc -std=gnu99

all: ppos_core

teste: all
	./teste1
	./teste2
	./teste3

ppos_core: ppos_core.c
	$(CC) $(CFLAGS) -o teste1 $< pingpong-tasks1.c
	$(CC) $(CFLAGS) -o teste2 $< pingpong-tasks2.c
	$(CC) $(CFLAGS) -o teste3 $< pingpong-tasks3.c

clear:
	rm -f *.o

purge: clear
	rm -f teste1
	rm -f teste2
	rm -f teste3
