#include "retangulo.h"
#include "../../utils/utils.h"
#include <stdlib.h>
#include <string.h>

/**
 * Internal Rectangle structure
 */
struct Retangulo {
  int id;
  double x;
  double y;
  double width;
  double height;
  char *border_color;
  char *fill_color;
};

void *retangulo_criar(int id, double x, double y, double largura, double altura,
                      const char *cor_borda, const char *cor_preenchimento) {
  if (!cor_borda || !cor_preenchimento) {
    return NULL;
  }

  struct Retangulo *retangulo = malloc(sizeof(struct Retangulo));
  if (!retangulo) {
    return NULL;
  }

  retangulo->id = id;
  retangulo->x = x;
  retangulo->y = y;
  retangulo->width = largura;
  retangulo->height = altura;

  retangulo->border_color = duplicate_string(cor_borda);
  if (!retangulo->border_color) {
    free(retangulo);
    return NULL;
  }

  retangulo->fill_color = duplicate_string(cor_preenchimento);
  if (!retangulo->fill_color) {
    free(retangulo->border_color);
    free(retangulo);
    return NULL;
  }

  return retangulo;
}

void retangulo_destruir(void *retangulo) {
  if (!retangulo)
    return;

  struct Retangulo *r = (struct Retangulo *)retangulo;
  free(r->border_color);
  free(r->fill_color);
  free(r);
}

int retangulo_get_id(void *retangulo) {
  if (!retangulo)
    return -1;
  return ((struct Retangulo *)retangulo)->id;
}

double retangulo_get_x(void *retangulo) {
  if (!retangulo)
    return 0.0;
  return ((struct Retangulo *)retangulo)->x;
}

double retangulo_get_y(void *retangulo) {
  if (!retangulo)
    return 0.0;
  return ((struct Retangulo *)retangulo)->y;
}

double retangulo_get_largura(void *retangulo) {
  if (!retangulo)
    return 0.0;
  return ((struct Retangulo *)retangulo)->width;
}

double retangulo_get_altura(void *retangulo) {
  if (!retangulo)
    return 0.0;
  return ((struct Retangulo *)retangulo)->height;
}

const char *retangulo_get_cor_borda(void *retangulo) {
  if (!retangulo)
    return NULL;
  return ((struct Retangulo *)retangulo)->border_color;
}

const char *retangulo_get_cor_preenchimento(void *retangulo) {
  if (!retangulo)
    return NULL;
  return ((struct Retangulo *)retangulo)->fill_color;
}