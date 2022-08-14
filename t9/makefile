# GRR20197152 GUILHERME COSTA PATEIRO
# Data da ultima modificacao 25/07/2022 21:59 
CFLAGS = -Wall -Wextra
CC = gcc -std=gnu99 -g

all: ppos_sleep

teste: all
	./teste

queue.o: queue.c queue.h
	$(CC) $(CFLAGS) -c $<

ppos_sleep: ppos_core.c queue.o
	$(CC) $(CFLAGS) -o teste $< queue.o pingpong-sleep.c

clear:
	rm -f *.o

purge: clear
	rm -f teste
