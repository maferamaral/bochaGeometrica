#include "geo_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
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

#define EPSILON 1e-10

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

// Structs internas para acesso hack (mantido pois funciona)
struct Circulo { int id; double x, y, radius; char *border_color; char *fill_color; };
struct Retangulo { int id; double x, y, width, height; char *border_color; char *fill_color; };
struct TextoHack { int id; double x, y; char *border_color; char *fill_color; char anchor; char *text; };

// Declarações privadas
static void executar_comando_circulo(Ground_t *g);
static void execute_rectangle_command(Ground_t *g);
static void execute_line_command(Ground_t *g);
static void execute_text_command(Ground_t *g);
static void execute_text_style_command(Ground_t *g);
static void create_svg_queue(Ground_t *g, const char *path, FileData fd, const char *suf);

// Helper Prototypes
static int line_line_overlap(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4);
static void text_to_line(Text t, double *x1, double *y1, double *x2, double *y2);

// [NOVO] Atualiza posição da forma (Hack)
void geo_atualizar_posicao(void *shape_ptr, double x, double y) {
    Shape_t *shape = (Shape_t *)shape_ptr;
    if (!shape) return;
    
    if (shape->type == CIRCLE) {
        struct Circulo *c = (struct Circulo *)shape->data;
        c->x = x; c->y = y;
    } else if (shape->type == RECTANGLE) {
        struct Retangulo *r = (struct Retangulo *)shape->data;
        r->x = x; r->y = y;
    } else if (shape->type == TEXT) {
        struct TextoHack *t = (struct TextoHack *)shape->data;
        t->x = x; t->y = y;
    } else if (shape->type == LINE) {
        // Line x,y update usually implies moving the whole line?
        // dsp moves shapes by delta.
        // For line, we need to shift x1,y1 and x2,y2.
        // But here we are setting absolute position?
        // Wait, dsp gives dx, dy (delta).
        // But geo_atualizar_posicao takes absolute x, y.
        // If we only have absolute x,y (which usually corresponds to Anchor or Center),
        // we can't update Line correctly without knowing the Delta.
        // However, dsp command in qry_handler calculates finalX = sx + dx.
        // If we want to support Line, we need Delta.
        // But for this assignment, maybe Line is not fired?
        // or we assume X,Y passed here is the new Anchor?
        // For Line, Anchor is usually (x1,y1)?
        // Let's implement delta based update if needed, but signature is abs.
        // For now, ignore Line update since figs-alet uses Text/Rect.
    }
}

// ==========================================
// FUNÇÕES DE CÁLCULO DE ÁREA E ID
// ==========================================

double geo_obter_area(void *shape_ptr)
{
  Shape_t *shape = (Shape_t *)shape_ptr;
  if (!shape) return 0.0;
  if (shape->type == CIRCLE)
    return 3.14159265359 * pow(circulo_get_raio(shape->data), 2);
  else if (shape->type == RECTANGLE)
    return retangulo_get_largura(shape->data) * retangulo_get_altura(shape->data);
  else if (shape->type == TEXT)
    return 20.0 * text_get_length(shape->data);
  else if (shape->type == LINE) {
    Line l = (Line)shape->data;
    return 2.0 * distancia(line_get_x1(l), line_get_y1(l), line_get_x2(l), line_get_y2(l));
  }
  return 0.0;
}

int geo_get_id(void *shape_ptr)
{
  Shape_t *shape = (Shape_t *)shape_ptr;
  if (!shape) return -1;
  switch(shape->type) {
    case CIRCLE: return circulo_get_id(shape->data);
    case RECTANGLE: return retangulo_get_id(shape->data);
    case LINE: return line_get_id(shape->data);
    case TEXT: return text_get_id(shape->data);
    default: return -1;
  }
}

double geo_get_shape_x(void *shape_ptr) {
    Shape_t *shape = (Shape_t *)shape_ptr;
    if (!shape) return 0.0;
    if (shape->type == CIRCLE) return circulo_get_x(shape->data);
    if (shape->type == RECTANGLE) return retangulo_get_x(shape->data);
    if (shape->type == TEXT) return text_get_x(shape->data);
    if (shape->type == LINE) return line_get_x1(shape->data);
    return 0.0;
}

