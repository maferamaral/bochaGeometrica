#include "pilha.h"
#include <stdlib.h>

typedef struct stack_node_t
{
  void *data;
  struct stack_node_t *next;
} StackNode;

typedef struct stack_t
{
  StackNode *top;
} *StackImpl;

static StackImpl as_impl(Stack stack) { return (StackImpl)stack; }

Stack stack_create()
{
  StackImpl s = (StackImpl)malloc(sizeof(*s));
  if (s == NULL)
  {
    return NULL;
  }
  s->top = NULL;
  return (Stack)s;
}

int stack_push(Stack stack, void *value)
{
  StackImpl s = as_impl(stack);
  if (s == NULL)
  {
    return 0;
  }
  StackNode *node = (StackNode *)malloc(sizeof(StackNode));
  if (node == NULL)
  {
    return 0;
  }
  node->data = value;
  node->next = s->top;
  s->top = node;
  return 1;
}

void *stack_pop(Stack stack)
{
  StackImpl s = as_impl(stack);
  if (s == NULL || s->top == NULL)
  {
    return NULL;
  }
  StackNode *node = s->top;
  void *value = node->data;
  s->top = node->next;
  free(node);
  return value;
}

int stack_is_empty(Stack stack)
{
  StackImpl s = as_impl(stack);
  if (s == NULL)
  {
    return 1;
  }
  return s->top == NULL;
}

int stack_size(Stack stack)
{
  StackImpl s = as_impl(stack);
  if (s == NULL)
  {
    return 0;
  }
  int count = 0;
  StackNode *curr = s->top;
  while (curr != NULL)
  {
    count++;
    curr = curr->next;
  }
  return count;
}

void *stack_peek_at(Stack stack, int index)
{
  StackImpl s = as_impl(stack);
  if (s == NULL || s->top == NULL)
  {
    return NULL;
  }
  StackNode *curr = s->top;
  for (int i = 0; i < index && curr != NULL; i++)
  {
    curr = curr->next;
  }
  return curr != NULL ? curr->data : NULL;
}

void stack_destroy(Stack stack)
{
  StackImpl s = as_impl(stack);
  if (s == NULL)
  {
    return;
  }
  StackNode *curr = s->top;
  while (curr != NULL)
  {
    StackNode *next = curr->next;
    free(curr);
    curr = next;
  }
  free(s);
}
