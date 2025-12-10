#include "geo_handler.h"
#include "../fila/fila.h"
#include "../formas/circulo/circulo.h"
#include "../formas/formas.h"
#include "../formas/linha/linha.h"
#include "../formas/retangulo/retangulo.h"
#include "../formas/text_style/text_style.h"
#include "../formas/texto/texto.h"
#include "../manipuladorDeArquivo/manipuladorDeArquivo.h"
#include "../pilha/pilha.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct
{
  Fila shapesQueue;
  Stack shapesStackToFree;
  Queue svgQueue;
} Ground_t;

typedef struct
{
  TipoForma type;
  void *data;
} Shape_t;

static void executar_comando_circulo(Ground_t *ground);
static void execute_rectangle_command(Ground_t *ground);
static void execute_line_command(Ground_t *ground);
static void execute_text_command(Ground_t *ground);
static void execute_text_style_command(Ground_t *ground);
static void create_svg_queue(Ground_t *ground, const char *output_path, FileData fileData, const char *command_suffix);

Ground execute_geo_commands(FileData fileData, const char *output_path, const char *command_suffix)
{
  Ground_t *ground = malloc(sizeof(Ground_t));
  if (!ground)
    exit(1);
  ground->shapesQueue = queue_create();
  ground->shapesStackToFree = stack_create();
  ground->svgQueue = queue_create();

  Queue lines = getLinesQueue(fileData);
  if (!lines)
  {
    free(ground);
    return NULL;
  }

  while (!queue_is_empty(lines))
  {
    char *line = (char *)queue_dequeue(lines);
    char *command = strtok(line, " ");
    if (!command)
      continue;

    if (strcmp(command, "c") == 0)
      executar_comando_circulo(ground);
    else if (strcmp(command, "r") == 0)
      execute_rectangle_command(ground);
    else if (strcmp(command, "l") == 0)
      execute_line_command(ground);
    else if (strcmp(command, "t") == 0)
      execute_text_command(ground);
    else if (strcmp(command, "ts") == 0)
      execute_text_style_command(ground);
  }
  create_svg_queue(ground, output_path, fileData, command_suffix);
  return ground;
}

void destroy_geo_waste(Ground ground)
{
  if (!ground)
    return;
  Ground_t *gt = (Ground_t *)ground;
  queue_destroy(gt->shapesQueue);
  queue_destroy(gt->svgQueue);
  while (!stack_is_empty(gt->shapesStackToFree))
    free(stack_pop(gt->shapesStackToFree));
  stack_destroy(gt->shapesStackToFree);
  free(ground);
}

Queue get_ground_queue(Ground ground) { return ground ? ((Ground_t *)ground)->shapesQueue : NULL; }
Stack get_ground_shapes_stack_to_free(Ground ground) { return ground ? ((Ground_t *)ground)->shapesStackToFree : NULL; }

// --- IMPLEMENTAÇÃO DAS NOVAS FUNÇÕES ---

void geo_escrever_svg_forma(void *shape_ptr, FILE *file)
{
  Shape_t *shape = (Shape_t *)shape_ptr;
  if (!shape || !file)
    return;

  if (shape->type == CIRCLE)
  {
    Circulo c = (Circulo)shape->data;
    fprintf(file, "<circle cx='%.2f' cy='%.2f' r='%.2f' fill='%s' stroke='%s' stroke-width='1'/>\n",
            circulo_get_x(c), circulo_get_y(c), circulo_get_raio(c), circulo_get_cor_preenchimento(c), circulo_get_cor_borda(c));
  }
  else if (shape->type == RECTANGLE)
  {
    Rectangle r = (Retangulo)shape->data;
    fprintf(file, "<rect x='%.2f' y='%.2f' width='%.2f' height='%.2f' fill='%s' stroke='%s' stroke-width='1'/>\n",
            retangulo_get_x(r), retangulo_get_y(r), retangulo_get_largura(r), retangulo_get_altura(r),
            retangulo_get_cor_preenchimento(r), retangulo_get_cor_borda(r));
  }
  else if (shape->type == LINE)
  {
    Line l = (Line)shape->data;
    fprintf(file, "<line x1='%.2f' y1='%.2f' x2='%.2f' y2='%.2f' stroke='%s' stroke-width='1'/>\n",
            line_get_x1(l), line_get_y1(l), line_get_x2(l), line_get_y2(l), line_get_color(l));
  }
  else if (shape->type == TEXT)
  {
    Text t = (Text)shape->data;
    char a = text_get_anchor(t);
    const char *anchor = (a == 'm') ? "middle" : (a == 'e' || a == 'f') ? "end"
                                                                        : "start";
    fprintf(file, "<text x='%.2f' y='%.2f' fill='%s' stroke='%s' text-anchor='%s'>%s</text>\n",
            text_get_x(t), text_get_y(t), text_get_fill_color(t), text_get_border_color(t), anchor, text_get_text(t));
  }
}

