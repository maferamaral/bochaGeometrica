#include "../fila/fila.h"
#include "../pilha/pilha.h"
#include "../utils/utils.h"
#include "../geo_handler/geo_handler.h"
#include "../formas/circulo/circulo.h"
#include "../formas/linha/linha.h"
#include "../formas/retangulo/retangulo.h"
#include "../formas/formas.h"
#include "../formas/texto/texto.h"
#include "qry_handler.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

typedef struct
{
  TipoForma type;
  void *data;
} Shape_t;

typedef struct
{
  int id;
  Stack *shapes;
} Loader_t;

typedef struct
{
  int id;
  double x;
  double y;
  Shape_t *shootingPosition;
  Loader_t *rightLoader;
  Loader_t *leftLoader;
  int rightLoaderId; // stable identifier, avoids dangling pointers after
                     // realloc
  int leftLoaderId;  // stable identifier, avoids dangling pointers after realloc
} Shooter_t;

typedef enum
{
  FREE_SHOOTERS_ARRAY,
  FREE_LOADERS_ARRAY,
  FREE_SHAPE_POSITION,
  FREE_STACK_HANDLE
} FreeType;

typedef struct
{
  void *ptr;
  FreeType type;
} FreeItem;

typedef struct
{
  Stack arena;            // elementos são ShapePositionOnArena_t
  Stack pilhaParaLiberar; // elementos são FreeItem
} Qry_t;

typedef struct
{
  Shape_t *shape;
  double x;
  double y;
  bool isAnnotated;
  double shooterX;
  double shooterY;
} ShapePositionOnArena_t;

// private functions
static void executar_comando_pd(Shooter_t **atiradores, int *atiradoresCount,
                                Stack pilhaParaLiberar);
static void executar_comando_lc(Loader_t **carregadores, int *carregadoresCount,
                                Ground ground, Stack pilhaParaLiberar, FILE *arquivoTxt);
static void executar_comando_atch(Loader_t **carregadores, int *carregadoresCount,
                                  Shooter_t **atiradores, int *atiradoresCount,
                                  Stack pilhaParaLiberar);
static void executar_operacao_deslocamento(Shooter_t **atiradores, int atiradoresCount,
                                           int idAtirador, const char *direcao,
                                           int vezes, Loader_t *carregadores,
                                           int carregadoresCount);
static void executar_operacao_disparo(Shooter_t **atiradores, int atiradoresCount,
                                      int idAtirador, double deslocamentoX, double deslocamentoY,
                                      const char *anotar, Stack arena,
                                      Stack pilhaParaLiberar);
static void executar_comando_shft(Shooter_t **atiradores, int *atiradoresCount,
                                  Loader_t *carregadores, int *carregadoresCount,
                                  FILE *arquivoTxt);
static void executar_comando_dsp(Shooter_t **atiradores, int *atiradoresCount,
                                 Stack arena, Stack pilhaParaLiberar, FILE *arquivoTxt);
static void executar_comando_rjd(Shooter_t **atiradores, int *atiradoresCount,
                                 Stack pilhaParaLiberar, Stack arena,
                                 Loader_t *carregadores, int *carregadoresCount,
                                 FILE *arquivoTxt);
static void executar_comando_calc(Stack arena, Ground ground, int quantidadeDeComandos, FILE *arquivoTxt);
static int encontrar_indice_atirador_por_id(Shooter_t **atiradores, int atiradoresCount,
                                            int id);

void destroy_qry_waste(Qry qry)
{
  Qry_t *qry_t = (Qry_t *)qry;
  while (!stack_is_empty(qry_t->pilhaParaLiberar))
  {
    FreeItem *item = (FreeItem *)stack_pop(qry_t->pilhaParaLiberar);

    if (item != NULL && item->ptr != NULL)
    {
      switch (item->type)
      {
      case FREE_SHOOTERS_ARRAY:
      case FREE_LOADERS_ARRAY:
      case FREE_SHAPE_POSITION:
        free(item->ptr);
        break;
      case FREE_STACK_HANDLE:
      {
        Stack *stack_handle = (Stack *)item->ptr;
        if (stack_handle != NULL && *stack_handle != NULL)
        {
          stack_destroy(*stack_handle);
        }
        free(item->ptr);
        break;
      }
      }
      free(item);
    }
  }
  stack_destroy(qry_t->arena);
  stack_destroy(qry_t->pilhaParaLiberar);
  free(qry_t);
}

// Helpers for calc
static double calcular_area_forma(TipoForma type, void *dadosForma);
typedef struct
{
  double minX;
  double minY;
  double maxX;
  double maxY;
} Aabb;
static Aabb criar_aabb_para_forma_na_arena(const ShapePositionOnArena_t *s);
static bool aabb_sobrepoe(Aabb a, Aabb b);
static bool formas_sobrepoem(const ShapePositionOnArena_t *a,
                             const ShapePositionOnArena_t *b);
static Shape_t *criar_wrapper_forma(TipoForma type, void *data);
static Shape_t *clonar_com_cor_borda(Shape_t *src,
                                     const char *novaCorBorda);
static Shape_t *clonar_com_cores_trocadas(Shape_t *src);
// Clone helpers setting a new position (x,y) based on arena placement
static Shape_t *clonar_com_posicao(Shape_t *src, double x, double y,
                                   Ground ground);
static Shape_t *clonar_com_cor_borda_na_posicao(Shape_t *src,
                                                const char *novaCorBorda,
                                                double x, double y,
                                                Ground ground);
static Shape_t *clonar_com_cores_trocadas_na_posicao(Shape_t *src, double x,
                                                     double y, Ground ground);
static void destruir_forma(Shape_t *s);

// SVG writer for final .qry result
static void escrever_svg_resultado_qry(FileData qryFileData, FileData geoFileData,
                                       Ground ground, Stack arena,
                                       const char *output_path);

Qry executar_comandos_qry(FileData qryFileData, FileData geoFileData,
                          Ground ground, const char *output_path)
{

  Qry_t *qry = malloc(sizeof(Qry_t));
  if (qry == NULL)
  {
    printf("Error: Failed to allocate memory for Qry\n");
    exit(1);
  }
  qry->arena = stack_create();
  qry->pilhaParaLiberar = stack_create();

  int quantidadeDeComandos = queue_size(getLinesQueue(qryFileData));

  // Note: qry should NOT be added to pilhaParaLiberar as it causes premature freeing

  Shooter_t *atiradores = NULL;
  int atiradoresCount = 0;
  Loader_t *carregadores = NULL;
  int carregadoresCount = 0;

  // Abrir arquivo .txt com o mesmo nome-base do SVG de saída, mas extensão .txt
  size_t geo_len = strlen(getFileName(geoFileData));
  size_t qry_len = strlen(getFileName(qryFileData));
  char *geo_base = malloc(geo_len + 1);
  char *qry_base = malloc(qry_len + 1);
  if (geo_base == NULL || qry_base == NULL)
  {
    printf("Error: Memory allocation failed for file name\n");
    free(geo_base);
    free(qry_base);
    return NULL;
  }
  strcpy(geo_base, getFileName(geoFileData));
  strcpy(qry_base, getFileName(qryFileData));
  strtok(geo_base, ".");
  strtok(qry_base, ".");
  size_t path_len = strlen(output_path);
  // geoBase-qryBase.txt
  size_t processed_name_len = strlen(geo_base) + 1 + strlen(qry_base);
  size_t total_len = path_len + 1 + processed_name_len + 4 + 1; // +4 for ".txt"
  char *output_txt_path = malloc(total_len);
  if (output_txt_path == NULL)
  {
    printf("Error: Memory allocation failed\n");
    free(geo_base);
    free(qry_base);
    return NULL;
  }
  int res = snprintf(output_txt_path, total_len, "%s/%s-%s.txt", output_path,
                     geo_base, qry_base);
  if (res < 0 || (size_t)res >= total_len)
  {
    printf("Error: Path construction failed\n");
    free(output_txt_path);
    free(geo_base);
    free(qry_base);
    return NULL;
  }
  FILE *arquivoTxt = fopen(output_txt_path, "w");
  free(geo_base);
  free(qry_base);
  free(output_txt_path);

  while (!queue_is_empty(getLinesQueue(qryFileData)))
  {
    char *line = (char *)queue_dequeue(getLinesQueue(qryFileData));
    char *command = strtok(line, " \t\r\n");

    if (command == NULL || *command == '\0')
    {
      continue;
    }

    if (strcmp(command, "pd") == 0)
    {
      executar_comando_pd(&atiradores, &atiradoresCount, qry->pilhaParaLiberar);
    }
    else if (strcmp(command, "lc") == 0)
    {
      executar_comando_lc(&carregadores, &carregadoresCount, ground, qry->pilhaParaLiberar,
                          arquivoTxt);
    }
    else if (strcmp(command, "atch") == 0)
    {
      executar_comando_atch(&carregadores, &carregadoresCount, &atiradores, &atiradoresCount,
                            qry->pilhaParaLiberar);
    }
    else if (strcmp(command, "shft") == 0)
    {
      executar_comando_shft(&atiradores, &atiradoresCount, carregadores, &carregadoresCount,
                            arquivoTxt);
    }
    else if (strcmp(command, "dsp") == 0)
    {
      executar_comando_dsp(&atiradores, &atiradoresCount, qry->arena,
                           qry->pilhaParaLiberar, arquivoTxt);
    }
    else if (strcmp(command, "rjd") == 0)
    {
      executar_comando_rjd(&atiradores, &atiradoresCount, qry->pilhaParaLiberar,
                           qry->arena, carregadores, &carregadoresCount, arquivoTxt);
    }
    else if (strcmp(command, "calc") == 0)
    {
      executar_comando_calc(qry->arena, ground, quantidadeDeComandos, arquivoTxt);
    }
    else
      printf("Unknown command: %s\n", command);
  }

  // After processing all commands, emit final SVG with remaining ground shapes
  // and visual annotations derived from the arena
  escrever_svg_resultado_qry(qryFileData, geoFileData, ground, qry->arena,
                             output_path);

  fclose(arquivoTxt);
  return (Qry)qry;
}

