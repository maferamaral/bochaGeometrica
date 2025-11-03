#ifndef DISPARADOR_H
#define DISPARADOR_H

#include "../pilha/pilha.h"
#include <stdio.h>

// Declarações opacas para os tipos internos
typedef struct Carregador Carregador;
typedef struct Disparador Disparador;

// Criação / destruição
Carregador *criar_carregador(int id);
Disparador *criar_disp(int id, float x, float y);
void destruir_carregador(Carregador *c);
void destruir_disparador(Disparador *d);

// Operações de carregamento e disparo
void carregar_disp(Disparador *d, int n, char *comando);
void *disparar(Disparador *d);

// Acessores e utilitários
int getId_carregador(Carregador *c);
int getId_disparador(Disparador *d);
int carregador_vazio(Carregador *c);
int carregador_pop(Carregador *c, void **out);
void set_carregador_esq(Disparador *d, Carregador *c);
void set_carregador_dir(Disparador *d, Carregador *c);
Carregador *get_carregador_esq(Disparador *d);
Carregador *get_carregador_dir(Disparador *d);
int disparador_vazio(Disparador *d);
int disparador_pop(Disparador *d, void **out);
float disparador_get_x(Disparador *d);
float disparador_get_y(Disparador *d);
void disparador_reportar_topo(Disparador *d, FILE *txt);
void carregador_reportar_figuras(Carregador *c, FILE *txt);

#endif // DISPARADOR_H
