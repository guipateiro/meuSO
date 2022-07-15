// GRR20197152 GUILHERME COSTA PATEIRO
// Data da ultima modificacao 11/07/2022 18:21

#include "ppos_data.h"
#include "ppos.h"
#include "queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <signal.h>
#include <sys/time.h>


//estados do programa
#define TERMINADA 0
#define RODANDO 3
#define DORMINDO 2
#define PRONTA 1

//variaveis globais do core
task_t *MAIN;			//gurada a task main
task_t *MAINAUX;		//guarda o a task que deve ser retornada pelo programas criados (atualmente o dispatcher)
task_t *DISPATCHER;		//guarda a task dispatcher	
task_t *ATUAL;			//guarda a task que esta atualemtne em execusao
int TICK = 20;
int ID = 0;				// id que serao atribuidos as novas tasks
int TAMBUFFER = 65536; 	//1024* 64 memoria alocada para cada contexto
int CODE = 1;			//codigo de retorno da ultima funcao
// estrutura que define um tratador de sinal (deve ser global ou static)
struct sigaction action;

// estrutura de inicialização to timer
struct itimerval timer;
// ajusta valores do temporizador


task_t *FILA_PRONTOS;   //fila de task que estao prontas para serem executada

//funcao qu selecioana a proxima task baseado na priridade dinamica dela
task_t* scheduler(){
	task_t *aux = FILA_PRONTOS;
	task_t *atual = aux;
	//percorre a listae encontra o elemento com menor prioridade
	do{
		if (aux->prio_din < atual->prio_din)
			atual = aux;
		aux = aux->next;
	}while(aux != FILA_PRONTOS);

	//remove o elementoda lista para anao ser afetado pelo decremento de prioridade
	queue_remove((queue_t**)&FILA_PRONTOS,(queue_t*)atual);
	//retorna a prioridade para o original
	atual->prio_din = atual->prio_est;
	//decrementa a prioridade de todos os elementos da fila
	if (queue_size((queue_t*)FILA_PRONTOS) > 0){
		aux = FILA_PRONTOS;
		do{
			aux->prio_din--;
			aux = aux->next;
		}while(aux != FILA_PRONTOS);
	}	
	//coloca o elemento selecionad de volta na fila de prontos
	queue_append((queue_t **)&FILA_PRONTOS,(queue_t*)atual);

	return atual;
}

//funcao de controle de fluxo, trava a volta da funcoes 
void dispacher(){
	//printf("inicio do dispatcher\n");
	MAINAUX = DISPATCHER;
	task_t *next = NULL;
	while(queue_size((queue_t*)FILA_PRONTOS) > 0){
		next = scheduler();
		//reseta o code
		CODE = PRONTA;
		if (next != NULL){
			//a volta do taskswitch modifica o CODE entao CODE muda entre 
			//a atribuicao e agora
			task_switch(next);

			//tratamento de CODE 	
			switch(next->status){
				case TERMINADA:
					queue_remove((queue_t**)&FILA_PRONTOS,(queue_t*)next);
					//printf("tarefa terminada com sucesso\n");
				break;
				case RODANDO:
					//ain nao usado
					//printf("algo esta estranho a tarefa ainda esta rodando\n");
				break;
				case DORMINDO:
					//ainda nao usado
					//printf("tarefa colocada para dormir\n");
				break;
				case PRONTA:
					//ainda nao usado
					//printf("tarefa pronta\n");
				break;
			}
		}
	}
	//recoloca a main para poder fazer um retorno	
	MAINAUX = MAIN;
	task_exit(0);
}

void tratador (){
  if (ATUAL == DISPATCHER || ATUAL == MAIN)
  	return;
  TICK--;
  if(! TICK){
  	TICK = 20;
  	task_yield();
  }
  return;
}

