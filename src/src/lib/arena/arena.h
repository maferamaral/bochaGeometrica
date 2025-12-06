#ifndef ARENA_H
#define ARENA_H
#include <stdio.h>
#include "../geo_handler/geo_handler.h"
#include "../relatorio/relatorio.h"

typedef struct Arena_t *Arena;

Arena arena_criar();
void arena_destruir(Arena a);
void arena_receber_disparo(Arena a, void *forma, double x, double y, double shooterX, double shooterY, int anotar);
void arena_processar_calc(Arena a, Ground ground, Relatorio r);
void arena_desenhar_svg_anotacoes(Arena a, FILE *svg);

#endif
