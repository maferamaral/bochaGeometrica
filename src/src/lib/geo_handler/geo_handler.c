#include "geo_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../fila/fila.h"
#include "../pilha/pilha.h"
#include "../manipuladorDeArquivo/manipuladorDeArquivo.h"
#include "../formas/circulo/circulo.h"
#include "../formas/formas.h"
#include "../formas/linha/linha.h"
#include "../formas/retangulo/retangulo.h"
#include "../formas/text_style/text_style.h"
#include "../formas/texto/texto.h"
#include "../utils/utils.h"

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

// Definições das estruturas internas para acesso em geo_aplicar_efeitos_clonagem
struct Circulo
{
  int id;
  double x;
  double y;
  double radius;
  char *border_color;
  char *fill_color;
};

struct Retangulo
{
  int id;
  double x;
  double y;
  double width;
  double height;
  char *border_color;
  char *fill_color;
};

// Declarações privadas
static void executar_comando_circulo(Ground_t *g);
static void execute_rectangle_command(Ground_t *g);
static void execute_line_command(Ground_t *g);
static void execute_text_command(Ground_t *g);
static void execute_text_style_command(Ground_t *g);
static void create_svg_queue(Ground_t *g, const char *path, FileData fd, const char *suf);

// ==========================================
// FUNÇÕES DE CÁLCULO
// ==========================================

double geo_obter_area(void *shape_ptr)
{
  Shape_t *shape = (Shape_t *)shape_ptr;
  if (!shape)
    return 0.0;
  if (shape->type == CIRCLE)
  {
    return 3.14159265359 * pow(circulo_get_raio(shape->data), 2);
  }
  else if (shape->type == RECTANGLE)
  {
    return retangulo_get_largura(shape->data) * retangulo_get_altura(shape->data);
  }
  else if (shape->type == TEXT)
  {
    return 20.0 * text_get_length(shape->data);
  }
  else if (shape->type == LINE)
  {
    Line l = (Line)shape->data;
    return 2.0 * distancia(line_get_x1(l), line_get_y1(l), line_get_x2(l), line_get_y2(l));
  }
  return 0.0;
}

static double max(double a, double b) { return a > b ? a : b; }
static double min(double a, double b) { return a < b ? a : b; }

static int rect_rect_overlap(double x1, double y1, double w1, double h1, double x2, double y2, double w2, double h2)
{
  return (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2);
}
static int circ_circ_overlap(double x1, double y1, double r1, double x2, double y2, double r2)
{
  double distSq = pow(x1 - x2, 2) + pow(y1 - y2, 2);
  double radSum = r1 + r2;
  return distSq <= (radSum * radSum);
}
static int circ_rect_overlap(double cx, double cy, double r, double rx, double ry, double rw, double rh)
{
  double closestX = max(rx, min(cx, rx + rw));
  double closestY = max(ry, min(cy, ry + rh));
  double distanceX = cx - closestX;
  double distanceY = cy - closestY;
  return ((distanceX * distanceX) + (distanceY * distanceY)) <= (r * r);
}

int geo_verificar_sobreposicao(void *shapeA_ptr, void *shapeB_ptr)
{
  Shape_t *A = (Shape_t *)shapeA_ptr;
  Shape_t *B = (Shape_t *)shapeB_ptr;
  if (!A || !B)
    return 0;

  if (A->type == CIRCLE && B->type == CIRCLE)
  {
    return circ_circ_overlap(circulo_get_x(A->data), circulo_get_y(A->data), circulo_get_raio(A->data),
                             circulo_get_x(B->data), circulo_get_y(B->data), circulo_get_raio(B->data));
  }
  else if (A->type == RECTANGLE && B->type == RECTANGLE)
  {
    return rect_rect_overlap(retangulo_get_x(A->data), retangulo_get_y(A->data), retangulo_get_largura(A->data), retangulo_get_altura(A->data),
                             retangulo_get_x(B->data), retangulo_get_y(B->data), retangulo_get_largura(B->data), retangulo_get_altura(B->data));
  }
  else if ((A->type == CIRCLE && B->type == RECTANGLE) || (A->type == RECTANGLE && B->type == CIRCLE))
  {
    Circulo c = (A->type == CIRCLE) ? (Circulo)A->data : (Circulo)B->data;
    Rectangle r = (A->type == RECTANGLE) ? (Retangulo)A->data : (Retangulo)B->data;
    return circ_rect_overlap(circulo_get_x(c), circulo_get_y(c), circulo_get_raio(c),
                             retangulo_get_x(r), retangulo_get_y(r), retangulo_get_largura(r), retangulo_get_altura(r));
  }
  return 0;
}

// ==========================================
// FUNÇÕES DE DESENHO E CLONAGEM
// ==========================================

