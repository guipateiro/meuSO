
#include "ppos_data.h"
#include "ppos.h"
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>

task_t *MAIN;
task_t *ATUAL;
int ID = 0;
int TAMBUFFER = 65536; //1024* 64

// Inicializa o sistema operacional; deve ser chamada no inicio do main()
void ppos_init () {

	/* desativa o buffer da saida padrao (stdout), usado pela função printf */
	setvbuf (stdout, 0, _IONBF, 0);

	MAIN = malloc(sizeof(task_t));
	MAIN->id = ID;
	ID++;
	MAIN->status = 1;
	MAIN->preemptable = 0;
	getcontext(&MAIN->context);

	ATUAL = MAIN;

}
// gerência de tarefas =========================================================

// Cria uma nova tarefa. Retorna um ID> 0 ou erro.
int task_create (task_t *task,void (*start_func)(void *),void *arg){

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


   	makecontext (&task->context, (void*)(*start_func), 1, arg);
   	return 1;
}

// Termina a tarefa corrente, indicando um valor de status encerramento
void task_exit (int exit_code) {
	task_t *aux = ATUAL;
	task_t *nova = MAIN;
	ATUAL = MAIN;
	swapcontext (&aux->context, &nova->context);
}

// alterna a execução para a tarefa indicada
int task_switch (task_t *task){
	task_t *aux = ATUAL;
	task_t *nova = task;
	ATUAL = task;
	swapcontext (&aux->context, &nova->context);
	return 0;
}

// retorna o identificador da tarefa corrente (main deve ser 0)
int task_id (){
		return (ATUAL->id);
}