/*
==========================
Private functions
==========================
*/

static void executar_comando_pd(Shooter_t **atiradores, int *atiradoresCount,
                                Stack pilhaParaLiberar)
{
  char *identifier = strtok(NULL, " ");
  char *posX = strtok(NULL, " ");
  char *posY = strtok(NULL, " ");

  *atiradoresCount += 1;

  *atiradores = realloc(*atiradores, *atiradoresCount * sizeof(Shooter_t));
  if (*atiradores == NULL)
  {
    printf("Error: Failed to allocate memory for Shooters\n");
    exit(1);
  }

  // Add atiradores array to pilhaParaLiberar for cleanup (update existing or create
  // new)
  bool atiradores_alreadeslocamentoY_tracked = false;
  int stack_size_count = stack_size(pilhaParaLiberar);
  for (int i = 0; i < stack_size_count; i++)
  {
    FreeItem *existing_item = (FreeItem *)stack_peek_at(pilhaParaLiberar, i);
    if (existing_item != NULL && existing_item->type == FREE_SHOOTERS_ARRAY)
    {
      // Update the existing item with the new pointer
      existing_item->ptr = *atiradores;
      atiradores_alreadeslocamentoY_tracked = true;
      break;
    }
  }

  if (!atiradores_alreadeslocamentoY_tracked)
  {
    FreeItem *atiradores_item = malloc(sizeof(FreeItem));
    if (atiradores_item != NULL)
    {
      atiradores_item->ptr = *atiradores;
      atiradores_item->type = FREE_SHOOTERS_ARRAY;
      stack_push(pilhaParaLiberar, atiradores_item);
    }
  }
  (*atiradores)[*atiradoresCount - 1] = (Shooter_t){.id = atoi(identifier),
                                                    .x = atof(posX),
                                                    .y = atof(posY),
                                                    .shootingPosition = NULL,
                                                    .rightLoader = NULL,
                                                    .leftLoader = NULL,
                                                    .rightLoaderId = -1,
                                                    .leftLoaderId = -1};
}

static void executar_comando_lc(Loader_t **carregadores, int *carregadoresCount,
                                Ground ground, Stack pilhaParaLiberar,
                                FILE *arquivoTxt)
{
  char *identifier = strtok(NULL, " ");
  char *firstXShapes = strtok(NULL, " ");

  int idCarregador = atoi(identifier);
  int newShapesCount = atoi(firstXShapes);

  fprintf(arquivoTxt, "COMANDO: lc\n");
  fprintf(arquivoTxt, "  Carregador: %d | Formas: %d\n", idCarregador, newShapesCount);

  // Verificar se loader alreadeslocamentoY exists
  int existingLoaderIndex = -1;
  for (int i = 0; i < *carregadoresCount; i++)
  {
    if ((*carregadores)[i].id == idCarregador)
    {
      existingLoaderIndex = i;
      break;
    }
  }

  if (existingLoaderIndex == -1)
  {
    // Criar novo loader
    *carregadoresCount += 1;
    *carregadores = realloc(*carregadores, *carregadoresCount * sizeof(Loader_t));
    if (*carregadores == NULL)
    {
      printf("Error: Failed to allocate memory for Loaders\n");
      exit(1);
    }

    // Add carregadores array to pilhaParaLiberar for cleanup (update existing or create
    // new)
    bool carregadores_alreadeslocamentoY_tracked = false;
    int stack_size_count = stack_size(pilhaParaLiberar);
    for (int i = 0; i < stack_size_count; i++)
    {
      FreeItem *existing_item = (FreeItem *)stack_peek_at(pilhaParaLiberar, i);
      if (existing_item != NULL && existing_item->type == FREE_LOADERS_ARRAY)
      {
        // Update the existing item with the new pointer
        existing_item->ptr = *carregadores;
        carregadores_alreadeslocamentoY_tracked = true;
        break;
      }
    }

    if (!carregadores_alreadeslocamentoY_tracked)
    {
      FreeItem *carregadores_item = malloc(sizeof(FreeItem));
      if (carregadores_item != NULL)
      {
        carregadores_item->ptr = *carregadores;
        carregadores_item->type = FREE_LOADERS_ARRAY;
        stack_push(pilhaParaLiberar, carregadores_item);
      }
    }
    (*carregadores)[*carregadoresCount - 1] = (Loader_t){.id = idCarregador, .shapes = NULL};
    existingLoaderIndex = *carregadoresCount - 1;
  }

  // Create stack if it doesn't exist
  if ((*carregadores)[existingLoaderIndex].shapes == NULL)
  {
    (*carregadores)[existingLoaderIndex].shapes = malloc(sizeof(Stack));
    if ((*carregadores)[existingLoaderIndex].shapes == NULL)
    {
      printf("Error: Failed to allocate memory for Loader stack\n");
      exit(1);
    }

    // Add shapes stack to pilhaParaLiberar for cleanup
    FreeItem *stack_item = malloc(sizeof(FreeItem));
    if (stack_item != NULL)
    {
      stack_item->ptr = (*carregadores)[existingLoaderIndex].shapes;
      stack_item->type = FREE_STACK_HANDLE;
      stack_push(pilhaParaLiberar, stack_item);
    }
    *(*carregadores)[existingLoaderIndex].shapes = stack_create();
    if (*(*carregadores)[existingLoaderIndex].shapes == NULL)
    {
      printf("Error: Failed to create stack for Loader\n");
      exit(1);
    }
  }

  // Add new shapes to the stack
  for (int i = 0; i < newShapesCount; i++)
  {
    Shape_t *shape = queue_dequeue(get_ground_queue(ground));
    if (shape != NULL)
    {
      if (!stack_push(*(*carregadores)[existingLoaderIndex].shapes, shape))
      {
        printf("Error: Failed to push shape to loader stack\n");
        exit(1);
      }
    }
  }
}

