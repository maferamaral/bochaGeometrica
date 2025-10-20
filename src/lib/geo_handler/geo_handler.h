#ifndef GEO_HANDLER_H
#define GEO_HANDLER_H
#include "../fila/fila.h"
#include "../pilha/pilha.h"
#include "../manipuladorDeArquivo/manipuladorDeArquivo.h"

typedef void *Ground;

// Função para executar o processamento de um arquivo GEO
// Recebe uma fila contendo as linhas do arquivo GEO a serem processadas
// geo_lines_queue: fila contendo as linhas do arquivo GEO
Ground execute_geo_commands(FileData fileData, const char *output_path,
                            const char *command_suffix);

void destroy_geo_waste(Ground ground);
Queue get_ground_queue(Ground ground);
Stack get_ground_shapes_stack_to_free(Ground ground);
#endif // GEO_HANDLER_H