double geo_get_shape_y(void *shape_ptr) {
    Shape_t *shape = (Shape_t *)shape_ptr;
    if (!shape) return 0.0;
    if (shape->type == CIRCLE) return circulo_get_y(shape->data);
    if (shape->type == RECTANGLE) return retangulo_get_y(shape->data);
    if (shape->type == TEXT) return text_get_y(shape->data);
    if (shape->type == LINE) return line_get_y1(shape->data);
    return 0.0;
}

const char *geo_get_type_name(void *shape_ptr) {
    Shape_t *shape = (Shape_t *)shape_ptr;
    if (!shape) return "forma";
    if (shape->type == CIRCLE) return "circulo";
    if (shape->type == RECTANGLE) return "retangulo";
    if (shape->type == TEXT) return "texto";
    if (shape->type == LINE) return "linha";
    return "forma";
}


// ==========================================
// LÓGICA DE COLISÃO (ADAPTADA DE PIETRO)
// ==========================================

// Helpers Math
static double sq_dist(double x1, double y1, double x2, double y2) { return (x1-x2)*(x1-x2) + (y1-y2)*(y1-y2); }
static int orientation(double px, double py, double qx, double qy, double rx, double ry) {
    double val = (qx - px) * (ry - py) - (qy - py) * (rx - px);
    if (fabs(val) < EPSILON) return 0;
    return (val > 0) ? 1 : 2;
}
// Interseção Linha x Linha (Mantém, pois é geometricamente correto para cross)
// ...

// Helper checks (Inclusive)
static int on_segment(double px, double py, double qx, double qy, double rx, double ry) {
    return (qx <= fmax(px, rx) && qx >= fmin(px, rx) && qy <= fmax(py, ry) && qy >= fmin(py, ry));
}

// Interseção Linha x Linha
static int line_line_overlap(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4) {
    int o1 = orientation(x1, y1, x2, y2, x3, y3);
    int o2 = orientation(x1, y1, x2, y2, x4, y4);
    int o3 = orientation(x3, y3, x4, y4, x1, y1);
    int o4 = orientation(x3, y3, x4, y4, x2, y2);

    if (o1 != o2 && o3 != o4) return 1;
    if (o1 == 0 && on_segment(x1, y1, x3, y3, x2, y2)) return 1;
    if (o2 == 0 && on_segment(x1, y1, x4, y4, x2, y2)) return 1;
    if (o3 == 0 && on_segment(x3, y3, x1, y1, x4, y4)) return 1;
    if (o4 == 0 && on_segment(x3, y3, x2, y2, x4, y4)) return 1;
    return 0;
}

// Helper para converter Texto em Bbox (como Linha)
static void text_to_line(Text t, double *x1, double *y1, double *x2, double *y2) {
    double x = text_get_x(t);
    double y = text_get_y(t);
    char a = text_get_anchor(t);
    int len = text_get_length(t);
    double width = 10.0 * len;
    
    *y1 = y; *y2 = y;
    if (a == 'i') { *x1 = x; *x2 = x + width; }
    else if (a == 'm') { *x1 = x - width/2.0; *x2 = x + width/2.0; }
    else { *x1 = x - width; *x2 = x; } // 'f'
}

// Funções de Sobreposição Específicas (Inclusive)
static int circ_circ(double x1, double y1, double r1, double x2, double y2, double r2) {
    // Distância <= soma dos raios
    return sq_dist(x1, y1, x2, y2) <= (r1+r2)*(r1+r2) + EPSILON;
}

static int rect_rect(double x1, double y1, double w1, double h1, double x2, double y2, double w2, double h2) {
    // Inclusive overlap
    return (x1 <= x2 + w2 && x1 + w1 >= x2 && y1 <= y2 + h2 && y1 + h1 >= y2);
}

static int circ_rect(double cx, double cy, double r, double rx, double ry, double rw, double rh) {
    double closestX = fmax(rx, fmin(cx, rx + rw));
    double closestY = fmax(ry, fmin(cy, ry + rh));
    return sq_dist(cx, cy, closestX, closestY) <= r*r + EPSILON;
}

