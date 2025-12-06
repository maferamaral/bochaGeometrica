#ifndef GEO_HANDLER_H
#define GEO_HANDLER_H
#include "../fila/fila.h"
#include "../pilha/pilha.h"
#include "../manipuladorDeArquivo/manipuladorDeArquivo.h"
#include <stdio.h>

typedef void *Ground;

// Função para executar o processamento de um arquivo GEO
Ground execute_geo_commands(FileData fileData, const char *output_path,
                            const char *command_suffix);

void destroy_geo_waste(Ground ground);
Queue get_ground_queue(Ground ground);
Stack get_ground_shapes_stack_to_free(Ground ground);

// Escreve a tag SVG de uma forma num ficheiro
void geo_escrever_svg_forma(void *shape, FILE *svg);

// Clona uma forma para uma nova posição (x,y) e regista-a no Ground para limpeza futura
void *geo_clonar_forma(void *shape, double x, double y, Ground ground);

#endif // GEO_HANDLER_H