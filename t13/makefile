# GRR20197152 GUILHERME COSTA PATEIRO
# Data da ultima modificacao 31/08/2022 21:40 
CFLAGS = -Wall -Wextra 
CC = gcc -std=gnu99 -g

all: ppos_disco

teste: all
	./teste

queue.o: queue.c queue.h
	$(CC) $(CFLAGS) -c $<

disk.o:disk.c disk.h
	$(CC) $(CFLAGS)	-c $<

ppos_disk.o:ppos_disk.c ppos_disk.h
	$(CC) $(CFLAGS)	-c $<

ppos_disco: ppos_core.c ppos_disk.o queue.o disk.o
	$(CC) $(CFLAGS) -o teste $^  pingpong-disco1.c -lrt
	echo "para executar ./teste ou make teste"

ppos_disco2: ppos_core.c ppos_disk.o queue.o disk.o
	$(CC) $(CFLAGS) -o teste $^  pingpong-disco2.c -lrt	
	echo "para executar ./teste ou make teste"

clear:
	rm -f *.o

purge: clear
	rm -f teste
