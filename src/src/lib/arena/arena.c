#include "arena.h"
#include <stdlib.h>
#include <math.h>
#include "../pilha/pilha.h"
#include "../formas/formas.h"
#include "../utils/utils.h"

// Structs internas simplificadas
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

// Funções auxiliares de geometria (implementar as de cálculo de área/colisão aqui)
double get_area(void *forma);                  // Implementar baseado no tipo
int check_overlap(ItemArena *a, ItemArena *b); // Implementar intersecção AABB

Arena arena_criar()
{
    Arena a = malloc(sizeof(struct Arena_t));
    a->itens = stack_create();
    return a;
}

void arena_destruir(Arena a)
{
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
    Stack temp = stack_create();
    while (!stack_is_empty(a->itens))
        stack_push(temp, stack_pop(a->itens));

    while (!stack_is_empty(temp))
    {
        ItemArena *I = stack_pop(temp);
        if (stack_is_empty(temp))
        {
            // Reinsere no ground e fim
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
                // J volta ao chão, I morre
            }
            else
            {
                relatorio_incrementar_clones(r);
                // I clona e troca cor, J troca borda
                // Ambos voltam ao chão
            }
        }
        else
        {
            // Ambos voltam ao chão
        }
        free(I);
        free(J);
    }
    stack_destroy(temp);
}

void arena_desenhar_svg_anotacoes(Arena a, FILE *svg)
{
    // Percorre pilha da arena e desenha linhas tracejadas se it->anotar == 1
}

// Stubs para compilar (devem ser preenchidos com a lógica real de geometria)
double get_area(void *forma) { return 100.0; }
int check_overlap(ItemArena *a, ItemArena *b) { return 0; }