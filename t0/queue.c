// GRR20197152 GUILHERME COSTA PATEIRO
// Data da ultima modificacao 20/06/2022 02:39 

#include "queue.h"
#include <stdio.h>

//------------------------------------------------------------------------------
// Conta o numero de elementos na fila
// Retorno: numero de elementos na fila

int queue_size (queue_t *queue){
   //caso onde a lista nao existe ou seja esta vazia
   if (queue == NULL)
      return 0;
   //caso nao esteja vazia tem pelo menos 1 elemento 
   int tam = 0;
   queue_t *aux = queue;
   //percorre a lista contando os elementos
   do{
      tam++;
      aux = aux->next;
   }while(aux != queue);
   return tam;
}   



//------------------------------------------------------------------------------
// Percorre a fila e imprime na tela seu conteúdo. A impressão de cada
// elemento é feita por uma função externa, definida pelo programa que
// usa a biblioteca. Essa função deve ter o seguinte protótipo:
//
// void print_elem (void *ptr) ; // ptr aponta para o elemento a imprimir

void queue_print (char *name, queue_t *queue, void print_elem (void*) ) {
   // imprime o cabecalho
   printf("%s:\t[", name);

   // se a fila não tiver elementos nao tem o que imprimir
   if(queue_size(queue) == 0){
      printf("]\n");
      return;
   }

   queue_t *aux = queue;
   do{

      print_elem(aux);
      printf(" "); 
      aux = aux->next;

   }while(aux != queue->prev);
   print_elem(aux);
   printf("]\n");
}


//------------------------------------------------------------------------------
// Insere um elemento no final da fila.
// Condicoes a verificar, gerando msgs de erro:
// - a fila deve existir
// - o elemento deve existir
// - o elemento nao deve estar em outra fila
// Retorno: 0 se sucesso, <0 se ocorreu algum erro

int queue_append (queue_t **queue, queue_t *elem){
   
   //caso o elemeto nao exista
   if (elem == NULL){
      fprintf(stderr, "O elemento nao existe !\n");
      return -2;
   }

   //caso onde o elemento ja esta em uma lista
   if (elem->next != NULL || elem->prev != NULL){
      fprintf(stderr, "O elemento ja esta em uma fila !\n");
      return -3;
   } 
   // caso a lista esteja vazia cria o primeiro elemento
   if(queue_size(*queue) == 0){
      *queue = elem;
      elem->next = elem;
      elem->prev = elem;
      return 0;
   }

   //ajusta os ponteiros para o novo elemento
   (*queue)->prev->next = elem;
   elem->prev = (*queue)->prev;
   (*queue)->prev = elem;
   elem->next = *queue;
   return 0;
}


//------------------------------------------------------------------------------
// Remove o elemento indicado da fila, sem o destruir.
// Condicoes a verificar, gerando msgs de erro:
// - a fila deve existir
// - a fila nao deve estar vazia
// - o elemento deve existir
// - o elemento deve pertencer a fila indicada
// Retorno: 0 se sucesso, <0 se ocorreu algum erro

int queue_remove(queue_t **queue, queue_t *elem){

   // elemento nao existe
   if(elem == NULL){
      fprintf(stderr, "O elemento nao existe !\n");
      return -1;
   }
   // caso a lista esteja vazia nao tem o que remover
   if(queue_size(*queue) == 0){
      fprintf(stderr, "A fila esta vazia !\n");
      return -2;
   }

   //caso o elemento ja nao esteja em nenhuma lista
   if(elem->next == NULL || elem->prev == NULL){
      fprintf(stderr, "O elemento nao esta em nenhuma fila !\n");
      return -3;
   }

   // caso queira remover o unico elemento da fila
   if(queue_size(*queue) == 1 && *queue == elem){
      // desacopla o elemento e destoi a fila
      *queue = NULL;
      elem->next = NULL;
      elem->prev = NULL;
      return 0;   
   }

   //caso elem seja igual ao primeiro elemento
   if(*queue == elem){
      queue_t *aux = (*queue)->next;
      // reorganiza os poteiros
      (*queue)->prev->next = (*queue)->next;
      (*queue)->next->prev = (*queue)->prev;

      //desacopla o elemento sem destrui-lo
      elem->next = NULL;
      elem->prev = NULL;

     *queue = aux;
     return 0;
   }

   // percorre a fila ate dar 1 volta ou achar o elemento
    queue_t *aux = *queue;
    do{
        aux = aux->next;
    }while(aux != *queue && aux != elem);

    // se deu a volta e nao encontrou o elemento 
    if(aux == *queue){
        fprintf(stderr, "O elemento nao esta nesta fila !\n");
        return -4;
    }

    // reorganiza os poteiros
    aux->prev->next = aux->next;
    aux->next->prev = aux->prev;
    
    //desacopla o elemento sem destrui-lo
    aux->next = NULL;
    aux->prev = NULL;
    return 0;   
}

