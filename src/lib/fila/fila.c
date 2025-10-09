#include <stdio.h>
#include <stdlib.h>
#include "fila.h"

// Estrutura para um nó da fila
typedef struct Node
{
    void *data;
    struct Node *next;
} NodeImpl;

// Estrutura para a fila que será tratada como void*
typedef struct QueueImpl
{
    NodeImpl *front; // Início da fila
    NodeImpl *rear;  // Fim da fila
} QueueImpl;

// Função para criar uma fila vazia
void *createQueue()
{
    QueueImpl *q = (QueueImpl *)malloc(sizeof(QueueImpl));
    q->front = q->rear = NULL;
    return (void *)q;
}

// Função para adicionar um elemento à fila
void enqueue(void *queue, void *value)
{
    QueueImpl *q = (QueueImpl *)queue;
    NodeImpl *temp = (NodeImpl *)malloc(sizeof(NodeImpl));
    temp->data = value;
    temp->next = NULL;

    if (q->rear == NULL)
    { // Caso a fila esteja vazia
        q->front = q->rear = temp;
        return;
    }

    q->rear->next = temp;
    q->rear = temp;
}

// Função para remover um elemento da fila
void *dequeue(void *queue)
{
    QueueImpl *q = (QueueImpl *)queue;
    if (q->front == NULL)
    { // Caso a fila esteja vazia
        printf("Fila vazia! Não é possível remover elementos.\n");
        return NULL;
    }

    NodeImpl *temp = q->front;
    void *value = temp->data;
    q->front = q->front->next;

    if (q->front == NULL)
    { // Caso a fila fique vazia após a remoção
        q->rear = NULL;
    }

    free(temp);
    return value;
}

// Função para exibir os elementos da fila
void displayQueue(void *queue)
{
    QueueImpl *q = (QueueImpl *)queue;
    if (q->front == NULL)
    {
        printf("Fila vazia!\n");
        return;
    }

    NodeImpl *temp = q->front;
    printf("Fila: ");
    while (temp != NULL)
    {
        printf("%p ", temp->data); // Exibe o ponteiro
        temp = temp->next;
    }
    printf("\n");
}
