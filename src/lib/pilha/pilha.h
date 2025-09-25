#ifndef PILHA_H
#define PILHA_H

// Estrutura do nó da pilha
typedef struct Node Node;

// Função para criar um novo nó
Node* createNode(int data);

// Função para empilhar (push)
void push(Node** top, int data);

// Função para desempilhar (pop)
int pop(Node** top);

// Função para verificar o elemento no topo (peek)
int peek(Node* top);

// Função para verificar se a pilha está vazia
int isEmpty(Node* top);

#endif // PILHA_H
