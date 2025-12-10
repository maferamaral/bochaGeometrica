#ifndef GEO_HANDLER_H
#define GEO_HANDLER_H

#include <stdio.h>
#include "../fila/fila.h"
#include "../pilha/pilha.h"
#include "../manipuladorDeArquivo/manipuladorDeArquivo.h"

typedef void *Ground;

// Processa o arquivo .geo e cria o estado inicial do chão
Ground execute_geo_commands(FileData fileData, const char *output_path, const char *command_suffix);

// Limpa a memória do chão
void destroy_geo_waste(Ground ground);

// Getters para as estruturas internas do Ground
Queue get_ground_queue(Ground ground);
Stack get_ground_shapes_stack_to_free(Ground ground);

// --- NOVAS FUNÇÕES NECESSÁRIAS PARA A CORREÇÃO ---

/**
 * Escreve a tag SVG correspondente à forma no arquivo fornecido.
 * Usado pelo relatorio.c para desenhar o estado final.
 */
void geo_escrever_svg_forma(void *shape, FILE *svg);

/**
 * Cria uma cópia profunda da forma 'shape', posicionada nas novas coordenadas (x, y).
 * A nova forma é automaticamente registrada no 'ground' para ser libertada no final (evita memory leaks).
 * Retorna um ponteiro para a nova forma (Shape_t*).
 */
void *geo_clonar_forma(void *shape, double x, double y, Ground ground);

#endif // GEO_HANDLER_H