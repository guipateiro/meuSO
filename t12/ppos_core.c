// GRR20197152 GUILHERME COSTA PATEIRO
// Data da ultima modificacao 25/08/2022 21:34

#include "ppos_data.h"
#include "ppos.h"
#include "queue.h"
#include <stdio.h>
#include <stdlib.h>
 #include <unistd.h>
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
task_t *MAINAUX;		//guarda o a task que deve ser retornada pelos programas criados (atualmente o dispatcher)
task_t *DISPATCHER;		//guarda a task dispatcher	
task_t *ATUAL;			//guarda a task que esta atualemtne em execusao
int TICK = 20;			// tamanho do quantum 
int ID = 0;				// id que serao atribuidos as novas tasks
int TAMBUFFER = 65536; 	//1024* 64 memoria alocada para cada contexto
unsigned int clock = 0;	//relogio do sistema
struct sigaction action;// estrutura que define um tratador de sinal (deve ser global ou static)
int tarefas_ativas = 0;	// contador de tarefas ativas
struct itimerval timer; // estrutura de inicialização to timer

task_t *FILA_PRONTOS 	= NULL ;   	//fila de task que estao prontas para serem executada
task_t *FILA_SUSPENSA	= NULL ;  	//fila de tarefas suspensas
task_t *FILA_TERMINADA 	= NULL ; 	//fila de tarefas terminadas, usada no task join
task_t *FILA_DORMITORIO	= NULL ; 	//fila de tarefas dormindo