void criatratador(){
	// registra a ação para o sinal SIGINT
	action.sa_handler = tratador ;
	sigemptyset (&action.sa_mask) ;
	action.sa_flags = 0 ;

	if (sigaction (SIGALRM, &action, 0) < 0){
		perror ("Erro em sigaction: ") ;
		exit (1) ;
	}
} 
void criatimer(){

	timer.it_value.tv_usec = 1000;		// primeiro disparo, em micro-segundos
	timer.it_value.tv_sec  = 0;			// primeiro disparo, em segundos
	timer.it_interval.tv_usec = 1000;	// disparos subsequentes, em micro-segundos
	timer.it_interval.tv_sec  = 0;   	// disparos subsequentes, em segundos
	
	if (setitimer (ITIMER_REAL, &timer, 0) < 0){
		perror ("Erro em setitimer: ") ;
		exit (1) ;
	}
}


// Inicializa o sistema operacional; deve ser chamada no inicio do main()
void ppos_init(){

	/* desativa o buffer da saida padrao (stdout), usado pela função printf */
	setvbuf (stdout, 0, _IONBF, 0);

	//cria a estrutura task de main
	MAIN = malloc(sizeof(task_t));
	MAIN->id = ID;
	
	MAIN->status = 1;
	MAIN->preemptable = 0;
	getcontext(&MAIN->context);
	task_setprio(MAIN,0);

	//incrementa a ID e cria a primeira tarefa atual;
	ID++;
	ATUAL = MAIN;
	criatratador();
	criatimer();
	DISPATCHER = malloc(sizeof(task_t));
	task_create(DISPATCHER,dispacher,NULL);


}
// gerência de tarefas =========================================================

// Cria uma nova tarefa. Retorna um ID> 0 ou erro.
int task_create(task_t *task,void (*start_func)(void *),void *arg){

	//cria o contexto
	ucontext_t Context;
	if (getcontext (&Context) < 0){
		perror("erro na criacao de contexto:");
		exit(1);
	}	

	//inicia as variaveis do contexto e aloca o buffer da stack
	char *stack;
	stack = malloc (TAMBUFFER);
	if (stack){
	Context.uc_stack.ss_sp = stack;
		Context.uc_stack.ss_size = TAMBUFFER;
		Context.uc_stack.ss_flags = 0;
		Context.uc_link = 0;
	}
	else{
		perror("Erro na criação da pilha: ") ;
		exit(1) ;
	}

	//inicia as variaveis do contexto
	task->context = Context;
	task->id = ID;
	ID++;
	task->status = PRONTA;
	task->preemptable = 0;
	task_setprio(task,0);

	makecontext(&task->context, (void*)(*start_func), 1, arg);
	if (task != DISPATCHER){
		queue_append((queue_t **)&FILA_PRONTOS,(queue_t*)task);
	}

	return 1;
}

// Termina a tarefa corrente, indicando um valor de status encerramento
// o retorno eh por padrao o dipsatacher mas pode ser setado usando MAINAUX (por exemplo como eh feito no proprio dispatcher)
void task_exit(int exit_code) {
	task_t *aux = ATUAL;
	task_t *nova = MAINAUX;
	ATUAL = MAINAUX;
	aux->status = TERMINADA;
	CODE = exit_code;
	if (swapcontext (&aux->context, &nova->context) < 0){
		perror("erro em troca de contexto: ");
		exit(1);
	}
}

// alterna a execução para a tarefa indicada
int task_switch(task_t *task){
	task_t *aux = ATUAL;
	task_t *nova = task;
	ATUAL = task;
	aux->status = PRONTA;
	task->status = RODANDO;
	if (swapcontext (&aux->context, &nova->context) < 0){
		perror("erro em troca de contexto: ");
		exit(1);
	}
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
	if (prio >= -20 && prio <= 20){
		task->prio_est = prio;
		task->prio_din = prio;
	}
	else{
		perror("prioridade escolhida nao esta no alcance -20 a 20 abortanto");
		exit(1);
	}	
	return;
}

// retorna a prioridade estática de uma tarefa (ou a tarefa atual)
int task_getprio (task_t *task){
	if (task == NULL)
		return ATUAL->prio_est;

	return task->prio_est;
}

