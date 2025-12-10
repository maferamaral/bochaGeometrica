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

// --- FUNÇÕES DE DESENHO E CLONAGEM ---
void geo_escrever_svg_forma(void *shape, FILE *svg);
// Destroi forma e libera memória interna
void geo_destruir_forma(void *shape_ptr);
void *geo_clonar_forma(void *shape_ptr, double x, double y, Ground ground);

// --- FUNÇÕES DE CÁLCULO (NECESSÁRIAS PARA O TXT) ---
double geo_obter_area(void *shape);
int geo_get_id(void *shape);
double geo_get_shape_x(void *shape);
double geo_get_shape_y(void *shape);
const char *geo_get_type_name(void *shape);
int geo_verificar_sobreposicao(void *shapeA, void *shapeB);

void geo_imprimir_forma_txt(void *shape, FILE *txt);

// Função para clonagem especial com efeitos (troca de cores)
void geo_aplicar_efeitos_clonagem(void *I_ptr, void *J_ptr, void *I_clone_ptr);

// Atualiza posição da forma (para refletir movimento no report)
void geo_atualizar_posicao(void *shape_ptr, double x, double y);

// Calcula BBox da forma
void geo_get_bbox(void *shape, double *minX, double *minY, double *maxX, double *maxY);

#endif // GEO_HANDLER_H