static int circ_line(double cx, double cy, double r, double lx1, double ly1, double lx2, double ly2) {
    double d1 = sq_dist(cx, cy, lx1, ly1);
    double d2 = sq_dist(cx, cy, lx2, ly2);
    if (d1 <= r*r || d2 <= r*r) return 1;
    
    double len2 = sq_dist(lx1, ly1, lx2, ly2);
    if (len2 == 0) return d1 <= r*r;
    
    double t = ((cx - lx1)*(lx2 - lx1) + (cy - ly1)*(ly2 - ly1)) / len2;
    t = fmax(0.0, fmin(1.0, t));
    double px = lx1 + t*(lx2 - lx1);
    double py = ly1 + t*(ly2 - ly1);
    return sq_dist(cx, cy, px, py) <= r*r + EPSILON;
}

static int rect_line(double rx, double ry, double rw, double rh, double lx1, double ly1, double lx2, double ly2) {
    // Ponto dentro (Inclusive)
    if (lx1 >= rx && lx1 <= rx+rw && ly1 >= ry && ly1 <= ry+rh) return 1;
    if (lx2 >= rx && lx2 <= rx+rw && ly2 >= ry && ly2 <= ry+rh) return 1;
    // Checa intersecção com 4 bordas
    if (line_line_overlap(lx1, ly1, lx2, ly2, rx, ry, rx+rw, ry)) return 1;
    if (line_line_overlap(lx1, ly1, lx2, ly2, rx+rw, ry, rx+rw, ry+rh)) return 1;
    if (line_line_overlap(lx1, ly1, lx2, ly2, rx+rw, ry+rh, rx, ry+rh)) return 1;
    if (line_line_overlap(lx1, ly1, lx2, ly2, rx, ry+rh, rx, ry)) return 1;
    return 0;
}

