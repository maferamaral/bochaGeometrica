#include "relatorio.h"
#include <stdlib.h>
#include <string.h>
#include "../pilha/pilha.h"
#include "../fila/fila.h"
#include "../arena/arena.h"
#include "../geo_handler/geo_handler.h"
#include "../utils/utils.h" // Para duplicate_string

struct Relatorio_t
{
    FILE *txtFile;
    char *svgPath;
    int totalDisparos;
    int totalClones;
    int totalEsmagados;
    double pontuacao;
};

Relatorio relatorio_criar(const char *geoName, const char *qryName, const char *outPath)
{
    Relatorio r = malloc(sizeof(struct Relatorio_t));
    if (r == NULL)
        return NULL;

    char baseGeo[256], baseQry[256];
    strncpy(baseGeo, geoName, 255);
    baseGeo[255] = '\0';
    strncpy(baseQry, qryName, 255);
    baseQry[255] = '\0';

    char *dot = strrchr(baseGeo, '.');
    if (dot)
        *dot = 0;
    dot = strrchr(baseQry, '.');
    if (dot)
        *dot = 0;

    size_t lenPath = strlen(outPath) + strlen(baseGeo) + strlen(baseQry) + 20;
    char *path = malloc(lenPath);

    if (path)
    {
        sprintf(path, "%s/%s-%s.txt", outPath, baseGeo, baseQry);
        r->txtFile = fopen(path, "w");

        sprintf(path, "%s/%s-%s.svg", outPath, baseGeo, baseQry);
        r->svgPath = duplicate_string(path); // Correção do strdup

        free(path);
    }
    else
    {
        r->txtFile = NULL;
        r->svgPath = NULL;
    }

    r->totalDisparos = 0;
    r->totalClones = 0;
    r->totalEsmagados = 0;
    r->pontuacao = 0.0;
    return r;
}

void relatorio_registrar_comando(Relatorio r, const char *cmd, const char *detalhes)
{
    if (r && r->txtFile)
        fprintf(r->txtFile, "COMANDO: %s\n%s\n", cmd, detalhes);
}

void relatorio_incrementar_disparos(Relatorio r)
{
    if (r)
        r->totalDisparos++;
}
void relatorio_incrementar_clones(Relatorio r)
{
    if (r)
        r->totalClones++;
}
void relatorio_incrementar_esmagados(Relatorio r)
{
    if (r)
        r->totalEsmagados++;
}
void relatorio_somar_pontuacao(Relatorio r, double area)
{
    if (r)
        r->pontuacao += area;
}

void relatorio_escrever_final(Relatorio r, int qtdComandos)
{
    if (!r || !r->txtFile)
        return;
    fprintf(r->txtFile, "\n=== RELATÓRIO FINAL ===\n");
    fprintf(r->txtFile, "Pontuação final: %.2lf\n", r->pontuacao);
    fprintf(r->txtFile, "Instruções executadas: %d\n", qtdComandos);
    fprintf(r->txtFile, "Total de disparos: %d\n", r->totalDisparos);
    fprintf(r->txtFile, "Formas clonadas: %d\n", r->totalClones);
    fprintf(r->txtFile, "Formas esmagadas: %d\n", r->totalEsmagados);
}

void relatorio_gerar_svg(Relatorio r, Ground ground, void *arena_ptr)
{
    if (!r || !r->svgPath)
        return;

    FILE *svg = fopen(r->svgPath, "w");
    if (!svg)
        return;

    fprintf(svg, "<?xml version=\"1.0\"?>\n<svg xmlns=\"http://www.w3.org/2000/svg\">\n");

    // 1. Desenha as formas do chão
    Queue q = get_ground_queue(ground);
    Queue temp = queue_create();

    while (!queue_is_empty(q))
    {
        void *s = queue_dequeue(q);
        // Desenha a forma usando a função do geo_handler
        geo_escrever_svg_forma(s, svg);
        queue_enqueue(temp, s);
    }
    // Restaura
    while (!queue_is_empty(temp))
        queue_enqueue(q, queue_dequeue(temp));
    queue_destroy(temp);

    // 2. Desenha as anotações da Arena
    if (arena_ptr)
    {
        arena_desenhar_svg_anotacoes(arena_ptr, svg);
    }

    fprintf(svg, "</svg>");
    fclose(svg);
}

void relatorio_destruir(Relatorio r)
{
    if (!r)
        return;
    if (r->txtFile)
        fclose(r->txtFile);
    if (r->svgPath)
        free(r->svgPath);
    free(r);
}

FILE *relatorio_get_txt(Relatorio r) { return r ? r->txtFile : NULL; }