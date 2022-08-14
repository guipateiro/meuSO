// GRR20197152 GUILHERME COSTA PATEIRO
// Data da ultima modificacao 25/07/2022 21:57

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
#define SUSPENSA 4
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
unsigned int clock = 0;
// estrutura que define um tratador de sinal (deve ser global ou static)
struct sigaction action;
int tarefas_ativas = 0;

// estrutura de inicialização to timer
struct itimerval timer;
// ajusta valores do temporizador

task_t *FILA_PRONTOS 	= NULL ;   //fila de task que estao prontas para serem executada
task_t *FILA_SUSPENSA	= NULL;  //fila de tarefas suspensas
task_t *FILA_TERMINADA 	= NULL ; 
task_t *FILA_DORMITORIO	= NULL ; 

void verificajoin(task_t *task){
	task_t *aux = FILA_SUSPENSA;
	
	if (queue_size((queue_t*)FILA_SUSPENSA) > 0){
		do{
			if (aux->espera == task){
				task_resume(aux,&FILA_SUSPENSA);
				if(queue_size((queue_t*)FILA_SUSPENSA) == 0)
					return;
			}	
			else{
				aux = aux->next;
			}
		}while(aux != FILA_SUSPENSA);
	}
	
	return;
}

void verifica_dormitorio(){
	
	task_t *aux = FILA_DORMITORIO;
	
	if (queue_size((queue_t*)FILA_DORMITORIO) > 0){
		do{
			if (aux->sleep <= systime()){
				task_resume(aux,&FILA_DORMITORIO);
				if(queue_size((queue_t*)FILA_DORMITORIO) == 0)
					return;
			}	
			else{
				aux = aux->next;
			}
		}while(aux != FILA_DORMITORIO);
	}
	return;
}

//funcao qu selecioana a proxima task baseado na priridade dinamica dela
task_t* scheduler(){
	task_t *aux = FILA_PRONTOS;
	task_t *atual = aux;
	//percorre a listae encontra o elemento com menor prioridade
	do{
		aux->prio_din--;
		if (aux->prio_din < atual->prio_din)
			atual = aux;
		aux = aux->next;
	}while(aux != FILA_PRONTOS);

	//retorna a prioridade para o original
	atual->prio_din = atual->prio_est;
	
	return atual;
}

//funcao de controle de fluxo, trava a volta da funcoes 
void dispacher(){
	//printf("inicio do dispatcher\n");
	MAINAUX = DISPATCHER;
	task_t *next = NULL;
	//printf ("TAREFAS : %i ", queue_size((queue_t*)FILA_PRONTOS));
	//fflush(stdout);
	while(tarefas_ativas > 0){
		next = NULL;
		if (queue_size((queue_t*)FILA_DORMITORIO) > 0){
			verifica_dormitorio();
		}

		if (queue_size((queue_t*)FILA_PRONTOS) > 0){
			next = scheduler();
		}
		else{
			sleep(1);
		}
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
					tarefas_ativas--;
					queue_append((queue_t**)&FILA_TERMINADA,(queue_t*)next);
					//printf("tarefa terminada com sucesso\n");
				break;
				case RODANDO:
					//ain nao usado
					//printf("algo esta estranho a tarefa ainda esta rodando\n");
				break;
				case DORMINDO:
					printf("colocado no dormitorio\n\n");
					//queue_remove((queue_t**)&FILA_PRONTOS,(queue_t*)next);
					//queue_append((queue_t**)&FILA_DORMITORIO,(queue_t*)next);
				break;
				case PRONTA:
					//ainda nao usado
					//printf("tarefa pronta\n");
				break;
			}
		}
	}
	printf("tarefas terminadas: %i\n",queue_size((queue_t*)FILA_TERMINADA));
	//recoloca a main para poder fazer um retorno	
	//MAINAUX = MAIN;
	task_exit(0);
}

void tratador (){
	clock++;
	if (ATUAL == DISPATCHER)
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
		perror ("Erro em set timer: ");
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
	MAIN->preemptable = 1;
	MAIN->status = PRONTA;
	MAIN->ativacoes = 0;
	getcontext(&MAIN->context);
	task_setprio(MAIN,0);
	queue_append((queue_t **)&FILA_PRONTOS,(queue_t*)MAIN);
	//incrementa a ID e cria a primeira tarefa atual;
	ID++;
	ATUAL = MAIN;
	criatratador();
	criatimer();
	tarefas_ativas++;
	DISPATCHER = malloc(sizeof(task_t));
	task_create(DISPATCHER,dispacher,NULL);
	task_yield();


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
	task->preemptable = 1;
	task->ativacoes = 0;
	task->tempo_ultimo_disparo = systime();
	task->tempo_exec = 0;
	task_setprio(task,0);

	makecontext(&task->context, (void*)(*start_func), 1, arg);
	if (task != DISPATCHER){
		queue_append((queue_t **)&FILA_PRONTOS,(queue_t*)task);
		tarefas_ativas++;
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
	aux->exit_code = exit_code;
	aux->tempo_exec += systime() - aux->tempo_ultimo_disparo;
	printf("Task %i exit: execution time %i ms, processor time %i ms, %i activations \n",aux->id, (systime() - aux->tempo_inic), aux->tempo_exec, aux->ativacoes );
	if (aux == DISPATCHER){
		exit(exit_code);
	}
	nova->status = RODANDO;
	nova->ativacoes ++;
	nova->tempo_ultimo_disparo = systime();

	verificajoin(aux);
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
	aux->tempo_exec += systime() - aux->tempo_ultimo_disparo;
	if (task->ativacoes == 0)
		task->tempo_inic = systime();
	task->status = RODANDO;
	task->ativacoes ++;
	task->tempo_ultimo_disparo = systime();
	TICK = 20;
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

unsigned int systime(){
	return clock;
}

void task_suspend (task_t **queue) {
	task_t *aux = ATUAL;
	if (queue_remove((queue_t**)&FILA_PRONTOS,(queue_t*)aux) != 0){
		return;
	}
	aux->status = SUSPENSA;
	queue_append((queue_t **)queue,(queue_t*)aux);
	return;
}

void task_resume(task_t *task, task_t **queue) {
	if (queue_remove((queue_t**)queue,(queue_t*)task) != 0){
		return;
	}
	task->status = PRONTA;
	queue_append((queue_t **)&FILA_PRONTOS,(queue_t*)task);
	return;
}

int task_join (task_t *task){
	ATUAL->espera = task;
	
	task_t *aux = FILA_TERMINADA;
	if (queue_size((queue_t*)FILA_TERMINADA) > 0){
		do{
			if (aux == task){
				return aux->exit_code;	
			}
			aux = aux->next;
		}while(aux != FILA_TERMINADA);
	}

	int flag = 0;
	aux = FILA_PRONTOS;

	if (queue_size((queue_t*)FILA_PRONTOS) > 0){
		do{
			if (aux == task){
				flag = 1;	
				break;
			}
			aux = aux->next;
		}while(aux != FILA_PRONTOS);
	}

	if(flag == 1){
		task_suspend(&FILA_SUSPENSA);
		task_yield();
		return ATUAL->espera->exit_code;
	}
	return -1;	
}

void task_sleep(int i){
	printf("algo passou aqui\n");
	task_t *aux = ATUAL;
	if (queue_remove((queue_t**)&FILA_PRONTOS,(queue_t*)aux) != 0){
		return;
	}
	ATUAL->sleep = systime() + i;
	ATUAL->status = DORMINDO;
	queue_append((queue_t **)&FILA_DORMITORIO,(queue_t*)aux);
	task_yield();
	return;
}