#ifndef FILA_H
#define FILA_H

// Definição do tipo Node com void* data
typedef void *Node;

typedef void *Queue;
// Função para criar uma fila vazia
Queue createQueue();
// Função para adicionar um elemento à fila
void enqueue(Queue q, void *value);
// Função para remover um elemento da fila
void *dequeue(Queue q);
// Função para exibir os elementos da fila
void displayQueue(Queue q);

#endif // FILA_H
