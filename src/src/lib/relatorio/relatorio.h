#ifndef RELATORIO_H
#define RELATORIO_H
#include <stdio.h>
#include "../geo_handler/geo_handler.h"

typedef struct Relatorio_t *Relatorio;

Relatorio relatorio_criar(const char *geoName, const char *qryName, const char *outPath);
void relatorio_registrar_comando(Relatorio r, const char *cmd, const char *detalhes);
void relatorio_incrementar_disparos(Relatorio r);
void relatorio_incrementar_clones(Relatorio r);
void relatorio_incrementar_esmagados(Relatorio r);
void relatorio_somar_pontuacao(Relatorio r, double area);
void relatorio_escrever_final(Relatorio r, int qtdComandos);
void relatorio_gerar_svg(Relatorio r, Ground ground, void *arena_ptr);
void relatorio_destruir(Relatorio r);
FILE *relatorio_get_txt(Relatorio r);

#endif