static void executar_comando_atch(Loader_t **carregadores, int *carregadoresCount,
                                  Shooter_t **atiradores, int *atiradoresCount,
                                  Stack pilhaParaLiberar)
{
  char *idAtirador = strtok(NULL, " ");
  char *leftLoaderId = strtok(NULL, " ");
  char *rightLoaderId = strtok(NULL, " ");

  int idAtiradorInt = atoi(idAtirador);
  int leftLoaderIdInt = atoi(leftLoaderId);
  int rightLoaderIdInt = atoi(rightLoaderId);

  int shooterIndex =
      encontrar_indice_atirador_por_id(atiradores, *atiradoresCount, idAtiradorInt);
  if (shooterIndex != -1)
  {
    // Find left and right carregadores by id; create empty ones if not found
    Loader_t *leftLoaderPtr = NULL;
    Loader_t *rightLoaderPtr = NULL;
    for (int j = 0; j < *carregadoresCount; j++)
    {
      if ((*carregadores)[j].id == leftLoaderIdInt)
      {
        leftLoaderPtr = &(*carregadores)[j];
      }
      if ((*carregadores)[j].id == rightLoaderIdInt)
      {
        rightLoaderPtr = &(*carregadores)[j];
      }
    }

    // Create left loader if it does not exist
    if (leftLoaderPtr == NULL)
    {
      *carregadoresCount += 1;
      *carregadores = realloc(*carregadores, *carregadoresCount * sizeof(Loader_t));
      if (*carregadores == NULL)
      {
        printf("Error: Failed to allocate memory for Loaders\n");
        exit(1);
      }
      // Track the (re)allocated carregadores array for later cleanup (update
      // existing or create new)
      bool carregadores_alreadeslocamentoY_tracked = false;
      int stack_size_count = stack_size(pilhaParaLiberar);
      for (int i = 0; i < stack_size_count; i++)
      {
        FreeItem *existing_item = (FreeItem *)stack_peek_at(pilhaParaLiberar, i);
        if (existing_item != NULL &&
            existing_item->type == FREE_LOADERS_ARRAY)
        {
          // Update the existing item with the new pointer
          existing_item->ptr = *carregadores;
          carregadores_alreadeslocamentoY_tracked = true;
          break;
        }
      }

      if (!carregadores_alreadeslocamentoY_tracked)
      {
        FreeItem *carregadores_item = malloc(sizeof(FreeItem));
        if (carregadores_item != NULL)
        {
          carregadores_item->ptr = *carregadores;
          carregadores_item->type = FREE_LOADERS_ARRAY;
          stack_push(pilhaParaLiberar, carregadores_item);
        }
      }
      (*carregadores)[*carregadoresCount - 1] =
          (Loader_t){.id = leftLoaderIdInt, .shapes = NULL};
      // Create empty shapes stack for the new loader
      (*carregadores)[*carregadoresCount - 1].shapes = malloc(sizeof(Stack));
      if ((*carregadores)[*carregadoresCount - 1].shapes == NULL)
      {
        printf("Error: Failed to allocate memory for Loader stack\n");
        exit(1);
      }
      FreeItem *stack_item = malloc(sizeof(FreeItem));
      if (stack_item != NULL)
      {
        stack_item->ptr = (*carregadores)[*carregadoresCount - 1].shapes;
        stack_item->type = FREE_STACK_HANDLE;
        stack_push(pilhaParaLiberar, stack_item);
      }
      *(*carregadores)[*carregadoresCount - 1].shapes = stack_create();
      if (*(*carregadores)[*carregadoresCount - 1].shapes == NULL)
      {
        printf("Error: Failed to create stack for Loader\n");
        exit(1);
      }
      leftLoaderPtr = &(*carregadores)[*carregadoresCount - 1];
    }

    // Create right loader if it does not exist
    if (rightLoaderPtr == NULL)
    {
      *carregadoresCount += 1;
      *carregadores = realloc(*carregadores, *carregadoresCount * sizeof(Loader_t));
      if (*carregadores == NULL)
      {
        printf("Error: Failed to allocate memory for Loaders\n");
        exit(1);
      }
      // Track the (re)allocated carregadores array for later cleanup (update
      // existing or create new)
      bool carregadores_alreadeslocamentoY_tracked = false;
      int stack_size_count = stack_size(pilhaParaLiberar);
      for (int i = 0; i < stack_size_count; i++)
      {
        FreeItem *existing_item = (FreeItem *)stack_peek_at(pilhaParaLiberar, i);
        if (existing_item != NULL &&
            existing_item->type == FREE_LOADERS_ARRAY)
        {
          // Update the existing item with the new pointer
          existing_item->ptr = *carregadores;
          carregadores_alreadeslocamentoY_tracked = true;
          break;
        }
      }

      if (!carregadores_alreadeslocamentoY_tracked)
      {
        FreeItem *carregadores_item = malloc(sizeof(FreeItem));
        if (carregadores_item != NULL)
        {
          carregadores_item->ptr = *carregadores;
          carregadores_item->type = FREE_LOADERS_ARRAY;
          stack_push(pilhaParaLiberar, carregadores_item);
        }
      }
      (*carregadores)[*carregadoresCount - 1] =
          (Loader_t){.id = rightLoaderIdInt, .shapes = NULL};
      // Create empty shapes stack for the new loader
      (*carregadores)[*carregadoresCount - 1].shapes = malloc(sizeof(Stack));
      if ((*carregadores)[*carregadoresCount - 1].shapes == NULL)
      {
        printf("Error: Failed to allocate memory for Loader stack\n");
        exit(1);
      }
      FreeItem *stack_item = malloc(sizeof(FreeItem));
      if (stack_item != NULL)
      {
        stack_item->ptr = (*carregadores)[*carregadoresCount - 1].shapes;
        stack_item->type = FREE_STACK_HANDLE;
        stack_push(pilhaParaLiberar, stack_item);
      }
      *(*carregadores)[*carregadoresCount - 1].shapes = stack_create();
      if (*(*carregadores)[*carregadoresCount - 1].shapes == NULL)
      {
        printf("Error: Failed to create stack for Loader\n");
        exit(1);
      }
      rightLoaderPtr = &(*carregadores)[*carregadoresCount - 1];
    }

    (*atiradores)[shooterIndex].leftLoader = leftLoaderPtr;
    (*atiradores)[shooterIndex].rightLoader = rightLoaderPtr;
    (*atiradores)[shooterIndex].leftLoaderId = leftLoaderIdInt;
    (*atiradores)[shooterIndex].rightLoaderId = rightLoaderIdInt;
  }
  else
  {
    printf("Error: Shooter with ID %d not found\n", idAtiradorInt);
  }
}

static void executar_operacao_deslocamento(Shooter_t **atiradores, int atiradoresCount,
                                           int idAtirador, const char *direcao,
                                           int vezes, Loader_t *carregadores,
                                           int carregadoresCount)
{
  int shooterIndex =
      encontrar_indice_atirador_por_id(atiradores, atiradoresCount, idAtirador);
  if (shooterIndex == -1)
  {
    printf("Error: Shooter with ID %d not found\n", idAtirador);
    return;
  }

  Shooter_t *shooter = &(*atiradores)[shooterIndex];

  // Resolve current loader pointers from stored IDs (rebinding after reallocs)
  Loader_t *resolvedLeft = NULL;
  Loader_t *resolvedRight = NULL;
  if (shooter->leftLoaderId != -1)
  {
    for (int i = 0; i < carregadoresCount; i++)
    {
      if (carregadores[i].id == shooter->leftLoaderId)
      {
        resolvedLeft = &carregadores[i];
        break;
      }
    }
  }
  if (shooter->rightLoaderId != -1)
  {
    for (int i = 0; i < carregadoresCount; i++)
    {
      if (carregadores[i].id == shooter->rightLoaderId)
      {
        resolvedRight = &carregadores[i];
        break;
      }
    }
  }
  shooter->leftLoader = resolvedLeft;
  shooter->rightLoader = resolvedRight;

  for (int i = 0; i < vezes; i++)
  {
    if (strcmp(direcao, "e") == 0)
    {
      // Verificar se left loader exists and has shapes
      if (shooter->leftLoader == NULL ||
          stack_is_empty(*(shooter->leftLoader->shapes)))
      {
        continue; // Skip silently if no shapes available
      }

      // If shooter has a shape, move it to right loader
      if (shooter->shootingPosition != NULL && shooter->rightLoader != NULL)
      {
        stack_push(*(shooter->rightLoader->shapes), shooter->shootingPosition);
      }

      shooter->shootingPosition = stack_pop(*(shooter->leftLoader->shapes));
    }
    if (strcmp(direcao, "d") == 0)
    {
      // Verificar se right loader exists and has shapes
      if (shooter->rightLoader == NULL ||
          stack_is_empty(*(shooter->rightLoader->shapes)))
      {
        continue; // Skip silently if no shapes available
      }

      // If shooter has a shape, move it to left loader
      if (shooter->shootingPosition != NULL && shooter->leftLoader != NULL)
      {
        stack_push(*(shooter->leftLoader->shapes), shooter->shootingPosition);
      }

      shooter->shootingPosition = stack_pop(*(shooter->rightLoader->shapes));
    }
  }
}

