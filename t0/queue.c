// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.4 -- Janeiro de 2022
// Definição e operações em uma fila genérica.

// ESTE ARQUIVO NÃO DEVE SER MODIFICADO - ELE SERÁ SOBRESCRITO NOS TESTES

#include "queue.h"

//------------------------------------------------------------------------------
// Conta o numero de elementos na fila
// Retorno: numero de elementos na fila

int queue_size (queue_t *queue){
   int tam = 0;
   if (queue == NULL)
      return 0;
   tam = 1;
   queue_t *original = queue;
   queue = queue->next;
   while (queue != original){
      tam++;
      queue = queue->next;
   }
   return tam;
}   



//------------------------------------------------------------------------------
// Percorre a fila e imprime na tela seu conteúdo. A impressão de cada
// elemento é feita por uma função externa, definida pelo programa que
// usa a biblioteca. Essa função deve ter o seguinte protótipo:
//
// void print_elem (void *ptr) ; // ptr aponta para o elemento a imprimir

void queue_print (char *name, queue_t *queue, void print_elem (void*) ) {

}

//------------------------------------------------------------------------------
// Insere um elemento no final da fila.
// Condicoes a verificar, gerando msgs de erro:
// - a fila deve existir
// - o elemento deve existir
// - o elemento nao deve estar em outra fila
// Retorno: 0 se sucesso, <0 se ocorreu algum erro

int queue_append (queue_t **queue, queue_t *elem){
   
   if (elem == NULL)
      return -2;
   if (elem->next != NULL || elem->prev != NULL)
      return -3;
   if(queue_size(*queue) == 0){
      *queue = elem;
      elem->next = elem;
      elem->prev =elem;
      return 0;
   }
   queue_t aux = *queue->prev;

   aux->next = elem;
   elem->prev = aux;
   *queue->prev = elem;
   elem->prox = *queue;
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

int queue_remove (queue_t **queue, queue_t *elem) ;

#endif

