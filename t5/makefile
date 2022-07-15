# GRR20197152 GUILHERME COSTA PATEIRO
# Data da ultima modificacao 11/07/2022 18:03 
CFLAGS = -Wall -Wextra
CC = gcc -std=gnu99

all: ppos_core

teste: all
	./teste

queue.o: queue.c queue.h
	$(CC) $(CFLAGS) -g -c $<

ppos_core: ppos_core.c queue.o
	$(CC) $(CFLAGS) -g -o teste $< queue.o pingpong-preempcao.c

stress: ppos_core.c queue.o
	$(CC) $(CFLAGS) -g -o teste $< queue.o pingpong-preempcao-stress.c	

clear:
	rm -f *.o

purge: clear
	rm -f teste