static void executar_comando_shft(Shooter_t **atiradores, int *atiradoresCount,
                                  Loader_t *carregadores, int *carregadoresCount,
                                  FILE *arquivoTxt)
{
  char *idAtirador = strtok(NULL, " ");
  char *botaoEsquerdoOuDireito = strtok(NULL, " ");
  char *vezesPressionado = strtok(NULL, " ");
  fprintf(arquivoTxt, "COMANDO: shft\n");

  int idAtiradorInt = atoi(idAtirador);
  int vezesPressionadoInt = atoi(vezesPressionado);

  fprintf(arquivoTxt, "  Atirador: %d | Botão: %s | Pressionado: %d\n",
          idAtiradorInt, botaoEsquerdoOuDireito, vezesPressionadoInt);

  executar_operacao_deslocamento(atiradores, *atiradoresCount, idAtiradorInt,
                                 botaoEsquerdoOuDireito, vezesPressionadoInt, carregadores,
                                 *carregadoresCount);
}

static void executar_operacao_disparo(Shooter_t **atiradores, int atiradoresCount,
                                      int idAtirador, double deslocamentoX, double deslocamentoY,
                                      const char *anotar, Stack arena,
                                      Stack pilhaParaLiberar)
{
  int shooterIndex =
      encontrar_indice_atirador_por_id(atiradores, atiradoresCount, idAtirador);
  if (shooterIndex == -1)
  {
    printf("Error: Shooter with ID %d not found\n", idAtirador);
    return;
  }

  Shooter_t *shooter = &(*atiradores)[shooterIndex];

  // Verificar se shooter has a shape to shoot
  if (shooter->shootingPosition == NULL)
  {
    return; // Skip silently if no shape to shoot
  }

  double shapeXOnArena = shooter->x + deslocamentoX;
  double shapeYOnArena = shooter->y + deslocamentoY;

  Shape_t *shape = (Shape_t *)shooter->shootingPosition;
  TipoForma shapeType = shape->type;

  // Add shape to arena
  ShapePositionOnArena_t *shapePositionOnArena =
      malloc(sizeof(ShapePositionOnArena_t));
  if (shapePositionOnArena == NULL)
  {
    printf("Error: Failed to allocate memory for ShapePositionOnArena\n");
    exit(1);
  }
  shapePositionOnArena->shape = shape;
  shapePositionOnArena->x = shapeXOnArena;
  shapePositionOnArena->y = shapeYOnArena;
  shapePositionOnArena->isAnnotated = strcmp(anotar, "v") == 0;
  shapePositionOnArena->shooterX = shooter->x;
  shapePositionOnArena->shooterY = shooter->y;

  // Clear shooter shooting position
  shooter->shootingPosition = NULL;

  stack_push(arena, (void *)shapePositionOnArena);
  FreeItem *shape_item = malloc(sizeof(FreeItem));
  if (shape_item != NULL)
  {
    shape_item->ptr = shapePositionOnArena;
    shape_item->type = FREE_SHAPE_POSITION;
    stack_push(pilhaParaLiberar, shape_item);
  }
}

static void executar_comando_dsp(Shooter_t **atiradores, int *atiradoresCount,
                                 Stack arena, Stack pilhaParaLiberar, FILE *arquivoTxt)
{
  char *idAtirador = strtok(NULL, " ");
  char *deslocamentoX = strtok(NULL, " ");
  char *deslocamentoY = strtok(NULL, " ");
  char *anotarDimensoes = strtok(NULL, " "); // this can be "v" or "i"

  int idAtiradorInt = atoi(idAtirador);
  double deslocamentoXDouble = atof(deslocamentoX);
  double deslocamentoYDouble = atof(deslocamentoY);

  fprintf(arquivoTxt, "COMANDO: dsp\n");
  fprintf(arquivoTxt, "  Atirador: %d | DX: %.2f | DY: %.2f\n",
          idAtiradorInt, deslocamentoXDouble, deslocamentoYDouble);
  fprintf(arquivoTxt, "  Anotar: %s\n", anotarDimensoes);

  executar_operacao_disparo(atiradores, *atiradoresCount, idAtiradorInt, deslocamentoXDouble,
                            deslocamentoYDouble, anotarDimensoes, arena, pilhaParaLiberar);
}

static void executar_comando_rjd(Shooter_t **atiradores, int *atiradoresCount,
                                 Stack pilhaParaLiberar, Stack arena,
                                 Loader_t *carregadores, int *carregadoresCount,
                                 FILE *arquivoTxt)
{
  char *idAtirador = strtok(NULL, " ");
  char *botaoEsquerdoOuDireito = strtok(NULL, " ");
  char *deslocamentoX = strtok(NULL, " ");
  char *deslocamentoY = strtok(NULL, " ");
  char *incrementoX = strtok(NULL, " ");
  char *incrementoY = strtok(NULL, " ");

  int idAtiradorInt = atoi(idAtirador);
  double deslocamentoXDouble = atof(deslocamentoX);
  double deslocamentoYDouble = atof(deslocamentoY);
  double incrementoXDouble = atof(incrementoX);
  double incrementoYDouble = atof(incrementoY);

  int shooterIndex =
      encontrar_indice_atirador_por_id(atiradores, *atiradoresCount, idAtiradorInt);
  if (shooterIndex == -1)
  {
    printf("Error: Shooter with ID %d not found\n", idAtiradorInt);
    return;
  }

  Shooter_t *shooter = &(*atiradores)[shooterIndex];
  Loader_t *loader = NULL;
  // Rebind current pointers based on IDs in case carregadores was reallocated
  if (strcmp(botaoEsquerdoOuDireito, "e") == 0)
  {
    // left side
    int targetId = (*atiradores)[shooterIndex].leftLoaderId;
    if (targetId != -1)
    {
      for (int i = 0; i < *carregadoresCount; i++)
      {
        if (carregadores[i].id == targetId)
        {
          (*atiradores)[shooterIndex].leftLoader = &carregadores[i];
          break;
        }
      }
    }
  }
  else if (strcmp(botaoEsquerdoOuDireito, "d") == 0)
  {
    int targetId = (*atiradores)[shooterIndex].rightLoaderId;
    if (targetId != -1)
    {
      for (int i = 0; i < *carregadoresCount; i++)
      {
        if (carregadores[i].id == targetId)
        {
          (*atiradores)[shooterIndex].rightLoader = &carregadores[i];
          break;
        }
      }
    }
  }
  // Select the same side that executar_operacao_deslocamento will consume from
  if (strcmp(botaoEsquerdoOuDireito, "e") == 0)
  {
    loader = shooter->leftLoader;
  }
  else if (strcmp(botaoEsquerdoOuDireito, "d") == 0)
  {
    loader = shooter->rightLoader;
  }
  else
  {
    printf("Error: Invalid button (should be 'e' or 'd')\n");
    return;
  }

  if (loader == NULL || loader->shapes == NULL)
  {
    printf("Error: Loader not found or shapes stack is NULL\n");
    return;
  }

  int vezes = 1;

  fprintf(arquivoTxt, "COMANDO: rjd\n");
  fprintf(arquivoTxt, "  Atirador: %d | Botão: %s\n",
          idAtiradorInt, botaoEsquerdoOuDireito);
  fprintf(arquivoTxt, "  DX: %.2f | DY: %.2f\n",
          deslocamentoXDouble, deslocamentoYDouble);
  fprintf(arquivoTxt, "  IncX: %.2f | IncY: %.2f\n",
          incrementoXDouble, incrementoYDouble);

  // Loop until loader is empty
  while (!stack_is_empty(*(loader->shapes)))
  {
    executar_operacao_deslocamento(atiradores, *atiradoresCount, idAtiradorInt,
                                   botaoEsquerdoOuDireito, 1, carregadores, *carregadoresCount);
    executar_operacao_disparo(atiradores, *atiradoresCount, idAtiradorInt,
                              vezes * incrementoXDouble + deslocamentoXDouble,
                              vezes * incrementoYDouble + deslocamentoYDouble, "i", arena,
                              pilhaParaLiberar);
    vezes++;
  }
}

