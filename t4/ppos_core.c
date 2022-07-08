
#include "ppos_data.h"
#include "ppos.h"
#include "queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>

#define TERMINADA 0
#define RODANDO 3
#define DORMINDO 2
#define PRONTA 1
task_t *MAIN;
task_t *MAINAUX;
task_t *DISPATCHER;
task_t *ATUAL;
int ID = 0;
int TAMBUFFER = 65536; //1024* 64
int CODE = 1;

task_t *FILA_PRONTOS;

task_t* scheduler(){
	task_t *aux = FILA_PRONTOS;
	task_t *atual = aux;
	do{
		if (aux->prio_din <= atual->prio_din)
			atual = aux;
		aux = aux->next;
	}while(aux != FILA_PRONTOS);


	queue_remove((queue_t**)&FILA_PRONTOS,(queue_t*)atual);
	atual->prio_din = atual->prio_est;
	//printf("escolhido elemento com prioridade %i \n", atual->prio_din);
	if (queue_size((queue_t*)FILA_PRONTOS) > 0){
		aux = FILA_PRONTOS;
		do{
			aux->prio_din--;
			aux = aux->next;
		}while(aux != FILA_PRONTOS);
	}	
	queue_append((queue_t **)&FILA_PRONTOS,(queue_t*)atual);

	return atual;
}

void dispacher(){
	//printf("inicio do dispatcher\n");
	MAINAUX = DISPATCHER;
	task_t *next = NULL;
	while(queue_size((queue_t*)FILA_PRONTOS) > 0){
		next = scheduler();
		CODE = PRONTA;
		if (next != NULL){
			task_switch(next);

			switch(CODE){
				case TERMINADA:
					queue_remove((queue_t**)&FILA_PRONTOS,(queue_t*)next);
					//printf("tarefa terminada com sucesso\n");
				break;
				case RODANDO:
					//printf("algo esta entranho a tarefa ainda esta roando\n");
				break;
				case DORMINDO:
					//printf("tarefa dormindo\n");
				break;
				case PRONTA:
					//printf("tarefa dormindo\n");
				break;
			}
		}
	}
	//printf("fim do dispatcher\n");	
	MAINAUX = MAIN;
	task_exit(0);
}

// Inicializa o sistema operacional; deve ser chamada no inicio do main()
void ppos_init(){

	/* desativa o buffer da saida padrao (stdout), usado pela função printf */
	setvbuf (stdout, 0, _IONBF, 0);

	MAIN = malloc(sizeof(task_t));
	MAIN->id = ID;
	ID++;
	MAIN->status = 1;
	MAIN->preemptable = 0;
	getcontext(&MAIN->context);
	task_setprio(MAIN,0);

	ATUAL = MAIN;

	DISPATCHER = malloc(sizeof(task_t));
	task_create(DISPATCHER,dispacher,NULL);


}
// gerência de tarefas =========================================================

// Cria uma nova tarefa. Retorna um ID> 0 ou erro.
int task_create(task_t *task,void (*start_func)(void *),void *arg){

	ucontext_t Context;
	getcontext (&Context);

	char *stack;
   	stack = malloc (TAMBUFFER);
   	if (stack){
    	Context.uc_stack.ss_sp = stack;
      	Context.uc_stack.ss_size = TAMBUFFER;
      	Context.uc_stack.ss_flags = 0;
      	Context.uc_link = 0;
   	}
   	else{
      	perror ("Erro na criação da pilha: ") ;
      	exit (1) ;
   	}


   	task->context = Context;
   	task->id = ID;
   	ID++;
   	task->status = 1;
   	task->preemptable = 0;
   	task_setprio(task,0);

   	makecontext (&task->context, (void*)(*start_func), 1, arg);
   	if (task != DISPATCHER){
   		queue_append((queue_t **)&FILA_PRONTOS,(queue_t*)task);
   		//printf("tamanho: %i\n", queue_size((queue_t*)FILA_PRONTOS));
   	}

   	return 1;
}

// Termina a tarefa corrente, indicando um valor de status encerramento
void task_exit(int exit_code) {
	task_t *aux = ATUAL;
	task_t *nova = MAINAUX;
	ATUAL = MAINAUX;
	CODE = exit_code;
	swapcontext (&aux->context, &nova->context);
}

// alterna a execução para a tarefa indicada
int task_switch(task_t *task){
	task_t *aux = ATUAL;
	task_t *nova = task;
	ATUAL = task;
	swapcontext (&aux->context, &nova->context);
	return 0;
}

// retorna o identificador da tarefa corrente (main deve ser 0)
int task_id(){
		return (ATUAL->id);
}



// libera o processador para a próxima tarefa, retornando à fila de tarefas
// prontas ("ready queue")
void task_yield(){
	task_switch(DISPATCHER);
}

// define a prioridade estática de uma tarefa (ou a tarefa atual)
void task_setprio (task_t *task, int prio){
	if (task == NULL){
		ATUAL->prio_est = prio;
		ATUAL->prio_din = prio;
		return;
	}
	task->prio_est = prio;
	task->prio_din = prio;
	return;
}

// retorna a prioridade estática de uma tarefa (ou a tarefa atual)
int task_getprio (task_t *task){
	if (task == NULL)
		return ATUAL->prio_est;

	return task->prio_est;
}