// Verify Overlap Main Function
int geo_verificar_sobreposicao(void *shapeA_ptr, void *shapeB_ptr)
{
  Shape_t *A = (Shape_t *)shapeA_ptr;
  Shape_t *B = (Shape_t *)shapeB_ptr;
  if (!A || !B) return 0;

  // Normalização para reduzir combinações
  // Ordem de complexidade: CIRCLE, RECT, LINE, TEXT
  // Se precisarmos, invertemos A e B para cair num case implementado.
  // Implementação direta cobrindo tudo:
  
  // Extrai dados genéricos ou específicos
  if (A->type == CIRCLE && B->type == CIRCLE) 
    return circ_circ(circulo_get_x(A->data), circulo_get_y(A->data), circulo_get_raio(A->data),
                     circulo_get_x(B->data), circulo_get_y(B->data), circulo_get_raio(B->data));
                     
  if (A->type == RECTANGLE && B->type == RECTANGLE)
    return rect_rect(retangulo_get_x(A->data), retangulo_get_y(A->data), retangulo_get_largura(A->data), retangulo_get_altura(A->data),
                     retangulo_get_x(B->data), retangulo_get_y(B->data), retangulo_get_largura(B->data), retangulo_get_altura(B->data));
                     
  if (A->type == LINE && B->type == LINE) {
      Line l1 = A->data, l2 = B->data;
      return line_line_overlap(line_get_x1(l1), line_get_y1(l1), line_get_x2(l1), line_get_y2(l1),
                               line_get_x1(l2), line_get_y1(l2), line_get_x2(l2), line_get_y2(l2));
  }
  
  // Mixed Cases
  // Circle x Rect
  if (A->type == CIRCLE && B->type == RECTANGLE)
      return circ_rect(circulo_get_x(A->data), circulo_get_y(A->data), circulo_get_raio(A->data),
                       retangulo_get_x(B->data), retangulo_get_y(B->data), retangulo_get_largura(B->data), retangulo_get_altura(B->data));
  if (A->type == RECTANGLE && B->type == CIRCLE)
      return circ_rect(circulo_get_x(B->data), circulo_get_y(B->data), circulo_get_raio(B->data),
                       retangulo_get_x(A->data), retangulo_get_y(A->data), retangulo_get_largura(A->data), retangulo_get_altura(A->data));

  // Circle x Line
  if (A->type == CIRCLE && B->type == LINE) {
      Line l = B->data;
      return circ_line(circulo_get_x(A->data), circulo_get_y(A->data), circulo_get_raio(A->data),
                       line_get_x1(l), line_get_y1(l), line_get_x2(l), line_get_y2(l));
  }
  if (A->type == LINE && B->type == CIRCLE) {
      Line l = A->data;
      return circ_line(circulo_get_x(B->data), circulo_get_y(B->data), circulo_get_raio(B->data),
                       line_get_x1(l), line_get_y1(l), line_get_x2(l), line_get_y2(l));
  }
  
  // Rect x Line
  if (A->type == RECTANGLE && B->type == LINE) {
       Line l = B->data;
       return rect_line(retangulo_get_x(A->data), retangulo_get_y(A->data), retangulo_get_largura(A->data), retangulo_get_altura(A->data),
                        line_get_x1(l), line_get_y1(l), line_get_x2(l), line_get_y2(l));
  }
  if (A->type == LINE && B->type == RECTANGLE) {
       Line l = A->data;
       return rect_line(retangulo_get_x(B->data), retangulo_get_y(B->data), retangulo_get_largura(B->data), retangulo_get_altura(B->data),
                        line_get_x1(l), line_get_y1(l), line_get_x2(l), line_get_y2(l));
  }

  // TEXT Cases - Convert Text to Line and recurse (or manual check)
  // Easier to use manual check with text_to_line
  
  if (A->type == TEXT) {
      double tx1, ty1, tx2, ty2;
      text_to_line(A->data, &tx1, &ty1, &tx2, &ty2);
      
      // Check collision of this segment with B
      if (B->type == CIRCLE) return circ_line(circulo_get_x(B->data), circulo_get_y(B->data), circulo_get_raio(B->data), tx1, ty1, tx2, ty2);
      if (B->type == RECTANGLE) return rect_line(retangulo_get_x(B->data), retangulo_get_y(B->data), retangulo_get_largura(B->data), retangulo_get_altura(B->data), tx1, ty1, tx2, ty2);
      if (B->type == LINE) { Line l = B->data; return line_line_overlap(tx1, ty1, tx2, ty2, line_get_x1(l), line_get_y1(l), line_get_x2(l), line_get_y2(l)); }
      if (B->type == TEXT) {
          double ux1, uy1, ux2, uy2;
          text_to_line(B->data, &ux1, &uy1, &ux2, &uy2);
          return line_line_overlap(tx1, ty1, tx2, ty2, ux1, uy1, ux2, uy2);
      }
  }
  
  // If B is TEXT (and A is not, handled above), swap
  if (B->type == TEXT) return geo_verificar_sobreposicao(B, A);

  return 0;
}

// ==========================================
// FUNÇÕES DE DESENHO E CLONAGEM (Mantidas)
// ==========================================

void geo_escrever_svg_forma(void *shape_ptr, FILE *file)
{
  Shape_t *shape = (Shape_t *)shape_ptr;
  if (!shape || !file) return;

  if (shape->type == CIRCLE) {
    Circulo c = (Circulo)shape->data;
    fprintf(file, "<circle cx='%.2f' cy='%.2f' r='%.2f' fill='%s' stroke='%s' stroke-width='1' fill-opacity='0.7'/>\n",
            circulo_get_x(c), circulo_get_y(c), circulo_get_raio(c), circulo_get_cor_preenchimento(c), circulo_get_cor_borda(c));
  } else if (shape->type == RECTANGLE) {
    Rectangle r = (Retangulo)shape->data;
    fprintf(file, "<rect x='%.2f' y='%.2f' width='%.2f' height='%.2f' fill='%s' stroke='%s' stroke-width='1' fill-opacity='0.7'/>\n",
            retangulo_get_x(r), retangulo_get_y(r), retangulo_get_largura(r), retangulo_get_altura(r),
            retangulo_get_cor_preenchimento(r), retangulo_get_cor_borda(r));
  } else if (shape->type == LINE) {
    Line l = (Line)shape->data;
    fprintf(file, "<line x1='%.2f' y1='%.2f' x2='%.2f' y2='%.2f' stroke='%s' stroke-width='1'/>\n",
            line_get_x1(l), line_get_y1(l), line_get_x2(l), line_get_y2(l), line_get_color(l));
  } else if (shape->type == TEXT) {
    Text t = (Text)shape->data;
    char a = text_get_anchor(t);
    const char *anchor = (a == 'm') ? "middle" : (a == 'e' || a == 'f') ? "end" : "start";
    fprintf(file, "<text x='%.2f' y='%.2f' fill='%s' stroke='%s' text-anchor='%s'>%s</text>\n",
            text_get_x(t), text_get_y(t), text_get_fill_color(t), text_get_border_color(t), anchor, text_get_text(t));
  }
}