void executar_comando_calc(Stack arena, Ground ground, int quantidadeDeComandos, FILE *arquivoTxt)
{
  // We need to process in launch order (oldest to newest). Arena is a stack
  // (LIFO), so first reverse into a temporary stack to get FIFO order when
  // popping.
  Stack temp = stack_create();
  while (!stack_is_empty(arena))
  {
    stack_push(temp, stack_pop(arena));
  }

  // Accumulate crushed area only for overlapping pairs (min area per pair)
  double area_esmagada_total = 0.0;

  // Now process adjacent pairs I (older) and J (I+1 newer).
  while (!stack_is_empty(temp))
  {
    ShapePositionOnArena_t *I = (ShapePositionOnArena_t *)stack_pop(temp);
    if (stack_is_empty(temp))
    {
      // No pair for I, return to ground at its arena position
      Shape_t *Ipos = clonar_com_posicao(I->shape, I->x, I->y, ground);
      if (Ipos != NULL)
      {
        queue_enqueue(get_ground_queue(ground), Ipos);
      }
      continue;
    }
    ShapePositionOnArena_t *J = (ShapePositionOnArena_t *)stack_pop(temp);

    bool overlap = formas_sobrepoem(I, J);
    if (overlap)
    {
      double areaI = calcular_area_forma(I->shape->type, I->shape->data);
      double areaJ = calcular_area_forma(J->shape->type, J->shape->data);
      // Add only the crushed area for this overlapping pair
      area_esmagada_total += (areaI < areaJ) ? areaI : areaJ;

      if (areaI < areaJ)
      {
        // I is destroyed; J goes back to ground at its arena position
        Shape_t *Jpos = clonar_com_posicao(J->shape, J->x, J->y, ground);
        if (Jpos != NULL)
        {
          queue_enqueue(get_ground_queue(ground), Jpos);
        }
      }
      else if (areaI >= areaJ)
      {
        // I changes border color of J to fill color of I, if applicable
        const char *fillColorI = NULL;
        switch (I->shape->type)
        {
        case CIRCLE:
          fillColorI = circulo_get_cor_preenchimento((Circulo)I->shape->data);
          break;
        case RECTANGLE:
          fillColorI = retangulo_get_cor_preenchimento((Retangulo)I->shape->data);
          break;
        case TEXT:
          fillColorI = text_get_fill_color((Text)I->shape->data);
          break;
        case LINE:
        case TEXT_STYLE:
          fillColorI = NULL;
          break;
        }

        // Prepare J' with new border and positioned at J
        Shape_t *JprimePos = NULL;
        if (fillColorI != NULL)
        {
          JprimePos = clonar_com_cor_borda_na_posicao(J->shape, fillColorI,
                                                      J->x, J->y, ground);
        }
        else
        {
          JprimePos = clonar_com_posicao(J->shape, J->x, J->y, ground);
        }

        // Both return to ground in original relative order (I, then J') at
        // their positions
        Shape_t *Ipos = clonar_com_posicao(I->shape, I->x, I->y, ground);
        if (Ipos != NULL)
        {
          queue_enqueue(get_ground_queue(ground), Ipos);
        }
        if (JprimePos != NULL)
        {
          queue_enqueue(get_ground_queue(ground), JprimePos);
        }

        // Clone I swapping border and fill (only if applicable), at I position
        Shape_t *IclonePos =
            clonar_com_cores_trocadas_na_posicao(I->shape, I->x, I->y, ground);
        if (IclonePos != NULL)
        {
          queue_enqueue(get_ground_queue(ground), IclonePos);
        }
      }
      else
      {
        // Equal areas: both return unchanged at their positions
        Shape_t *Ipos = clonar_com_posicao(I->shape, I->x, I->y, ground);
        Shape_t *Jpos = clonar_com_posicao(J->shape, J->x, J->y, ground);
        if (Ipos != NULL)
        {
          queue_enqueue(get_ground_queue(ground), Ipos);
        }
        if (Jpos != NULL)
        {
          queue_enqueue(get_ground_queue(ground), Jpos);
        }
      }
    }
    else
    {
      // No overlap: both return unchanged in the same relative order, placed at
      // their positions
      Shape_t *Ipos = clonar_com_posicao(I->shape, I->x, I->y, ground);
      Shape_t *Jpos = clonar_com_posicao(J->shape, J->x, J->y, ground);
      if (Ipos != NULL)
      {
        queue_enqueue(get_ground_queue(ground), Ipos);
      }
      if (Jpos != NULL)
      {
        queue_enqueue(get_ground_queue(ground), Jpos);
      }
    }
  }

  // Exibir o calculated result
  fprintf(arquivoTxt, "COMANDO: calc\n");
  fprintf(arquivoTxt, "  Área esmagada: %.2lf\n", area_esmagada_total);
  fprintf(arquivoTxt, "  Quantidade de comandos: %d\n", quantidadeDeComandos);
  // Limpar temporary stack
  stack_destroy(temp);
}

static int encontrar_indice_atirador_por_id(Shooter_t **atiradores, int atiradoresCount,
                                            int id)
{
  for (int i = 0; i < atiradoresCount; i++)
  {
    if ((*atiradores)[i].id == id)
    {
      return i;
    }
  }
  return -1;
}

// =====================
// Helpers implementation
// =====================

static Shape_t *criar_wrapper_forma(TipoForma type, void *data)
{
  Shape_t *s = (Shape_t *)malloc(sizeof(Shape_t));
  if (s == NULL)
  {
    printf("Error: Failed to allocate shape wrapper\n");
    exit(1);
  }
  s->type = type;
  s->data = data;
  return s;
}

static double calcular_area_forma(TipoForma type, void *dadosForma)
{
  switch (type)
  {
  case CIRCLE:
  {
    double r = circulo_get_raio((Circulo)dadosForma);
    return 3.141592653589793 * r * r;
  }
  case RECTANGLE:
  {
    double w = retangulo_get_largura((Retangulo)dadosForma);
    double h = retangulo_get_altura((Retangulo)dadosForma);
    return w * h;
  }
  case LINE:
  {
    double x1 = line_get_x1((Line)dadosForma);
    double y1 = line_get_y1((Line)dadosForma);
    double x2 = line_get_x2((Line)dadosForma);
    double y2 = line_get_y2((Line)dadosForma);
    double deslocamentoX = x2 - x1;
    double deslocamentoY = y2 - y1;
    double len = (deslocamentoX * deslocamentoX + deslocamentoY * deslocamentoY) > 0.0 ? sqrt(deslocamentoX * deslocamentoX + deslocamentoY * deslocamentoY) : 0.0;
    return 2.0 * len;
  }
  case TEXT:
  {
    const char *txt = text_get_text((Text)dadosForma);
    int len = (int)strlen(txt);
    return 20.0 * (double)len;
  }
  case TEXT_STYLE:
    return 0.0;
  }
  return 0.0;
}

