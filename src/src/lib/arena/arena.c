#include "arena.h"
#include <stdlib.h>
#include <math.h>
#include "../pilha/pilha.h"
#include "../formas/formas.h"
#include "../utils/utils.h"

// Structs internas
typedef struct
{
    void *forma;
    double x, y;
    double origX, origY;
    int anotar;
} ItemArena;

struct Arena_t
{
    Stack itens;
};

// Protótipos
double get_area(void *forma);
int check_overlap(ItemArena *a, ItemArena *b);

Arena arena_criar()
{
    Arena a = malloc(sizeof(struct Arena_t));
    a->itens = stack_create();
    return a;
}

void arena_destruir(Arena a)
{
    if (!a)
        return;
    while (!stack_is_empty(a->itens))
        free(stack_pop(a->itens));
    stack_destroy(a->itens);
    free(a);
}

void arena_receber_disparo(Arena a, void *forma, double x, double y, double sx, double sy, int anotar)
{
    ItemArena *it = malloc(sizeof(ItemArena));
    it->forma = forma;
    it->x = x;
    it->y = y;
    it->origX = sx;
    it->origY = sy;
    it->anotar = anotar;
    stack_push(a->itens, it);
}

void arena_processar_calc(Arena a, Ground ground, Relatorio r)
{
    if (!a)
        return;
    Stack temp = stack_create();
    while (!stack_is_empty(a->itens))
        stack_push(temp, stack_pop(a->itens));

    while (!stack_is_empty(temp))
    {
        ItemArena *I = stack_pop(temp);
        if (stack_is_empty(temp))
        {
            // Reinsere no ground e fim (lógica simplificada)
            free(I);
            continue;
        }
        ItemArena *J = stack_pop(temp);

        if (check_overlap(I, J))
        {
            double areaI = get_area(I->forma);
            double areaJ = get_area(J->forma);

            if (areaI < areaJ)
            {
                relatorio_incrementar_esmagados(r);
                relatorio_somar_pontuacao(r, areaI);
            }
            else
            {
                relatorio_incrementar_clones(r);
            }
        }
        free(I);
        free(J);
    }
    stack_destroy(temp);
}

void arena_desenhar_svg_anotacoes(Arena a, FILE *svg)
{
    if (!a || !svg)
        return;

    Stack temp = stack_create();

    // Percorre a pilha para desenhar sem destruir os dados
    while (!stack_is_empty(a->itens))
    {
        ItemArena *it = stack_pop(a->itens);
        stack_push(temp, it);

        if (it->anotar)
        {
            // Linha tracejada do disparador até o ponto final
            fprintf(svg, "<line x1='%.2f' y1='%.2f' x2='%.2f' y2='%.2f' "
                         "stroke='red' stroke-width='1' stroke-dasharray='5,5' />\n",
                    it->origX, it->origY, it->x, it->y);

            // Ponto no destino
            fprintf(svg, "<circle cx='%.2f' cy='%.2f' r='2' fill='red' />\n",
                    it->x, it->y);

            // Guias de dimensão (opcional, como no exemplo do PDF)
            fprintf(svg, "<line x1='%.2f' y1='%.2f' x2='%.2f' y2='%.2f' stroke='purple' stroke-dasharray='2,2' stroke-width='0.5'/>\n",
                    it->origX, it->origY, it->x, it->origY); // Horizontal
            fprintf(svg, "<line x1='%.2f' y1='%.2f' x2='%.2f' y2='%.2f' stroke='purple' stroke-dasharray='2,2' stroke-width='0.5'/>\n",
                    it->x, it->origY, it->x, it->y); // Vertical
        }
    }

    // Restaura
    while (!stack_is_empty(temp))
    {
        stack_push(a->itens, stack_pop(temp));
    }
    stack_destroy(temp);
}

// Stubs simplificados
double get_area(void *forma) { return 100.0; }
int check_overlap(ItemArena *a, ItemArena *b) { return 0; }