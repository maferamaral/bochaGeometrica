#ifndef FILA_H
#define FILA_H

#include <stdio.h>
#include <stdlib.h>


typedef struct Queue Queue;
// Função para criar uma fila vazia

Queue* createQueue();

// Função para adicionar um elemento à fila
void enqueue(Queue* q, int value);

// Função para remover um elemento da fila
int dequeue(Queue* q);

// Função para exibir os elementos da fila
void displayQueue(Queue* q);

#endif // FILA_H