static Aabb criar_aabb_para_forma_na_arena(const ShapePositionOnArena_t *s)
{
  Aabb box;
  switch (s->shape->type)
  {
  case CIRCLE:
  {
    double r = circulo_get_raio((Circulo)s->shape->data);
    box.minX = s->x - r;
    box.maxX = s->x + r;
    box.minY = s->y - r;
    box.maxY = s->y + r;
    break;
  }
  case RECTANGLE:
  {
    double w = retangulo_get_largura((Retangulo)s->shape->data);
    double h = retangulo_get_altura((Retangulo)s->shape->data);
    box.minX = s->x;
    box.minY = s->y;
    box.maxX = s->x + w;
    box.maxY = s->y + h;
    break;
  }
  case TEXT:
  {
    // Treat text as a horizontal segment based on anchor, with length 10.0 *
    // |t|
    Text t = (Text)s->shape->data;
    const char *txt = text_get_text(t);
    int len = (int)strlen(txt);
    double segLen = 10.0 * (double)len;
    char anchor = text_get_anchor(t);
    double x1 = s->x;
    double y1 = s->y;
    double x2 = s->x;
    double y2 = s->y;
    if (anchor == 'i' || anchor == 'I')
    {
      // start anchor to the right
      x2 = s->x + segLen;
      y2 = s->y;
    }
    else if (anchor == 'f' || anchor == 'F' || anchor == 'e' ||
             anchor == 'E')
    {
      // end anchor to the left (Portuguese 'f'inal / 'e'nd)
      x1 = s->x - segLen;
      y1 = s->y;
    }
    else if (anchor == 'm' || anchor == 'M')
    {
      // middle anchor centered at s->x
      x1 = s->x - segLen * 0.5;
      y1 = s->y;
      x2 = s->x + segLen * 0.5;
      y2 = s->y;
    }
    else
    {
      // default to start ('i') if unknown
      x2 = s->x + segLen;
      y2 = s->y;
    }

    double deslocamentoX = x2 - x1;
    double deslocamentoY = y2 - y1; // will be 0.0 for horizontal segment
    double minLocalX = (deslocamentoX < 0.0) ? deslocamentoX : 0.0;
    double maxLocalX = (deslocamentoX > 0.0) ? deslocamentoX : 0.0;
    double minLocalY = (deslocamentoY < 0.0) ? deslocamentoY : 0.0;
    double maxLocalY = (deslocamentoY > 0.0) ? deslocamentoY : 0.0;
    // thickness 2.0 => inflate by 1 on each side (same rule as LINE)
    box.minX = x1 + minLocalX - 1.0;
    box.maxX = x1 + maxLocalX + 1.0;
    box.minY = y1 + minLocalY - 1.0;
    box.maxY = y1 + maxLocalY + 1.0;
    break;
  }
  case LINE:
  {
    double x1 = line_get_x1((Line)s->shape->data);
    double y1 = line_get_y1((Line)s->shape->data);
    double x2 = line_get_x2((Line)s->shape->data);
    double y2 = line_get_y2((Line)s->shape->data);
    double deslocamentoX = x2 - x1;
    double deslocamentoY = y2 - y1;
    double minLocalX = (deslocamentoX < 0.0) ? deslocamentoX : 0.0;
    double maxLocalX = (deslocamentoX > 0.0) ? deslocamentoX : 0.0;
    double minLocalY = (deslocamentoY < 0.0) ? deslocamentoY : 0.0;
    double maxLocalY = (deslocamentoY > 0.0) ? deslocamentoY : 0.0;
    // thickness 2.0 => inflate by 1 on each side
    box.minX = s->x + minLocalX - 1.0;
    box.maxX = s->x + maxLocalX + 1.0;
    box.minY = s->y + minLocalY - 1.0;
    box.maxY = s->y + maxLocalY + 1.0;
    break;
  }
  case TEXT_STYLE:
  {
    // No extent, treat as empty box
    box.minX = box.maxX = s->x;
    box.minY = box.maxY = s->y;
    break;
  }
  }
  return box;
}

static bool aabb_sobrepoe(Aabb a, Aabb b)
{
  if (a.maxX < b.minX)
    return false;
  if (b.maxX < a.minX)
    return false;
  if (a.maxY < b.minY)
    return false;
  if (b.maxY < a.minY)
    return false;
  return true;
}

static bool formas_sobrepoem(const ShapePositionOnArena_t *a,
                             const ShapePositionOnArena_t *b)
{
  Aabb aa = criar_aabb_para_forma_na_arena(a);
  Aabb bb = criar_aabb_para_forma_na_arena(b);
  return aabb_sobrepoe(aa, bb);
}

static Shape_t *clonar_com_cor_borda(Shape_t *src,
                                     const char *novaCorBorda)
{
  switch (src->type)
  {
  case CIRCLE:
  {
    Circulo c = (Circulo)src->data;
    int id = circulo_get_id(c);
    double x = 0.0; // position handled by arena, keep model values
    double y = 0.0;
    // Preserve original geometry from getters
    x = circulo_get_x(c);
    y = circulo_get_y(c);
    double r = circulo_get_raio(c);
    const char *fill = circulo_get_cor_preenchimento(c);
    Circulo nc = circulo_criar(id, x, y, r, novaCorBorda, fill);
    return criar_wrapper_forma(CIRCLE, nc);
  }
  case RECTANGLE:
  {
    Retangulo r = (Retangulo)src->data;
    int id = retangulo_get_id(r);
    double x = retangulo_get_x(r);
    double y = retangulo_get_y(r);
    double w = retangulo_get_largura(r);
    double h = retangulo_get_altura(r);
    const char *fill = retangulo_get_cor_preenchimento(r);
    Retangulo nr = retangulo_criar(id, x, y, w, h, novaCorBorda, fill);
    return criar_wrapper_forma(RECTANGLE, nr);
  }
  case TEXT:
  {
    Text t = (Text)src->data;
    int id = text_get_id(t);
    double x = text_get_x(t);
    double y = text_get_y(t);
    const char *fill = text_get_fill_color(t);
    char anchor = text_get_anchor(t);
    const char *txt = text_get_text(t);
    Text nt = text_create(id, x, y, novaCorBorda, fill, anchor, txt);
    return criar_wrapper_forma(TEXT, nt);
  }
  case LINE:
  {
    Line l = (Line)src->data;
    int id = line_get_id(l);
    double x1 = line_get_x1(l);
    double y1 = line_get_y1(l);
    double x2 = line_get_x2(l);
    double y2 = line_get_y2(l);
    Line nl = line_create(id, x1, y1, x2, y2, novaCorBorda);
    return criar_wrapper_forma(LINE, nl);
  }
  case TEXT_STYLE:
    return NULL;
  }
  return NULL;
}

static Shape_t *clonar_com_cores_trocadas(Shape_t *src)
{
  switch (src->type)
  {
  case CIRCLE:
  {
    Circulo c = (Circulo)src->data;
    int id = circulo_get_id(c);
    double x = circulo_get_x(c);
    double y = circulo_get_y(c);
    double r = circulo_get_raio(c);
    const char *border = circulo_get_cor_borda(c);
    const char *fill = circulo_get_cor_preenchimento(c);
    Circulo nc = circulo_criar(id, x, y, r, fill, border);
    return criar_wrapper_forma(CIRCLE, nc);
  }
  case RECTANGLE:
  {
    Retangulo r = (Retangulo)src->data;
    int id = retangulo_get_id(r);
    double x = retangulo_get_x(r);
    double y = retangulo_get_y(r);
    double w = retangulo_get_largura(r);
    double h = retangulo_get_altura(r);
    const char *border = retangulo_get_cor_borda(r);
    const char *fill = retangulo_get_cor_preenchimento(r);
    Retangulo nr = retangulo_criar(id, x, y, w, h, fill, border);
    return criar_wrapper_forma(RECTANGLE, nr);
  }
  case TEXT:
  {
    Text t = (Text)src->data;
    int id = text_get_id(t);
    double x = text_get_x(t);
    double y = text_get_y(t);
    const char *border = text_get_border_color(t);
    const char *fill = text_get_fill_color(t);
    char anchor = text_get_anchor(t);
    const char *txt = text_get_text(t);
    Text nt = text_create(id, x, y, fill, border, anchor, txt);
    return criar_wrapper_forma(TEXT, nt);
  }
  case LINE:
  {
    Line l = (Line)src->data;
    int id = line_get_id(l);
    double x1 = line_get_x1(l);
    double y1 = line_get_y1(l);
    double x2 = line_get_x2(l);
    double y2 = line_get_y2(l);
    const char *c = line_get_color(l);
    char *inv = invert_color(c);
    if (inv == NULL)
      return NULL;
    Line nl = line_create(id, x1, y1, x2, y2, inv);
    free(inv);
    return criar_wrapper_forma(LINE, nl);
  }
  case TEXT_STYLE:
    // No color to invert on style descriptor
    return NULL;
  }
  return NULL;
}

