// Arquivo modificado 
// GRR20197152 GUILHERME COSTA PATEIRO
// Data da ultima modificacao 31/08/2022 01:22

// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.4 -- Janeiro de 2022

// interface do gerente de disco rígido (block device driver)

#ifndef __DISK_MGR__
#define __DISK_MGR__

// estruturas de dados e rotinas de inicializacao e acesso
// a um dispositivo de entrada/saida orientado a blocos,
// tipicamente um disco rigido.
#include "disk.h"
#include "ppos_data.h"
//estrutura que representa um pedido do disco
typedef struct
{
  // completar com os campos necessarios
	struct diskcall_t *next , *prev;
	int comando;
	int block;
	void *buffer;
} diskcall_t;

// estrutura que representa um disco no sistema operacional
typedef struct
{
  // completar com os campos necessarios
	int flag;
	semaphore_t semaforo;
	task_t *queue;
	diskcall_t *pedidos;
} disk_t ;



// inicializacao do gerente de disco
// retorna -1 em erro ou 0 em sucesso
// numBlocks: tamanho do disco, em blocos
// blockSize: tamanho de cada bloco do disco, em bytes
int disk_mgr_init (int *numBlocks, int *blockSize) ;

// leitura de um bloco, do disco para o buffer
int disk_block_read (int block, void *buffer) ;

// escrita de um bloco, do buffer para o disco
int disk_block_write (int block, void *buffer) ;

#endif
