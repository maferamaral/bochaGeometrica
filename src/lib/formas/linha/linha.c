#include "linha.h"
#include "../../utils/utils.h"
#include <stdlib.h>
#include <string.h>

/**
 * Internal Line structure
 */
struct Line {
  int id;
  double x1;
  double y1;
  double x2;
  double y2;
  char *color;
};

void *line_create(int id, double x1, double y1, double x2, double y2,
                  const char *color) {
  if (!color) {
    return NULL;
  }

  struct Line *line = malloc(sizeof(struct Line));
  if (!line) {
    return NULL;
  }

  line->id = id;
  line->x1 = x1;
  line->y1 = y1;
  line->x2 = x2;
  line->y2 = y2;

  line->color = duplicate_string(color);
  if (!line->color) {
    free(line);
    return NULL;
  }

  return line;
}

void line_destroy(void *line) {
  if (!line)
    return;

  struct Line *l = (struct Line *)line;
  free(l->color);
  free(l);
}

int line_get_id(void *line) {
  if (!line)
    return -1;
  return ((struct Line *)line)->id;
}

double line_get_x1(void *line) {
  if (!line)
    return 0.0;
  return ((struct Line *)line)->x1;
}

double line_get_y1(void *line) {
  if (!line)
    return 0.0;
  return ((struct Line *)line)->y1;
}

double line_get_x2(void *line) {
  if (!line)
    return 0.0;
  return ((struct Line *)line)->x2;
}

double line_get_y2(void *line) {
  if (!line)
    return 0.0;
  return ((struct Line *)line)->y2;
}

const char *line_get_color(void *line) {
  if (!line)
    return NULL;
  return ((struct Line *)line)->color;
}