// =====================
// Positioning helpers
// =====================

static void destruir_forma(Shape_t *s)
{
  if (s == NULL)
    return;
  switch (s->type)
  {
  case CIRCLE:
    circulo_destruir((Circulo)s->data);
    break;
  case RECTANGLE:
    retangulo_destruir((Retangulo)s->data);
    break;
  case LINE:
    line_destroy((Line)s->data);
    break;
  case TEXT:
    text_destroy((Text)s->data);
    break;
  case TEXT_STYLE:
    // nothing allocated
    break;
  }
  free(s);
}

static Shape_t *clonar_com_posicao(Shape_t *src, double x, double y,
                                   Ground ground)
{
  if (src == NULL)
    return NULL;
  Shape_t *cloned = NULL;
  switch (src->type)
  {
  case CIRCLE:
  {
    Circulo c = (Circulo)src->data;
    int id = circulo_get_id(c);
    double r = circulo_get_raio(c);
    const char *border = circulo_get_cor_borda(c);
    const char *fill = circulo_get_cor_preenchimento(c);
    Circulo nc = circulo_criar(id, x, y, r, border, fill);
    cloned = criar_wrapper_forma(CIRCLE, nc);
    break;
  }
  case RECTANGLE:
  {
    Retangulo r = (Retangulo)src->data;
    int id = retangulo_get_id(r);
    double w = retangulo_get_largura(r);
    double h = retangulo_get_altura(r);
    const char *border = retangulo_get_cor_borda(r);
    const char *fill = retangulo_get_cor_preenchimento(r);
    Retangulo nr = retangulo_criar(id, x, y, w, h, border, fill);
    cloned = criar_wrapper_forma(RECTANGLE, nr);
    break;
  }
  case TEXT:
  {
    Text t = (Text)src->data;
    int id = text_get_id(t);
    const char *border = text_get_border_color(t);
    const char *fill = text_get_fill_color(t);
    char anchor = text_get_anchor(t);
    const char *txt = text_get_text(t);
    Text nt = text_create(id, x, y, border, fill, anchor, txt);
    cloned = criar_wrapper_forma(TEXT, nt);
    break;
  }
  case LINE:
  {
    Line l = (Line)src->data;
    int id = line_get_id(l);
    double deslocamentoX = line_get_x2(l) - line_get_x1(l);
    double deslocamentoY = line_get_y2(l) - line_get_y1(l);
    Line nl = line_create(id, x, y, x + deslocamentoX, y + deslocamentoY, line_get_color(l));
    cloned = criar_wrapper_forma(LINE, nl);
    break;
  }
  case TEXT_STYLE:
    return NULL;
  }

  // Add cloned shape to shapesStackToFree for proper cleanup
  if (cloned != NULL && ground != NULL)
  {
    stack_push(get_ground_shapes_stack_to_free(ground), cloned);
  }

  return cloned;
}

static Shape_t *clonar_com_cor_borda_na_posicao(Shape_t *src,
                                                const char *novaCorBorda,
                                                double x, double y,
                                                Ground ground)
{
  if (src == NULL)
    return NULL;
  Shape_t *cloned = NULL;
  switch (src->type)
  {
  case CIRCLE:
  {
    Circulo c = (Circulo)src->data;
    int id = circulo_get_id(c);
    double r = circulo_get_raio(c);
    const char *fill = circulo_get_cor_preenchimento(c);
    Circulo nc = circulo_criar(id, x, y, r, novaCorBorda, fill);
    cloned = criar_wrapper_forma(CIRCLE, nc);
    break;
  }
  case RECTANGLE:
  {
    Retangulo r = (Retangulo)src->data;
    int id = retangulo_get_id(r);
    double w = retangulo_get_largura(r);
    double h = retangulo_get_altura(r);
    const char *fill = retangulo_get_cor_preenchimento(r);
    Retangulo nr = retangulo_criar(id, x, y, w, h, novaCorBorda, fill);
    cloned = criar_wrapper_forma(RECTANGLE, nr);
    break;
  }
  case TEXT:
  {
    Text t = (Text)src->data;
    int id = text_get_id(t);
    const char *fill = text_get_fill_color(t);
    char anchor = text_get_anchor(t);
    const char *txt = text_get_text(t);
    Text nt = text_create(id, x, y, novaCorBorda, fill, anchor, txt);
    cloned = criar_wrapper_forma(TEXT, nt);
    break;
  }
  case LINE:
  {
    Line l = (Line)src->data;
    int id = line_get_id(l);
    double deslocamentoX = line_get_x2(l) - line_get_x1(l);
    double deslocamentoY = line_get_y2(l) - line_get_y1(l);
    Line nl = line_create(id, x, y, x + deslocamentoX, y + deslocamentoY, novaCorBorda);
    cloned = criar_wrapper_forma(LINE, nl);
    break;
  }
  case TEXT_STYLE:
    return NULL;
  }

  // Add cloned shape to shapesStackToFree for proper cleanup
  if (cloned != NULL && ground != NULL)
  {
    stack_push(get_ground_shapes_stack_to_free(ground), cloned);
  }

  return cloned;
}

static Shape_t *clonar_com_cores_trocadas_na_posicao(Shape_t *src, double x,
                                                     double y, Ground ground)
{
  if (src == NULL)
    return NULL;
  Shape_t *cloned = NULL;
  switch (src->type)
  {
  case CIRCLE:
  {
    Circulo c = (Circulo)src->data;
    int id = circulo_get_id(c);
    double r = circulo_get_raio(c);
    const char *border = circulo_get_cor_borda(c);
    const char *fill = circulo_get_cor_preenchimento(c);
    Circulo nc = circulo_criar(id, x, y, r, fill, border);
    cloned = criar_wrapper_forma(CIRCLE, nc);
    break;
  }
  case RECTANGLE:
  {
    Retangulo r = (Retangulo)src->data;
    int id = retangulo_get_id(r);
    double w = retangulo_get_largura(r);
    double h = retangulo_get_altura(r);
    const char *border = retangulo_get_cor_borda(r);
    const char *fill = retangulo_get_cor_preenchimento(r);
    Retangulo nr = retangulo_criar(id, x, y, w, h, fill, border);
    cloned = criar_wrapper_forma(RECTANGLE, nr);
    break;
  }
  case TEXT:
  {
    Text t = (Text)src->data;
    int id = text_get_id(t);
    const char *border = text_get_border_color(t);
    const char *fill = text_get_fill_color(t);
    char anchor = text_get_anchor(t);
    const char *txt = text_get_text(t);
    Text nt = text_create(id, x, y, fill, border, anchor, txt);
    cloned = criar_wrapper_forma(TEXT, nt);
    break;
  }
  case LINE:
  {
    Line l = (Line)src->data;
    int id = line_get_id(l);
    double deslocamentoX = line_get_x2(l) - line_get_x1(l);
    double deslocamentoY = line_get_y2(l) - line_get_y1(l);
    const char *c = line_get_color(l);
    char *inv = invert_color(c);
    if (inv == NULL)
      return NULL;
    Line nl = line_create(id, x, y, x + deslocamentoX, y + deslocamentoY, inv);
    free(inv);
    cloned = criar_wrapper_forma(LINE, nl);
    break;
  }
  case TEXT_STYLE:
    return NULL;
  }

  // Add cloned shape to shapesStackToFree for proper cleanup
  if (cloned != NULL && ground != NULL)
  {
    stack_push(get_ground_shapes_stack_to_free(ground), cloned);
  }

  return cloned;
}