void geo_escrever_svg_forma(void *shape_ptr, FILE *file)
{
  Shape_t *shape = (Shape_t *)shape_ptr;
  if (!shape || !file)
    return;

  // Adicionada transparência (fill-opacity) para ficar igual ao gabarito
  if (shape->type == CIRCLE)
  {
    Circulo c = (Circulo)shape->data;
    fprintf(file, "<circle cx='%.2f' cy='%.2f' r='%.2f' fill='%s' stroke='%s' stroke-width='1' fill-opacity='0.7'/>\n",
            circulo_get_x(c), circulo_get_y(c), circulo_get_raio(c), circulo_get_cor_preenchimento(c), circulo_get_cor_borda(c));
  }
  else if (shape->type == RECTANGLE)
  {
    Rectangle r = (Retangulo)shape->data;
    fprintf(file, "<rect x='%.2f' y='%.2f' width='%.2f' height='%.2f' fill='%s' stroke='%s' stroke-width='1' fill-opacity='0.7'/>\n",
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

// ... (Funções de parsing execute_geo_commands, etc. mantêm-se iguais às originais) ...
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

// [NOVO] Implementação da função que escreve o texto detalhado
void geo_imprimir_forma_txt(void *shape_ptr, FILE *txt)
{
  if (!shape_ptr || !txt)
    return;
  Shape_t *shape = (Shape_t *)shape_ptr;
  if (shape->type == CIRCLE)
  {
    Circulo c = (Circulo)shape->data;
    fprintf(txt, "Circulo ID:%d (x:%.2f, y:%.2f) r:%.2f corb:%s corp:%s\n",
            circulo_get_id(c), circulo_get_x(c), circulo_get_y(c),
            circulo_get_raio(c), circulo_get_cor_borda(c), circulo_get_cor_preenchimento(c));
  }
  else if (shape->type == RECTANGLE)
  {
    Rectangle r = (Retangulo)shape->data;
    fprintf(txt, "Retangulo ID:%d (x:%.2f, y:%.2f) w:%.2f h:%.2f corb:%s corp:%s\n",
            retangulo_get_id(r), retangulo_get_x(r), retangulo_get_y(r),
            retangulo_get_largura(r), retangulo_get_altura(r),
            retangulo_get_cor_borda(r), retangulo_get_cor_preenchimento(r));
  }
  else if (shape->type == LINE)
  {
    Line l = (Line)shape->data;
    fprintf(txt, "Linha ID:%d (x1:%.2f, y1:%.2f) (x2:%.2f, y2:%.2f) cor:%s\n",
            line_get_id(l), line_get_x1(l), line_get_y1(l),
            line_get_x2(l), line_get_y2(l), line_get_color(l));
  }
  else if (shape->type == TEXT)
  {
    Text t = (Text)shape->data;
    fprintf(txt, "Texto ID:%d (x:%.2f, y:%.2f) ancora:%c conteudo:\"%s\"\n",
            text_get_id(t), text_get_x(t), text_get_y(t),
            text_get_anchor(t), text_get_text(t));
  }
}

// [NOVO] Implementação da troca de cores (hack acessando structs internas)
// Necessário para o efeito de "mistura" na clonagem
void geo_aplicar_efeitos_clonagem(void *I_ptr, void *J_ptr, void *I_clone_ptr)
{
  if (!I_ptr || !J_ptr || !I_clone_ptr)
    return;

  Shape_t *I = (Shape_t *)I_ptr;
  Shape_t *J = (Shape_t *)J_ptr;
  Shape_t *Ic = (Shape_t *)I_clone_ptr;

  // Precisamos acessar as structs internas (definidas acima neste arquivo)
  char *fill_I = NULL;

  if (I->type == RECTANGLE)
    fill_I = ((struct Retangulo *)I->data)->fill_color;
  else if (I->type == CIRCLE)
    fill_I = ((struct Circulo *)I->data)->fill_color;

  // 1. I muda a cor da borda de J para ser a cor de preenchimento de I
  if (fill_I)
  {
    if (J->type == RECTANGLE)
    {
      struct Retangulo *rJ = (struct Retangulo *)J->data;
      if (rJ->border_color)
        free(rJ->border_color);
      rJ->border_color = duplicate_string(fill_I);
    }
    else if (J->type == CIRCLE)
    {
      struct Circulo *cJ = (struct Circulo *)J->data;
      if (cJ->border_color)
        free(cJ->border_color);
      cJ->border_color = duplicate_string(fill_I);
    }
  }

  // 2. Clone de I inverte borda e preenchimento
  if (Ic->type == RECTANGLE)
  {
    struct Retangulo *rIc = (struct Retangulo *)Ic->data;
    char *temp = rIc->fill_color;
    rIc->fill_color = rIc->border_color;
    rIc->border_color = temp;
  }
  else if (Ic->type == CIRCLE)
  {
    struct Circulo *cIc = (struct Circulo *)Ic->data;
    char *temp = cIc->fill_color;
    cIc->fill_color = cIc->border_color;
    cIc->border_color = temp;
  }
}