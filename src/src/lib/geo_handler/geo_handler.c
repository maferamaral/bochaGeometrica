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
#include <stdlib.h> // Adicionado para malloc/free

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

// private functions
static void executar_comando_circulo(Ground_t *ground);
static void execute_rectangle_command(Ground_t *ground);
static void execute_line_command(Ground_t *ground);
static void execute_text_command(Ground_t *ground);
static void execute_text_style_command(Ground_t *ground);
static void create_svg_queue(Ground_t *ground, const char *output_path,
                             FileData fileData, const char *command_suffix);

Ground execute_geo_commands(FileData fileData, const char *output_path,
                            const char *command_suffix)
{
  Ground_t *ground = malloc(sizeof(Ground_t));
  if (ground == NULL)
  {
    printf("Error: Failed to allocate memory for Ground\n");
    exit(1);
  }

  ground->shapesQueue = queue_create();
  ground->shapesStackToFree = stack_create();
  ground->svgQueue = queue_create();

  Queue lines = getLinesQueue(fileData);
  if (!lines)
  {
    free(ground);
    return NULL;
  }

  // Iterar sobre uma cópia ou preservando a fila original se necessário
  // Assumindo que podemos consumir a fila do FileData:
  while (!queue_is_empty(lines))
  {
    char *line = (char *)queue_dequeue(lines);
    // Fazemos uma cópia da linha para não estragar a string original se for reutilizada
    // Mas queue_dequeue retorna o ponteiro. O strtok modifica.
    // O ideal seria duplicar, mas vamos seguir o padrão do teu código.
    char *command = strtok(line, " ");

    if (command == NULL)
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
  Ground_t *ground_t = (Ground_t *)ground;
  queue_destroy(ground_t->shapesQueue);
  queue_destroy(ground_t->svgQueue);
  while (!stack_is_empty(ground_t->shapesStackToFree))
  {
    Shape_t *shape = stack_pop(ground_t->shapesStackToFree);
    // Nota: data é libertado pelos destrutores específicos se necessário,
    // mas aqui estamos apenas a libertar o wrapper Shape_t
    free(shape);
  }
  stack_destroy(ground_t->shapesStackToFree);
  free(ground);
}

Queue get_ground_queue(Ground ground)
{
  if (!ground)
    return NULL;
  Ground_t *ground_t = (Ground_t *)ground;
  return ground_t->shapesQueue;
}

Stack get_ground_shapes_stack_to_free(Ground ground)
{
  if (!ground)
    return NULL;
  Ground_t *ground_t = (Ground_t *)ground;
  return ground_t->shapesStackToFree;
}

// ========================
// FUNÇÃO PÚBLICA DE DESENHO (NOVA)
// ========================
void geo_escrever_svg_forma(void *shape_ptr, FILE *file)
{
  Shape_t *shape = (Shape_t *)shape_ptr;
  if (!shape || !file)
    return;

  if (shape->type == CIRCLE)
  {
    Circulo c = (Circulo)shape->data;
    fprintf(file, "<circle cx='%.2f' cy='%.2f' r='%.2f' fill='%s' stroke='%s' stroke-width='1'/>\n",
            circulo_get_x(c), circulo_get_y(c), circulo_get_raio(c),
            circulo_get_cor_preenchimento(c), circulo_get_cor_borda(c));
  }
  else if (shape->type == RECTANGLE)
  {
    Rectangle r = (Retangulo)shape->data;
    fprintf(file, "<rect x='%.2f' y='%.2f' width='%.2f' height='%.2f' fill='%s' stroke='%s' stroke-width='1'/>\n",
            retangulo_get_x(r), retangulo_get_y(r),
            retangulo_get_largura(r), retangulo_get_altura(r),
            retangulo_get_cor_preenchimento(r), retangulo_get_cor_borda(r));
  }
  else if (shape->type == LINE)
  {
    Line l = (Line)shape->data;
    fprintf(file, "<line x1='%.2f' y1='%.2f' x2='%.2f' y2='%.2f' stroke='%s' stroke-width='1'/>\n",
            line_get_x1(l), line_get_y1(l), line_get_x2(l), line_get_y2(l),
            line_get_color(l));
  }
  else if (shape->type == TEXT)
  {
    Text t = (Text)shape->data;
    char anchor = text_get_anchor(t);
    const char *anchorStr = "start";
    if (anchor == 'm')
      anchorStr = "middle";
    if (anchor == 'e' || anchor == 'f')
      anchorStr = "end";

    fprintf(file, "<text x='%.2f' y='%.2f' fill='%s' stroke='%s' text-anchor='%s'>%s</text>\n",
            text_get_x(t), text_get_y(t), text_get_fill_color(t),
            text_get_border_color(t), anchorStr, text_get_text(t));
  }
}

// ========================
// Funções Privadas
// ========================

static void executar_comando_circulo(Ground_t *ground)
{
  char *identifier = strtok(NULL, " ");
  char *posX = strtok(NULL, " ");
  char *posY = strtok(NULL, " ");
  char *radius = strtok(NULL, " ");
  char *borderColor = strtok(NULL, " ");
  char *fillColor = strtok(NULL, " ");

  Circulo circulo = circulo_criar(atoi(identifier), atof(posX), atof(posY),
                                  atof(radius), borderColor, fillColor);

  Shape_t *shape = malloc(sizeof(Shape_t));
  shape->type = CIRCLE;
  shape->data = circulo;
  queue_enqueue(ground->shapesQueue, shape);
  stack_push(ground->shapesStackToFree, shape);
  queue_enqueue(ground->svgQueue, shape);
}

static void execute_rectangle_command(Ground_t *ground)
{
  char *identifier = strtok(NULL, " ");
  char *posX = strtok(NULL, " ");
  char *posY = strtok(NULL, " ");
  char *width = strtok(NULL, " ");
  char *height = strtok(NULL, " ");
  char *borderColor = strtok(NULL, " ");
  char *fillColor = strtok(NULL, " ");

  Rectangle rectangle =
      retangulo_criar(atoi(identifier), atof(posX), atof(posY), atof(width),
                      atof(height), borderColor, fillColor);

  Shape_t *shape = malloc(sizeof(Shape_t));
  shape->type = RECTANGLE;
  shape->data = rectangle;
  queue_enqueue(ground->shapesQueue, shape);
  stack_push(ground->shapesStackToFree, shape);
  queue_enqueue(ground->svgQueue, shape);
}

static void execute_line_command(Ground_t *ground)
{
  char *identifier = strtok(NULL, " ");
  char *x1 = strtok(NULL, " ");
  char *y1 = strtok(NULL, " ");
  char *x2 = strtok(NULL, " ");
  char *y2 = strtok(NULL, " ");
  char *color = strtok(NULL, " ");

  Line line = line_create(atoi(identifier), atof(x1), atof(y1), atof(x2),
                          atof(y2), color);

  Shape_t *shape = malloc(sizeof(Shape_t));
  shape->type = LINE;
  shape->data = line;
  queue_enqueue(ground->shapesQueue, shape);
  stack_push(ground->shapesStackToFree, shape);
  queue_enqueue(ground->svgQueue, shape);
}

static void execute_text_command(Ground_t *ground)
{
  char *identifier = strtok(NULL, " ");
  char *posX = strtok(NULL, " ");
  char *posY = strtok(NULL, " ");
  char *borderColor = strtok(NULL, " ");
  char *fillColor = strtok(NULL, " ");
  char *anchor = strtok(NULL, " ");
  char *text = strtok(NULL, "");

  Text text_obj = text_create(atoi(identifier), atof(posX), atof(posY),
                              borderColor, fillColor, *anchor, text);

  Shape_t *shape = malloc(sizeof(Shape_t));
  shape->type = TEXT;
  shape->data = text_obj;
  queue_enqueue(ground->shapesQueue, shape);
  stack_push(ground->shapesStackToFree, shape);
  queue_enqueue(ground->svgQueue, shape);
}

static void execute_text_style_command(Ground_t *ground)
{
  char *fontFamily = strtok(NULL, " ");
  char *fontWeight = strtok(NULL, " ");
  char *fontSize = strtok(NULL, " ");

  TextStyle text_style_obj =
      text_style_create(fontFamily, *fontWeight, atoi(fontSize));

  Shape_t *shape = malloc(sizeof(Shape_t));
  shape->type = TEXT_STYLE;
  shape->data = text_style_obj;
  queue_enqueue(ground->shapesQueue, shape);
  stack_push(ground->shapesStackToFree, shape);
  queue_enqueue(ground->svgQueue, shape);
}

static void create_svg_queue(Ground_t *ground, const char *output_path,
                             FileData fileData, const char *command_suffix)
{
  const char *original_file_name = getFileName(fileData);
  size_t name_len = strlen(original_file_name);
  char *file_name = malloc(name_len + 50); // margem extra
  strcpy(file_name, original_file_name);
  if (command_suffix != NULL)
  {
    strcat(file_name, "-");
    strcat(file_name, command_suffix);
  }

  size_t path_len = strlen(output_path);
  size_t total_len = path_len + strlen(file_name) + 10;
  char *output_path_with_file = malloc(total_len);

  sprintf(output_path_with_file, "%s/%s.svg", output_path, file_name);

  FILE *file = fopen(output_path_with_file, "w");
  if (file != NULL)
  {
    fprintf(file, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(file, "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 1000 1000\">\n");

    // Desenha formas da fila SVG temporária criada durante parsing
    while (!queue_is_empty(ground->svgQueue))
    {
      Shape_t *shape = queue_dequeue(ground->svgQueue);
      if (shape)
        geo_escrever_svg_forma(shape, file);
    }

    fprintf(file, "</svg>\n");
    fclose(file);
  }

  free(output_path_with_file);
  free(file_name);
}