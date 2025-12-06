#ifndef GEO_HANDLER_H
#define GEO_HANDLER_H
#include "../fila/fila.h"
#include "../pilha/pilha.h"
#include "../manipuladorDeArquivo/manipuladorDeArquivo.h"
#include <stdio.h> // Necessário para FILE*

typedef void *Ground;

// Função para executar o processamento de um arquivo GEO
Ground execute_geo_commands(FileData fileData, const char *output_path,
                            const char *command_suffix);

void destroy_geo_waste(Ground ground);
Queue get_ground_queue(Ground ground);
Stack get_ground_shapes_stack_to_free(Ground ground);

// NOVA FUNÇÃO: Permite desenhar uma forma opaca num ficheiro SVG
void geo_escrever_svg_forma(void *shape, FILE *svg);

#endif // GEO_HANDLER_H