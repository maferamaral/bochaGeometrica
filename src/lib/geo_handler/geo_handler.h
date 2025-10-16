#ifndef GEO_HANDLER_H
#define GEO_HANDLER_H
#include "../fila/fila.h"

// Função para executar o processamento de um arquivo GEO
// Recebe uma fila contendo as linhas do arquivo GEO a serem processadas
// geo_lines_queue: fila contendo as linhas do arquivo GEO
void executar_geo(Queue geo_lines_queue);

#endif // GEO_HANDLER_H