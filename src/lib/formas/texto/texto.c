#include "texto.h"
#include "../../utils/utils.h"
#include <stdlib.h>
#include <string.h>

/**
 * Internal Text structure
 */
struct Text {
  int id;
  double x;
  double y;
  char *border_color;
  char *fill_color;
  char anchor;
  char *text;
};

void *text_create(int id, double x, double y, const char *border_color,
                  const char *fill_color, char anchor, const char *text) {
  if (!border_color || !fill_color || !text) {
    return NULL;
  }

  struct Text *t = malloc(sizeof(struct Text));
  if (!t) {
    return NULL;
  }

  t->id = id;
  t->x = x;
  t->y = y;
  t->anchor = anchor;

  t->border_color = duplicate_string(border_color);
  if (!t->border_color) {
    free(t);
    return NULL;
  }

  t->fill_color = duplicate_string(fill_color);
  if (!t->fill_color) {
    free(t->border_color);
    free(t);
    return NULL;
  }

  t->text = duplicate_string(text);
  if (!t->text) {
    free(t->border_color);
    free(t->fill_color);
    free(t);
    return NULL;
  }

  return t;
}

void text_destroy(void *text) {
  if (!text)
    return;

  struct Text *t = (struct Text *)text;
  free(t->border_color);
  free(t->fill_color);
  free(t->text);
  free(t);
}

int text_get_id(void *text) {
  if (!text)
    return -1;
  return ((struct Text *)text)->id;
}

double text_get_x(void *text) {
  if (!text)
    return 0.0;
  return ((struct Text *)text)->x;
}

double text_get_y(void *text) {
  if (!text)
    return 0.0;
  return ((struct Text *)text)->y;
}

const char *text_get_border_color(void *text) {
  if (!text)
    return NULL;
  return ((struct Text *)text)->border_color;
}

const char *text_get_fill_color(void *text) {
  if (!text)
    return NULL;
  return ((struct Text *)text)->fill_color;
}

char text_get_anchor(void *text) {
  if (!text)
    return '\0';
  return ((struct Text *)text)->anchor;
}

const char *text_get_text(void *text) {
  if (!text)
    return NULL;
  return ((struct Text *)text)->text;
}