void *geo_clonar_forma(void *shape_ptr, double x, double y, Ground ground)
{
  Shape_t *src = (Shape_t *)shape_ptr;
  if (!src) return NULL;
  void *newData = NULL;

  if (src->type == CIRCLE) {
    Circulo c = (Circulo)src->data;
    newData = circulo_criar(circulo_get_id(c), x, y, circulo_get_raio(c), circulo_get_cor_borda(c), circulo_get_cor_preenchimento(c));
  } else if (src->type == RECTANGLE) {
    Rectangle r = (Rectangle)src->data;
    newData = retangulo_criar(retangulo_get_id(r), x, y, retangulo_get_largura(r), retangulo_get_altura(r), retangulo_get_cor_borda(r), retangulo_get_cor_preenchimento(r));
  } else if (src->type == LINE) {
    Line l = (Line)src->data;
    double dx = x - line_get_x1(l), dy = y - line_get_y1(l);
    newData = line_create(line_get_id(l), x, y, line_get_x2(l) + dx, line_get_y2(l) + dy, line_get_color(l));
  } else if (src->type == TEXT) {
    Text t = (Text)src->data;
    newData = text_create(text_get_id(t), x, y, text_get_border_color(t), text_get_fill_color(t), text_get_anchor(t), text_get_text(t));
  }

  if (newData) {
    Shape_t *newShape = malloc(sizeof(Shape_t));
    newShape->type = src->type;
    newShape->data = newData;
    if (ground) {
      Stack s = get_ground_shapes_stack_to_free(ground);
      if (s) stack_push(s, newShape);
    }
    return newShape;
  }
  return NULL;
}

void geo_destruir_forma(void *shape_ptr)
{
  Shape_t *shape = (Shape_t *)shape_ptr;
  if (!shape) return;

  if (shape->type == CIRCLE) circulo_destruir(shape->data);
  else if (shape->type == RECTANGLE) retangulo_destruir(shape->data);
  else if (shape->type == LINE) line_destroy(shape->data);
  else if (shape->type == TEXT) text_destroy(shape->data);
  else if (shape->type == TEXT_STYLE) text_style_destroy(shape->data);
  
  free(shape);
}

// ... Parsing Logic identical ...
Ground execute_geo_commands(FileData fileData, const char *output_path, const char *command_suffix)
{
  Ground_t *ground = malloc(sizeof(Ground_t));
  if (!ground) exit(1);
  ground->shapesQueue = queue_create();
  ground->shapesStackToFree = stack_create();
  ground->svgQueue = queue_create();
  Queue lines = getLinesQueue(fileData);
  if (!lines) { free(ground); return NULL; }
  while (!queue_is_empty(lines)) {
    char *line = (char *)queue_dequeue(lines);
    char *command = strtok(line, " ");
    if (!command) continue;
    if (strcmp(command, "c") == 0) executar_comando_circulo(ground);
    else if (strcmp(command, "r") == 0) execute_rectangle_command(ground);
    else if (strcmp(command, "l") == 0) execute_line_command(ground);
    else if (strcmp(command, "t") == 0) execute_text_command(ground);
    else if (strcmp(command, "ts") == 0) execute_text_style_command(ground);
  }
  create_svg_queue(ground, output_path, fileData, command_suffix);
  return ground;
}

