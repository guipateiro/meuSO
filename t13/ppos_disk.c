// GRR20197152 GUILHERME COSTA PATEIRO
// Data da ultima modificacao 01/09/2022 15:08

#include "ppos_disk.h"
#include "ppos.h"
#include "queue.h"
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>

extern struct task_t *DRIVERDISCO;
disk_t *DISK;
extern int fim;
extern struct task_t *DISPATCHER;


struct sigaction disco;// estrutura que define um tratador de sinal (deve ser global ou static)

void tratador_disco(){
	DISK->flag = 1; 
	//printf("SIGUSR1\n");
	//printf("task status = %i \n", DRIVERDISCO->status );
	if (DRIVERDISCO->status == SUSPENSA){
		//printf("suspensa?\n");
   		task_resume(DRIVERDISCO,&(DISPATCHER->espera));
      	// acorda o gerente de disco (põe ele na fila de prontas)
   	}
	return;
}

void criatratador_disco(){
	// registra a ação para o sinal SIGINT
	disco.sa_handler = tratador_disco;
	sigemptyset (&disco.sa_mask) ;
	disco.sa_flags = 0 ;

	if (sigaction (SIGUSR1, &disco, 0) < 0){
		perror ("Erro em sigaction: ") ;
		exit (1) ;
	}
} 


void diskDriverBody (){
   	while (!fim){
			// obtém o semáforo de acesso ao disco
		sem_down(&(DISK->semaforo));
		
			// se foi acordado devido a um sinal do disco
		if (DISK->flag == 1){
		    // acorda a tarefa cujo pedido foi atendido
		    //printf("fez algo ?\n");
			task_resume(DISK->queue,&(DISK->queue));
			queue_remove((queue_t**)&DISK->pedidos,(queue_t*)DISK->pedidos);
			DISK->flag = 0;
		}

		  // se o disco estiver livre e houver pedidos de E/S na fila
		if ((disk_cmd (DISK_CMD_STATUS, 0, 0) == 1) && (DISK->queue != NULL)){
			// escolhe na fila o pedido a ser atendido, usando FCFS
		    // solicita ao disco a operação de E/S, usando disk_cmd()
		    //printf("fez pedido\n");
		    disk_cmd(DISK->pedidos->comando,DISK->pedidos->block, DISK->pedidos->buffer);
		}

		  	// libera o semáforo de acesso ao disco
		sem_up(&(DISK->semaforo));
			// suspende a tarefa corrente (retorna ao dispatcher)
		//if (DISK->flag == 0){
		task_suspend(&(DISPATCHER->espera));
		//printf("task status = %i \n", DRIVERDISCO->status );
		task_yield();
		//}	
   }
   task_exit(0);
}

// inicializacao do gerente de disco
// retorna -1 em erro ou 0 em sucesso
// numBlocks: tamanho do disco, em blocos
// blockSize: tamanho de cada bloco do disco, em bytes
int disk_mgr_init (int *numBlocks, int *blockSize){
	DRIVERDISCO = malloc(sizeof(task_t));
	task_create(DRIVERDISCO,diskDriverBody,NULL);
	DISK = malloc(sizeof(disk_t));
	DISK->queue = NULL;
	DISK->pedidos= NULL;
	DISK->flag = 0;
	criatratador_disco();
	sem_create(&(DISK->semaforo), 1);
	if (disk_cmd (DISK_CMD_INIT, 0, 0) < 0){
		perror("erro ao inicializaro disco\n");
		return -1;
	}
	*numBlocks = disk_cmd (DISK_CMD_DISKSIZE, 0, 0) ;
	*blockSize = disk_cmd (DISK_CMD_BLOCKSIZE, 0, 0) ;
	return 0;
}

// leitura de um bloco, do disco para o buffer
int disk_block_read (int block, void *buffer){

  	// obtém o semáforo de acesso ao disco
 	sem_down(&(DISK->semaforo));
   	// inclui o pedido na fila_disco
 	diskcall_t *novo = malloc(sizeof(diskcall_t));
 	novo->comando = DISK_CMD_READ;
 	novo->block = block;
 	novo->buffer = buffer;

 	queue_append((queue_t **)&DISK->pedidos,(queue_t*)novo);

 	//printf("task status = %i \n", DRIVERDISCO->status );
   	if (DRIVERDISCO->status == SUSPENSA){
   		task_resume(DRIVERDISCO,&(DISPATCHER->espera));
      	// acorda o gerente de disco (põe ele na fila de prontas)
   	}
 
   	// libera semáforo de acesso ao disco
 	sem_up(&(DISK->semaforo));
   	// suspende a tarefa corrente (retorna ao dispatcher)
 	task_suspend(&(DISK->queue));
 	task_yield();
 	return 0;
}

// escrita de um bloco, do buffer para o disco
int disk_block_write (int block, void *buffer){

   	// obtém o semáforo de acesso ao disco
 	sem_down(&(DISK->semaforo));
   	// inclui o pedido na fila_disco
 	diskcall_t *novo = malloc(sizeof(diskcall_t));
 	novo->comando = DISK_CMD_WRITE;
 	novo->block = block;
 	novo->buffer = buffer;

 	queue_append((queue_t **)&DISK->pedidos,(queue_t*)novo);

 	//printf("task status = %i \n", DRIVERDISCO->status );
   	if (DRIVERDISCO->status == SUSPENSA){
   		task_resume(DRIVERDISCO,&(DISPATCHER->espera));
      	// acorda o gerente de disco (põe ele na fila de prontas)
   	}
 
   	// libera semáforo de acesso ao disco
 	sem_up(&(DISK->semaforo));
   	// suspende a tarefa corrente (retorna ao dispatcher)
 	task_suspend(&(DISK->queue));
 	task_yield();
 	return 0;
}