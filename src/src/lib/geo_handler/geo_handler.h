#ifndef GEO_HANDLER_H
#define GEO_HANDLER_H
#include "../fila/fila.h"
#include "../pilha/pilha.h"
#include "../manipuladorDeArquivo/manipuladorDeArquivo.h"
#include <stdio.h>

typedef void *Ground;

Ground execute_geo_commands(FileData fileData, const char *output_path,
                            const char *command_suffix);

void destroy_geo_waste(Ground ground);
Queue get_ground_queue(Ground ground);
Stack get_ground_shapes_stack_to_free(Ground ground);

// FUNÇÕES ESSENCIAIS PARA O RELATÓRIO E ARENA
void geo_escrever_svg_forma(void *shape, FILE *svg);
void *geo_clonar_forma(void *shape, double x, double y, Ground ground);

#endif // GEO_HANDLER_H