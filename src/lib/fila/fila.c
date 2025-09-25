#include <stdio.h>
#include <stdlib.h>
#include "fila.h"

// Estrutura para um nó da fila
typedef struct Node {
    void* data;
    struct Node* next;
} Node;

// Estrutura para a fila
typedef struct Queue {
    Node* front; // Início da fila
    Node* rear;  // Fim da fila
} Queue;

// Função para criar uma fila vazia
Queue* createQueue() {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    q->front = q->rear = NULL;
    return q;
}

// Função para adicionar um elemento à fila
void enqueue(Queue* q, void* value) {
    Node* temp = (Node*)malloc(sizeof(Node));
    temp->data = value;
    temp->next = NULL;

    if (q->rear == NULL) { // Caso a fila esteja vazia
        q->front = q->rear = temp;
        return;
    }

    q->rear->next = temp;
    q->rear = temp;
}

// Função para remover um elemento da fila
void* dequeue(Queue* q) {
    if (q->front == NULL) { // Caso a fila esteja vazia
        printf("Fila vazia! Não é possível remover elementos.\n");
        return NULL;
    }

    Node* temp = q->front;
    void* value = temp->data;
    q->front = q->front->next;

    if (q->front == NULL) { // Caso a fila fique vazia após a remoção
        q->rear = NULL;
    }

    free(temp);
    return value; // ou apenas 'return value;' se for void*
}

// Função para exibir os elementos da fila
void displayQueue(Queue* q) {
    if (q->front == NULL) {
        printf("Fila vazia!\n");
        return;
    }

    Node* temp = q->front;
    printf("Fila: ");
    while (temp != NULL) {
        printf("%p ", temp->data); // Exibe o ponteiro
        temp = temp->next;
    }
    printf("\n");
}