// =====================
// SVG writer implementation
// =====================
static void escrever_svg_resultado_qry(FileData qryFileData, FileData geoFileData,
                                       Ground ground, Stack arena,
                                       const char *output_path)
{
  const char *geo_name_src = getFileName(geoFileData);
  const char *qry_name_src = getFileName(qryFileData);
  size_t geo_len = strlen(geo_name_src);
  size_t qry_len = strlen(qry_name_src);

  char *geo_base = malloc(geo_len + 1);
  char *qry_base = malloc(qry_len + 1);
  if (geo_base == NULL || qry_base == NULL)
  {
    printf("Error: Memory allocation failed for file name\n");
    free(geo_base);
    free(qry_base);
    return;
  }
  strcpy(geo_base, geo_name_src);
  strcpy(qry_base, qry_name_src);
  strtok(geo_base, ".");
  strtok(qry_base, ".");

  // geoBase-qryBase.svg
  size_t path_len = strlen(output_path);
  size_t processed_name_len = strlen(geo_base) + 1 + strlen(qry_base);
  size_t total_len = path_len + 1 + processed_name_len + 4 + 1;
  char *output_path_with_file = malloc(total_len);
  if (output_path_with_file == NULL)
  {
    printf("Error: Memory allocation failed\n");
    free(geo_base);
    free(qry_base);
    return;
  }

  int result = snprintf(output_path_with_file, total_len, "%s/%s-%s.svg",
                        output_path, geo_base, qry_base);
  if (result < 0 || (size_t)result >= total_len)
  {
    printf("Error: Path construction failed\n");
    free(output_path_with_file);
    free(geo_base);
    free(qry_base);
    return;
  }

  FILE *file = fopen(output_path_with_file, "w");
  if (file == NULL)
  {
    printf("Error: Failed to open file: %s\n", output_path_with_file);
    free(output_path_with_file);
    free(geo_base);
    free(qry_base);
    return;
  }

  fprintf(file, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
  fprintf(file, "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 1000 "
                "1000\">\n");

  // Render remaining shapes from Ground without destroying the queue
  Queue groundQueue = get_ground_queue(ground);
  Queue tempQueue = queue_create();
  while (!queue_is_empty(groundQueue))
  {
    Shape_t *shape = (Shape_t *)queue_dequeue(groundQueue);
    if (shape != NULL)
    {
      if (shape->type == CIRCLE)
      {
        Circulo circle = (Circulo)shape->data;
        fprintf(
            file,
            "<circle cx='%.2f' cy='%.2f' r='%.2f' fill='%s' stroke='%s'/>\n",
            circulo_get_x(circle), circulo_get_y(circle),
            circulo_get_raio(circle), circulo_get_cor_preenchimento(circle),
            circulo_get_cor_borda(circle));
      }
      else if (shape->type == RECTANGLE)
      {
        Retangulo rectangle = (Retangulo)shape->data;
        fprintf(file,
                "<rect x='%.2f' y='%.2f' width='%.2f' height='%.2f' fill='%s' "
                "stroke='%s'/>\n",
                retangulo_get_x(rectangle), retangulo_get_y(rectangle),
                retangulo_get_largura(rectangle), retangulo_get_altura(rectangle),
                retangulo_get_cor_preenchimento(rectangle),
                retangulo_get_cor_borda(rectangle));
      }
      else if (shape->type == LINE)
      {
        Line line = (Line)shape->data;
        fprintf(file,
                "<line x1='%.2f' y1='%.2f' x2='%.2f' y2='%.2f' stroke='%s'/>\n",
                line_get_x1(line), line_get_y1(line), line_get_x2(line),
                line_get_y2(line), line_get_color(line));
      }
      else if (shape->type == TEXT)
      {
        Text text = (Text)shape->data;
        char anchor = text_get_anchor(text);
        const char *text_anchor = "start";
        if (anchor == 'm' || anchor == 'M')
        {
          text_anchor = "middle";
        }
        else if (anchor == 'e' || anchor == 'E')
        {
          text_anchor = "end";
        }
        else if (anchor == 's' || anchor == 'S')
        {
          text_anchor = "start";
        }
        fprintf(file,
                "<text x='%.2f' y='%.2f' fill='%s' stroke='%s' "
                "text-anchor='%s'>%s</text>\n",
                text_get_x(text), text_get_y(text), text_get_fill_color(text),
                text_get_border_color(text), text_anchor, text_get_text(text));
      }
    }
    queue_enqueue(tempQueue, shape);
  }
  // restore ground queue
  while (!queue_is_empty(tempQueue))
  {
    queue_enqueue(groundQueue, queue_dequeue(tempQueue));
  }
  queue_destroy(tempQueue);

  // Render annotations from arena without destroying the stack
  Stack tempStack = stack_create();
  while (!stack_is_empty(arena))
  {
    ShapePositionOnArena_t *s = (ShapePositionOnArena_t *)stack_pop(arena);
    if (s != NULL && s->isAnnotated)
    {
      // dashed line from shooter to landed position
      fprintf(file,
              "<line x1='%.2f' y1='%.2f' x2='%.2f' y2='%.2f' stroke='red' "
              "stroke-dasharray='4,2' stroke-width='1'/>\n",
              s->shooterX, s->shooterY, s->x, s->y);
      // small circle marker at landed position
      fprintf(file,
              "<circle cx='%.2f' cy='%.2f' r='3' fill='none' stroke='red' "
              "stroke-width='1'/>\n",
              s->x, s->y);

      // dimension guides (horizontal then vertical) and labels (deslocamentoX, deslocamentoY)
      double deslocamentoX = s->x - s->shooterX;
      double deslocamentoY = s->y - s->shooterY;
      double midHx = s->shooterX + deslocamentoX * 0.5;
      double midHy = s->shooterY;
      double midVx = s->x;
      double midVy = s->shooterY + deslocamentoY * 0.5;

      // horizontal guide
      fprintf(file,
              "<line x1='%.2f' y1='%.2f' x2='%.2f' y2='%.2f' stroke='purple' "
              "stroke-dasharray='2,2' stroke-width='0.8'/>\n",
              s->shooterX, s->shooterY, s->x, s->shooterY);
      // vertical guide
      fprintf(file,
              "<line x1='%.2f' y1='%.2f' x2='%.2f' y2='%.2f' stroke='purple' "
              "stroke-dasharray='2,2' stroke-width='0.8'/>\n",
              s->x, s->shooterY, s->x, s->y);

      // deslocamentoX label above horizontal guide
      fprintf(file,
              "<text x='%.2f' y='%.2f' fill='purple' font-size='12' "
              "text-anchor='middle'>%.2f</text>\n",
              midHx, midHy - 5.0, deslocamentoX);

      // deslocamentoY label rotated near vertical guide
      fprintf(file,
              "<text x='%.2f' y='%.2f' fill='purple' font-size='12' "
              "text-anchor='middle' transform='rotate(-90 %.2f "
              "%.2f)'>%.2f</text>\n",
              midVx + 10.0, midVy, midVx + 10.0, midVy, deslocamentoY);
    }
    stack_push(tempStack, s);
  }
  // restore arena stack (original order)
  while (!stack_is_empty(tempStack))
  {
    stack_push(arena, stack_pop(tempStack));
  }
  stack_destroy(tempStack);

  fprintf(file, "</svg>\n");
  fclose(file);
  free(output_path_with_file);
  free(geo_base);
  free(qry_base);
}