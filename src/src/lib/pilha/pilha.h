#ifndef PILHA_H
#define PILHA_H

// Tipo opaco da pilha
typedef void *Stack;

// Cria uma pilha vazia
Stack stack_create();
// Empilha um elemento (ponteiro genérico)
int stack_push(Stack stack, void *value);
// Desempilha e retorna o elemento; retorna NULL se vazia
void *stack_pop(Stack stack);
// Retorna 1 se vazia, 0 caso contrário
int stack_is_empty(Stack stack);
// Retorna o tamanho da pilha
int stack_size(Stack stack);
// Retorna o elemento no índice especificado (0 = topo)
void *stack_peek_at(Stack stack, int index);
// Destroi a pilha liberando nós internos (não libera os dados apontados)
void stack_destroy(Stack stack);

#endif // PILHA_H
