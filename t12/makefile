# GRR20197152 GUILHERME COSTA PATEIRO
# Data da ultima modificacao 25/08/2022 01:07 
CFLAGS = -Wall -Wextra
CC = gcc -std=gnu99 #-g

all: ppos_mqueue

teste: all
	./teste

queue.o: queue.c queue.h
	$(CC) $(CFLAGS) -c $<

ppos_mqueue: ppos_core.c queue.o
	$(CC) $(CFLAGS) -o teste $< queue.o pingpong-mqueue.c -lm

clear:
	rm -f *.o

purge: clear
	rm -f teste
