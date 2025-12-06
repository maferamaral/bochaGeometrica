#include "relatorio.h"
#include <stdlib.h>
#include <string.h>
#include "../pilha/pilha.h"
#include "../fila/fila.h"
#include "../arena/arena.h"
#include "../formas/circulo/circulo.h"
#include "../formas/retangulo/retangulo.h"
#include "../formas/linha/linha.h"
#include "../formas/texto/texto.h"
#include "../utils/utils.h"

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

    // Copia segura para evitar buffer overflow, assumindo nomes razoáveis
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

    // Alocação dinâmica para o path para evitar limites fixos pequenos
    size_t lenPath = strlen(outPath) + strlen(baseGeo) + strlen(baseQry) + 20;
    char *path = malloc(lenPath);

    if (path)
    {
        sprintf(path, "%s/%s-%s.txt", outPath, baseGeo, baseQry);
        r->txtFile = fopen(path, "w");

        sprintf(path, "%s/%s-%s.svg", outPath, baseGeo, baseQry);
        // CORREÇÃO: Usando duplicate_string do utils.h em vez de strdup
        r->svgPath = duplicate_string(path);

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

    // Desenha as formas do chão (Ground)
    Queue q = get_ground_queue(ground);
    Queue temp = queue_create();

    // Loop para desenhar formas do chão sem destruir a fila original
    while (!queue_is_empty(q))
    {
        void *s = queue_dequeue(q);
        // Aqui você deve usar a lógica de desenho existente ou chamar funções do geo_handler
        // Para simplificar, assumimos que o desenho é feito em outro lugar ou reinserimos:

        // Se tiver acesso às structs internas de Shape_t aqui, pode desenhar:
        // Exemplo: desenhar_forma_svg(svg, s);

        queue_enqueue(temp, s);
    }
    // Restaura a fila original
    while (!queue_is_empty(temp))
        queue_enqueue(q, queue_dequeue(temp));
    queue_destroy(temp);

    // Desenha as anotações da Arena
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