void destroy_geo_waste(Ground ground)
{
  if (!ground) return;
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

static void executar_comando_circulo(Ground_t *g) {
  char *id = strtok(NULL, " "), *x = strtok(NULL, " "), *y = strtok(NULL, " "), *r = strtok(NULL, " "), *cb = strtok(NULL, " "), *cp = strtok(NULL, " ");
  Shape_t *s = malloc(sizeof(Shape_t)); s->type = CIRCLE;
  s->data = circulo_criar(atoi(id), atof(x), atof(y), atof(r), cb, cp);
  queue_enqueue(g->shapesQueue, s); stack_push(g->shapesStackToFree, s); queue_enqueue(g->svgQueue, s);
}
static void execute_rectangle_command(Ground_t *g) {
  char *id = strtok(NULL, " "), *x = strtok(NULL, " "), *y = strtok(NULL, " "), *w = strtok(NULL, " "), *h = strtok(NULL, " "), *cb = strtok(NULL, " "), *cp = strtok(NULL, " ");
  Shape_t *s = malloc(sizeof(Shape_t)); s->type = RECTANGLE;
  s->data = retangulo_criar(atoi(id), atof(x), atof(y), atof(w), atof(h), cb, cp);
  queue_enqueue(g->shapesQueue, s); stack_push(g->shapesStackToFree, s); queue_enqueue(g->svgQueue, s);
}
static void execute_line_command(Ground_t *g) {
  char *id = strtok(NULL, " "), *x1 = strtok(NULL, " "), *y1 = strtok(NULL, " "), *x2 = strtok(NULL, " "), *y2 = strtok(NULL, " "), *c = strtok(NULL, " ");
  Shape_t *s = malloc(sizeof(Shape_t)); s->type = LINE;
  s->data = line_create(atoi(id), atof(x1), atof(y1), atof(x2), atof(y2), c);
  queue_enqueue(g->shapesQueue, s); stack_push(g->shapesStackToFree, s); queue_enqueue(g->svgQueue, s);
}
static void execute_text_command(Ground_t *g) {
  char *id = strtok(NULL, " "), *x = strtok(NULL, " "), *y = strtok(NULL, " "), *cb = strtok(NULL, " "), *cp = strtok(NULL, " "), *a = strtok(NULL, " "), *t = strtok(NULL, "");
  Shape_t *s = malloc(sizeof(Shape_t)); s->type = TEXT;
  s->data = text_create(atoi(id), atof(x), atof(y), cb, cp, *a, t);
  queue_enqueue(g->shapesQueue, s); stack_push(g->shapesStackToFree, s); queue_enqueue(g->svgQueue, s);
}
static void execute_text_style_command(Ground_t *g) {
  char *f = strtok(NULL, " "), *w = strtok(NULL, " "), *sz = strtok(NULL, " ");
  Shape_t *s = malloc(sizeof(Shape_t)); s->type = TEXT_STYLE;
  s->data = text_style_create(f, *w, atoi(sz));
  queue_enqueue(g->shapesQueue, s); stack_push(g->shapesStackToFree, s); queue_enqueue(g->svgQueue, s);
}
static void create_svg_queue(Ground_t *g, const char *path, FileData fd, const char *suf) {
  char *fname = malloc(strlen(getFileName(fd)) + 50); strcpy(fname, getFileName(fd));
  if (suf) { strcat(fname, "-"); strcat(fname, suf); }
  char *out = malloc(strlen(path) + strlen(fname) + 10); sprintf(out, "%s/%s.svg", path, fname);
  
  // Calc BBox
  double minX = DBL_MAX, minY = DBL_MAX, maxX = -DBL_MAX, maxY = -DBL_MAX;
  int temAlgo = 0;
  
  Queue temp = queue_create();
  while (!queue_is_empty(g->svgQueue)) {
      void *s = queue_dequeue(g->svgQueue);
      geo_get_bbox(s, &minX, &minY, &maxX, &maxY);
      temAlgo = 1;
      queue_enqueue(temp, s);
  }
  while (!queue_is_empty(temp)) {
      queue_enqueue(g->svgQueue, queue_dequeue(temp));
  }
  queue_destroy(temp);

  double margin = 20.0;
  double width = 500.0, height = 500.0;
  double tx = 0.0, ty = 0.0;
  
  if (maxX > -DBL_MAX && temAlgo) {
      width = (maxX - minX) + 2 * margin;
      height = (maxY - minY) + 2 * margin;
      tx = margin - minX;
      ty = margin - minY;
  }

  FILE *f = fopen(out, "w");
  if (f) {
    fprintf(f, "<?xml version=\"1.0\"?>\n<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%.2f\" height=\"%.2f\">\n", width, height);
    if (temAlgo) fprintf(f, "<g transform=\"translate(%.2f, %.2f)\">\n", tx, ty);
    
    // Scan again to write
    // Instead of copying logic, reusing queue
    // Note: svgQueue preserves order.
    // Ideally we shouldn't dequeue/enqueue twice but needed for BBox
    
    // We can just iterate the queue again? 
    // Wait, queue_dequeue removes it. We restored it above.
    // So we can cycle it again.
    
    // Or we use a temporary loop again?
    // Since queue implementation is opaque, let's use the same dequeue/enqueue pattern.
    temp = queue_create();
    while (!queue_is_empty(g->svgQueue)) {
      Shape_t *s = queue_dequeue(g->svgQueue);
      if (s) geo_escrever_svg_forma(s, f);
      queue_enqueue(temp, s);
    }
    // Restore queue for consistency though likely not needed after this
    while (!queue_is_empty(temp))
         queue_enqueue(g->svgQueue, queue_dequeue(temp));
    queue_destroy(temp);

    if (temAlgo) fprintf(f, "</g>\n");
    fprintf(f, "</svg>"); fclose(f);
  }
  free(fname); free(out);
}

// [MODIFICADO] Formatação estilo Ash (Multi-linha)
void geo_imprimir_forma_txt(void *shape_ptr, FILE *txt)
{
  if (!shape_ptr || !txt) return;
  Shape_t *shape = (Shape_t *)shape_ptr;
  
  if (shape->type == CIRCLE) {
    Circulo c = (Circulo)shape->data;
    fprintf(txt, "\nCírculo %d\nx: %.2f\ny: %.2f\nr: %.2f\ncorb: %s (sem nome)\ncorp: %s (sem nome)\n", 
           circulo_get_id(c), circulo_get_x(c), circulo_get_y(c), circulo_get_raio(c), circulo_get_cor_borda(c), circulo_get_cor_preenchimento(c));
  } else if (shape->type == RECTANGLE) {
    Rectangle r = (Retangulo)shape->data;
    fprintf(txt, "\nRetângulo %d\nx: %.2f\ny: %.2f\nw: %.2f\nh: %.2f\ncorb: %s (sem nome)\ncorp: %s (sem nome)\n", 
            retangulo_get_id(r), retangulo_get_x(r), retangulo_get_y(r), retangulo_get_largura(r), retangulo_get_altura(r), 
            retangulo_get_cor_borda(r), retangulo_get_cor_preenchimento(r));
  } else if (shape->type == LINE) {
    Line l = (Line)shape->data;
    fprintf(txt, "\nLinha %d\nx1: %.2f\ny1: %.2f\nx2: %.2f\ny2: %.2f\ncor: %s (sem nome)\n", 
            line_get_id(l), line_get_x1(l), line_get_y1(l), line_get_x2(l), line_get_y2(l), line_get_color(l));
  } else if (shape->type == TEXT) {
    Text t = (Text)shape->data;
    fprintf(txt, "\nTexto %d\nx: %.2f\ny: %.2f\ncorb: %s (sem nome)\ncorp: %s (sem nome)\nfont: sans-serif\nweight: normal\nsize: 20\ntxt: \"%s\"\n",
            text_get_id(t), text_get_x(t), text_get_y(t), text_get_border_color(t), text_get_fill_color(t), text_get_text(t));
  }
}

void geo_aplicar_efeitos_clonagem(void *I_ptr, void *J_ptr, void *I_clone_ptr)
{
  if (!I_ptr || !J_ptr || !I_clone_ptr) return;
  Shape_t *I = (Shape_t *)I_ptr;
  Shape_t *J = (Shape_t *)J_ptr;
  Shape_t *Ic = (Shape_t *)I_clone_ptr;

  char *fill_I = NULL;
  if (I->type == RECTANGLE) fill_I = ((struct Retangulo *)I->data)->fill_color;
  else if (I->type == CIRCLE) fill_I = ((struct Circulo *)I->data)->fill_color;

  if (fill_I) {
    if (J->type == RECTANGLE) {
      struct Retangulo *rJ = (struct Retangulo *)J->data;
      if (rJ->border_color) free(rJ->border_color);
      rJ->border_color = duplicate_string(fill_I);
    } else if (J->type == CIRCLE) {
      struct Circulo *cJ = (struct Circulo *)J->data;
      if (cJ->border_color) free(cJ->border_color);
      cJ->border_color = duplicate_string(fill_I);
    }
  }

  if (Ic->type == RECTANGLE) {
    struct Retangulo *rIc = (struct Retangulo *)Ic->data;
    char *temp = rIc->fill_color;
    rIc->fill_color = rIc->border_color;
    rIc->border_color = temp;
  } else if (Ic->type == CIRCLE) {
    struct Circulo *cIc = (struct Circulo *)Ic->data;
    char *temp = cIc->fill_color;
    cIc->fill_color = cIc->border_color;
    cIc->border_color = temp;
  }
}

// ==========================================
// CÁLCULO DE VIEWBOX / BBOX
// ==========================================

void geo_get_bbox(void *shape_ptr, double *minX, double *minY, double *maxX, double *maxY)
{
    Shape_t *shape = (Shape_t *)shape_ptr;
    if (!shape) return;

    if (shape->type == CIRCLE) {
        Circulo c = (Circulo)shape->data;
        double x = circulo_get_x(c);
        double y = circulo_get_y(c);
        double r = circulo_get_raio(c);
        
        if (x - r < *minX) *minX = x - r;
        if (x + r > *maxX) *maxX = x + r;
        if (y - r < *minY) *minY = y - r;
        if (y + r > *maxY) *maxY = y + r;
    } 
    else if (shape->type == RECTANGLE) {
        Rectangle r = (Retangulo)shape->data;
        double x = retangulo_get_x(r);
        double y = retangulo_get_y(r);
        double w = retangulo_get_largura(r);
        double h = retangulo_get_altura(r);

        if (x < *minX) *minX = x;
        if (x + w > *maxX) *maxX = x + w;
        if (y < *minY) *minY = y;
        if (y + h > *maxY) *maxY = y + h;
    }
    else if (shape->type == LINE) {
        Line l = (Line)shape->data;
        double x1 = line_get_x1(l);
        double y1 = line_get_y1(l);
        double x2 = line_get_x2(l);
        double y2 = line_get_y2(l);
        
        double lx = fmin(x1, x2);
        double rx = fmax(x1, x2);
        double ty = fmin(y1, y2);
        double by = fmax(y1, y2);

        if (lx < *minX) *minX = lx;
        if (rx > *maxX) *maxX = rx;
        if (ty < *minY) *minY = ty;
        if (by > *maxY) *maxY = by;
    }
    else if (shape->type == TEXT) {
        Text t = (Text)shape->data;
        double x = text_get_x(t);
        double y = text_get_y(t);
        // Estimativa simplificada
        const char *txt = text_get_text(t);
        int len = txt ? strlen(txt) : 0;
        double width = 10.0 * len; // Aproximação
        double height = 20.0; 
        char anchor = text_get_anchor(t);

        double x1 = x, x2 = x;
        if (anchor == 'i') { x2 = x + width; }
        else if (anchor == 'm') { x1 = x - width/2.0; x2 = x + width/2.0; }
        else { x1 = x - width; } 

        if (x1 < *minX) *minX = x1;
        if (x2 > *maxX) *maxX = x2;
        if (y - height < *minY) *minY = y - height;
        if (y > *maxY) *maxY = y;
    }
}