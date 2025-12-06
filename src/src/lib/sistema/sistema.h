#ifndef SISTEMA_H
#define SISTEMA_H
#include "../pilha/pilha.h"
#include "../geo_handler/geo_handler.h"

typedef struct Sistema_t *Sistema;

Sistema sistema_criar();
void sistema_destruir(Sistema s);
void sistema_add_atirador(Sistema s, int id, double x, double y);
void sistema_add_carregador(Sistema s, int id, int numFormas, Ground ground);
void sistema_atch(Sistema s, int idAtirador, int idEsq, int idDir);
void sistema_shft(Sistema s, int idAtirador, const char *lado, int n);
void *sistema_preparar_disparo(Sistema s, int idAtirador, double *xOut, double *yOut);

#endif