#include "fila.h"
#include <stdlib.h>

typedef struct node_t {
  void *data;
  struct node_t *next;
} Node;

struct queue_t {
  Node *front; // Início da fila
  Node *rear;  // Fim da fila
};

static struct queue_t *as_impl(Queue q) { return (struct queue_t *)q; }

Queue queue_create() {
  struct queue_t *impl = (struct queue_t *)malloc(sizeof(struct queue_t));
  if (impl == NULL) {
    return NULL;
  }
  impl->front = NULL;
  impl->rear = NULL;
  return (Queue)impl;
}

void queue_enqueue(Queue q, void *value) {
  struct queue_t *impl = as_impl(q);
  if (impl == NULL) {
    return;
  }
  Node *temp = (Node *)malloc(sizeof(Node));
  if (temp == NULL) {
    return;
  }
  temp->data = value;
  temp->next = NULL;

  if (impl->rear == NULL) {
    impl->front = temp;
    impl->rear = temp;
    return;
  }

  impl->rear->next = temp;
  impl->rear = temp;
}

void *queue_dequeue(Queue q) {
  struct queue_t *impl = as_impl(q);
  if (impl == NULL || impl->front == NULL) {
    return NULL;
  }

  Node *temp = impl->front;
  void *value = temp->data;
  impl->front = temp->next;

  if (impl->front == NULL) {
    impl->rear = NULL;
  }

  free(temp);
  return value;
}

int queue_is_empty(Queue q) {
  struct queue_t *impl = as_impl(q);
  if (impl == NULL) {
    return 1;
  }
  return impl->front == NULL;
}

void queue_destroy(Queue q) {
  struct queue_t *impl = as_impl(q);
  if (impl == NULL) {
    return;
  }
  Node *curr = impl->front;
  while (curr != NULL) {
    Node *next = curr->next;
    free(curr);
    curr = next;
  }
  free(impl);
}