void verificajoin(task_t *task){
	task_t *aux = FILA_SUSPENSA;
	
	if (queue_size((queue_t*)FILA_SUSPENSA) > 0){
		do{
			if (aux->espera == task){
				task_resume(aux,&FILA_SUSPENSA);
				if(queue_size((queue_t*)FILA_SUSPENSA) > 0)
					aux = FILA_SUSPENSA->next;
				else{
					return;
				}
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
	
	do{
		if (aux->sleep <= systime()){
			task_resume(aux,&FILA_DORMITORIO);
			if(queue_size((queue_t*)FILA_DORMITORIO) > 0)
				aux = FILA_DORMITORIO->next;
			else{
				return;
			}
		}	
		else{
			aux = aux->next;
		}
	}while(aux != FILA_DORMITORIO);
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
		//verifica a fila de tarefas dormindo caso ela nao esteja vazia
		if (queue_size((queue_t*)FILA_DORMITORIO) > 0){
			verifica_dormitorio();
		}

		//escolhe a proxima tarefa ativa
		//printf("tarefas ativas :%i \n" ,queue_size((queue_t*)FILA_PRONTOS));
		if (queue_size((queue_t*)FILA_PRONTOS) > 0){
			next = scheduler();
		}
		else{
			//caso nao tenha tarefas ativas dorme por 1 quantum (20ms)
			sleep(1);
		}

		if (next != NULL){

			task_switch(next);
	
			switch(next->status){
				case TERMINADA:
					queue_remove((queue_t**)&FILA_PRONTOS,(queue_t*)next);
					tarefas_ativas--;
					queue_append((queue_t**)&FILA_TERMINADA,(queue_t*)next);
				break;
				case RODANDO:
					//ainda nao usado (pelo visto nunca vai ser)
					//printf("algo esta estranho a tarefa ainda esta rodando\n");
				break;
				case DORMINDO:
					//printf("colocado no dormitorio\n\n");
					//queue_remove((queue_t**)&FILA_PRONTOS,(queue_t*)next);
					//queue_append((queue_t**)&FILA_DORMITORIO,(queue_t*)next);
				break;
				case PRONTA:
					//queue_remove((queue_t**)&FILA_DORMITORIO,(queue_t*)next);
					//queue_append((queue_t**)&FILA_PRONTOS,(queue_t*)next);
				break;
			}
		}
	}
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
	//CODE = exit_code;
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

//retorna o relogio do sistema atual
unsigned int systime(){
	return clock;
}

//suspende a terefa ATUAL e a coloca na lista apontada por queue 
//chamar um task_yeld() apos utilizar essa funcao para evitar erros
void task_suspend (task_t **queue) {
	task_t *aux = ATUAL;
	if (queue_remove((queue_t**)&FILA_PRONTOS,(queue_t*)aux) != 0){
		return;
	}
	aux->status = SUSPENSA;
	queue_append((queue_t **)queue,(queue_t*)aux);
	return;
}

//retorna a task "task" que esta na fila "queue" para a filade terfas prontas
void task_resume(task_t *task, task_t **queue) {
	if (queue_remove((queue_t**)queue,(queue_t*)task) != 0){
		return;
	}
	task->status = PRONTA;
	queue_append((queue_t **)&FILA_PRONTOS,(queue_t*)task);
	return;
}

//faz a tarefa atual esperar o termino de "task"
//caso "task" ja tennha terminado nao faz nada
//caso "task" ainda esteja em execusao suspende a tarefa atual
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

	task_suspend(&FILA_SUSPENSA);
	task_yield();
	return ATUAL->espera->exit_code;
}

//pausa a execusao da tarefa atual por "i" milisegundos
void task_sleep(int i){
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


//cria um semaforo contendo "i" intancias
int sem_create (semaphore_t *s, int value) {
	if (value < 0){
		perror("tamanho do semaforo deve ser um numero positivo");
		return -1;
	}

    s->counter = value;
    s->queue = NULL;

    return 0;
}

//faz down no semaforo
//caso counter seja negativo suspende as tarefas que tentarem dar down ate alguma tarefa dar up 
int sem_down (semaphore_t *s) {
    if(s == NULL){
        return -1;
    }
    s->counter--;
    if(s->counter < 0){
        task_suspend(&(s->queue));
        task_yield();
        if(s == NULL){
            return -1;
        }
    }
    
    return 0;
}

//faz up no semaforo
//acorda 1 tarefa caso a fila do semaforo seja nao vazia
int sem_up (semaphore_t *s) {
    if(s == NULL) {
        return -1;
    }

    s->counter++;
    if(queue_size((queue_t*)s->queue) > 0){
        task_resume(s->queue,&(s->queue));
    }

    return 0;
}

//destroi o semaforo
//acorda todas as tarefas que esperam pela semaforo 
int sem_destroy (semaphore_t *s) {
    while(queue_size((queue_t*)s->queue) > 0){
        task_resume(s->queue,&(s->queue));
    }   
    s->counter = 0;
    s = NULL;
    return 0;
}

//cria uma fila de mensagens com "max" mensagens de tamanho "size" cada
int mqueue_create (mqueue_t *queue, int max, int size){
    queue->tam_buffer = max;
    queue->tam_msg = size;
    sem_create(&(queue->p_buffer), 1);
    sem_create(&(queue->s_c), 0);
    sem_create(&(queue->s_p), queue->tam_buffer);
   	queue->indice_p = 0;
    queue->indice_c = 0;
    queue->buffer = malloc(queue->tam_buffer * queue->tam_msg);
    if(!queue->buffer){
        perror("erro na alocacao da fila de mensagens");
        return -1;
    }
    return 0;
}

//envia uma mensagem "msg" para a fila "queue"
int mqueue_send (mqueue_t *queue, void *msg) {
    if(queue->tam_buffer == -1)
    	return -1;
    if((sem_down(&(queue->s_p)) < 0) || (sem_down(&(queue->p_buffer)) < 0))
    	return -1;
    if(queue->tam_buffer == -1)
		return -1;

    memcpy(((queue->buffer)+ (queue->indice_p * queue->tam_msg)), msg, queue->tam_msg);
    queue->indice_p++;
    queue->indice_p = queue->indice_p % queue->tam_buffer;

    if((sem_up(&(queue->p_buffer)) < 0) || (sem_up(&(queue->s_c)) < 0))
    	return -1;
    return 0;
}

//pega uma mensagem "msg" da fila "queue"
int mqueue_recv (mqueue_t *queue, void *msg) {
	if(queue->tam_buffer == -1)
		return -1;
    if((sem_down(&(queue->s_c)) < 0) ||(sem_down(&(queue->p_buffer)) < 0))
    	return -1; 
    if(queue->tam_buffer == -1)
		return -1;

    memcpy(msg, (queue->buffer) + (queue->indice_c * queue->tam_msg), queue->tam_msg);
    queue->indice_c++;
    queue->indice_c = queue->indice_c % queue->tam_buffer;

    if((sem_up(&(queue->p_buffer)) < 0) || (sem_up(&(queue->s_p)) < 0)) 
    	return -1;

    return 0;
}

//finaliza uma fila de mensagens, 
int mqueue_destroy (mqueue_t *queue) {
    sem_destroy(&(queue->p_buffer));
    sem_destroy(&(queue->s_c));
    sem_destroy(&(queue->s_p));
    //xunxo, gambiarra, engenharia de emergencia, artificio tecnico, asset de codigo
    //nao sei como fazer diferente disso mas é o unico metodo que eu achei para lidar com o travamento pos feinalizacao
    sem_create(&(queue->p_buffer), 100);
    sem_create(&(queue->s_c), 100);
    sem_create(&(queue->s_p), 100);
    
    free(queue->buffer);

    queue->tam_buffer = -1;
    queue->tam_msg = 0;

    return 1;
}