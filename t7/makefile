# GRR20197152 GUILHERME COSTA PATEIRO
# Data da ultima modificacao 25/07/2022 21:59 
CFLAGS = -Wall -Wextra
CC = gcc -std=gnu99

all: ppos_maintask

teste: all
	./teste

queue.o: queue.c queue.h
	$(CC) $(CFLAGS) -g -c $<

ppos_maintask: ppos_core.c queue.o
	$(CC) $(CFLAGS) -g -o teste $< queue.o pingpong-maintask.c

clear:
	rm -f *.o

purge: clear
	rm -f teste
