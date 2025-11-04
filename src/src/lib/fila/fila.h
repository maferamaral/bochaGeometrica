#ifndef FILA_H
#define FILA_H

#include <stdio.h>
#include <stdlib.h>

// Tipo opaco da fila
typedef void *Queue;
// Alias para compatibilidade com código existente
typedef Queue Fila;

// Cria uma fila vazia
Queue queue_create();
// Enfileira um elemento (ponteiro genérico)
void queue_enqueue(Queue queue, void *value);
// Desenfileira e retorna o elemento; retorna NULL se vazia
void *queue_dequeue(Queue queue);
// Retorna 1 se vazia, 0 caso contrário
int queue_is_empty(Queue queue);
// Destroi a fila liberando nós internos (não libera os dados apontados)
void queue_destroy(Queue queue);
// Retorna o tamanho da fila
int queue_size(Queue queue);
#endif // FILA_H
