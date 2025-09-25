#include <stdio.h>
#include <stdlib.h>
#include "pilha.h"

// Estrutura do nó da pilha
typedef struct Node {
    int data;
    struct Node* next;
} Node;

// Função para criar um novo nó
Node* createNode(int data) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (!newNode) {
        printf("Erro de alocação de memória!\n");
        exit(1);
    }
    newNode->data = data;
    newNode->next = NULL;
    return newNode;
}

// Função para empilhar (push)
void push(Node** top, int data) {
    Node* newNode = createNode(data);
    newNode->next = *top;
    *top = newNode;
    printf("Elemento %d empilhado.\n", data);
}

// Função para desempilhar (pop)
int pop(Node** top) {
    if (*top == NULL) {
        printf("A pilha está vazia! Não é possível desempilhar.\n");
        return -1;
    }
    Node* temp = *top;
    int poppedData = temp->data;
    *top = (*top)->next;
    free(temp);
    return poppedData;
}

// Função para verificar o elemento no topo (peek)
int peek(Node* top) {
    if (top == NULL) {
        printf("A pilha está vazia!\n");
        return -1;
    }
    return top->data;
}

// Função para verificar se a pilha está vazia
int isEmpty(Node* top) {
    return top == NULL;
}