void *geo_clonar_forma(void *shape_ptr, double x, double y, Ground ground)
{
  Shape_t *src = (Shape_t *)shape_ptr;
  if (!src)
    return NULL;

  void *newData = NULL;
  if (src->type == CIRCLE)
  {
    Circulo c = (Circulo)src->data;
    newData = circulo_criar(circulo_get_id(c), x, y, circulo_get_raio(c), circulo_get_cor_borda(c), circulo_get_cor_preenchimento(c));
  }
  else if (src->type == RECTANGLE)
  {
    Rectangle r = (Rectangle)src->data;
    newData = retangulo_criar(retangulo_get_id(r), x, y, retangulo_get_largura(r), retangulo_get_altura(r), retangulo_get_cor_borda(r), retangulo_get_cor_preenchimento(r));
  }
  else if (src->type == LINE)
  {
    Line l = (Line)src->data;
    double dx = x - line_get_x1(l), dy = y - line_get_y1(l);
    newData = line_create(line_get_id(l), x, y, line_get_x2(l) + dx, line_get_y2(l) + dy, line_get_color(l));
  }
  else if (src->type == TEXT)
  {
    Text t = (Text)src->data;
    newData = text_create(text_get_id(t), x, y, text_get_border_color(t), text_get_fill_color(t), text_get_anchor(t), text_get_text(t));
  }

  if (newData)
  {
    Shape_t *newShape = malloc(sizeof(Shape_t));
    newShape->type = src->type;
    newShape->data = newData;
    if (ground)
    {
      Stack s = get_ground_shapes_stack_to_free(ground);
      if (s)
        stack_push(s, newShape);
    }
    return newShape;
  }
  return NULL;
}

// Funções de parsing (Simplificadas para poupar espaço)
static void executar_comando_circulo(Ground_t *g)
{
  char *id = strtok(NULL, " "), *x = strtok(NULL, " "), *y = strtok(NULL, " "), *r = strtok(NULL, " "), *cb = strtok(NULL, " "), *cp = strtok(NULL, " ");
  Shape_t *s = malloc(sizeof(Shape_t));
  s->type = CIRCLE;
  s->data = circulo_criar(atoi(id), atof(x), atof(y), atof(r), cb, cp);
  queue_enqueue(g->shapesQueue, s);
  stack_push(g->shapesStackToFree, s);
  queue_enqueue(g->svgQueue, s);
}
static void execute_rectangle_command(Ground_t *g)
{
  char *id = strtok(NULL, " "), *x = strtok(NULL, " "), *y = strtok(NULL, " "), *w = strtok(NULL, " "), *h = strtok(NULL, " "), *cb = strtok(NULL, " "), *cp = strtok(NULL, " ");
  Shape_t *s = malloc(sizeof(Shape_t));
  s->type = RECTANGLE;
  s->data = retangulo_criar(atoi(id), atof(x), atof(y), atof(w), atof(h), cb, cp);
  queue_enqueue(g->shapesQueue, s);
  stack_push(g->shapesStackToFree, s);
  queue_enqueue(g->svgQueue, s);
}
static void execute_line_command(Ground_t *g)
{
  char *id = strtok(NULL, " "), *x1 = strtok(NULL, " "), *y1 = strtok(NULL, " "), *x2 = strtok(NULL, " "), *y2 = strtok(NULL, " "), *c = strtok(NULL, " ");
  Shape_t *s = malloc(sizeof(Shape_t));
  s->type = LINE;
  s->data = line_create(atoi(id), atof(x1), atof(y1), atof(x2), atof(y2), c);
  queue_enqueue(g->shapesQueue, s);
  stack_push(g->shapesStackToFree, s);
  queue_enqueue(g->svgQueue, s);
}
static void execute_text_command(Ground_t *g)
{
  char *id = strtok(NULL, " "), *x = strtok(NULL, " "), *y = strtok(NULL, " "), *cb = strtok(NULL, " "), *cp = strtok(NULL, " "), *a = strtok(NULL, " "), *t = strtok(NULL, "");
  Shape_t *s = malloc(sizeof(Shape_t));
  s->type = TEXT;
  s->data = text_create(atoi(id), atof(x), atof(y), cb, cp, *a, t);
  queue_enqueue(g->shapesQueue, s);
  stack_push(g->shapesStackToFree, s);
  queue_enqueue(g->svgQueue, s);
}
static void execute_text_style_command(Ground_t *g)
{
  char *f = strtok(NULL, " "), *w = strtok(NULL, " "), *sz = strtok(NULL, " ");
  Shape_t *s = malloc(sizeof(Shape_t));
  s->type = TEXT_STYLE;
  s->data = text_style_create(f, *w, atoi(sz));
  queue_enqueue(g->shapesQueue, s);
  stack_push(g->shapesStackToFree, s);
  queue_enqueue(g->svgQueue, s);
}
static void create_svg_queue(Ground_t *g, const char *path, FileData fd, const char *suf)
{
  char *fname = malloc(strlen(getFileName(fd)) + 50);
  strcpy(fname, getFileName(fd));
  if (suf)
  {
    strcat(fname, "-");
    strcat(fname, suf);
  }
  char *out = malloc(strlen(path) + strlen(fname) + 10);
  sprintf(out, "%s/%s.svg", path, fname);
  FILE *f = fopen(out, "w");
  if (f)
  {
    fprintf(f, "<?xml version=\"1.0\"?>\n<svg xmlns=\"http://www.w3.org/2000/svg\">\n");
    while (!queue_is_empty(g->svgQueue))
    {
      Shape_t *s = queue_dequeue(g->svgQueue);
      if (s)
        geo_escrever_svg_forma(s, f);
    }
    fprintf(f, "</svg>");
    fclose(f);
  }
  free(fname);
  free(out);
}