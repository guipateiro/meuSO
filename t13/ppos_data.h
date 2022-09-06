// Arquivo modificado 
// GRR20197152 GUILHERME COSTA PATEIRO
// Data da ultima modificacao 06/09/2022 20:17

// PROJETO ORIGINAL FEITO POR:
// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.4 -- Janeiro de 2022

// Estruturas de dados internas do sistema operacional

#ifndef __PPOS_DATA__
#define __PPOS_DATA__

#include <ucontext.h>		// biblioteca POSIX de trocas de contexto
#include <string.h>

//estados do programa
#define TERMINADA 0
#define SUSPENSA 4
#define RODANDO 3
#define DORMINDO 2
#define PRONTA 1

// Estrutura que define um Task Control Block (TCB)
typedef struct task_t
{
  struct task_t *prev, *next ;		// ponteiros para usar em filas
  int id ;				                // identificador da tarefa
  ucontext_t context ;			      // contexto armazenado da tarefa
  short status ;			            // pronta, rodando, suspensa, ...
  short preemptable ;			        // pode ser preemptada?
  short prio_est;                 // prioridade estatica
  short prio_din;                 // prioridade dinamica
  int ativacoes;                  // numero de ativações
  unsigned int tempo_exec;        // tempo (em ms) que a tarefa esteve no processador
  unsigned int tempo_inic;        // tempo (no relogio do sistema) que a tarefa iniciou
  unsigned int tempo_ultimo_disparo; // tempo (no relogio do sistema) que a terafa ganhou a cpu da ultima vez 
  int exit_code;                  // codigo de saida da terefa ao ser finalizada
  struct task_t *espera;          // fila contendo as tarefas que esperam essa tarefa ser finalizada
  unsigned int sleep;             // tempo (no relogio do sistema) que a terefa deve acordar

  // ... (outros campos serão adicionados mais tarde)
} task_t ;

// estrutura que define um semáforo
typedef struct
{
  int counter;            // numero de instancias
  task_t *queue;          // queue de espera
} semaphore_t ;


// estrutura que define um mutex
typedef struct
{
	int i;  // preencher quando necessário
} mutex_t ;

// estrutura que define uma barreira
typedef struct
{
 int i; // preencher quando necessário
} barrier_t ;

// estrutura que define uma fila de mensagens
typedef struct
{
  
  semaphore_t p_buffer, s_c, s_p; // semaforos
  int tam_buffer;                 // numero de mensagens no buffer
  int tam_msg;                    // tamanho da mensagem
  void *buffer;                   // buffer circular
  int indice_p, indice_c ;        // indice de leitura e escrita
} mqueue_t ;

#endif


