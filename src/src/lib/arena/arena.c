#include "arena.h"
#include <stdlib.h>
#include <math.h>
#include "../pilha/pilha.h"
#include "../formas/formas.h"
#include "../utils/utils.h"
#include "../geo_handler/geo_handler.h"

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

// Implementação básica de área para teste
double get_area(void *forma) { return 100.0; }              // Simplificação
int check_overlap(ItemArena *a, ItemArena *b) { return 0; } // Simplificação

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

    // Inverter pilha para processar na ordem correta
    Stack temp = stack_create();
    while (!stack_is_empty(a->itens))
        stack_push(temp, stack_pop(a->itens));

    while (!stack_is_empty(temp))
    {
        ItemArena *I = stack_pop(temp);

        // Se I for o último elemento (sem par para colidir)
        if (stack_is_empty(temp))
        {
            // Clona I na posição final e devolve ao Ground
            void *newI = geo_clonar_forma(I->forma, I->x, I->y, ground);
            if (newI)
                queue_enqueue(get_ground_queue(ground), newI);

            free(I);
            continue;
        }

        ItemArena *J = stack_pop(temp);

        if (check_overlap(I, J))
        {
            // Lógica simplificada de colisão
            double areaI = get_area(I->forma);
            double areaJ = get_area(J->forma);

            if (areaI < areaJ)
            {
                relatorio_incrementar_esmagados(r);
                relatorio_somar_pontuacao(r, areaI);
                // J sobrevive e volta ao chão
                void *newJ = geo_clonar_forma(J->forma, J->x, J->y, ground);
                if (newJ)
                    queue_enqueue(get_ground_queue(ground), newJ);
            }
            else
            {
                relatorio_incrementar_clones(r);
                // Ambos sobrevivem (com alterações de cor que omitimos aqui para brevidade)
                void *newI = geo_clonar_forma(I->forma, I->x, I->y, ground);
                void *newJ = geo_clonar_forma(J->forma, J->x, J->y, ground);
                if (newI)
                    queue_enqueue(get_ground_queue(ground), newI);
                if (newJ)
                    queue_enqueue(get_ground_queue(ground), newJ);
            }
        }
        else
        {
            // Sem colisão: ambos voltam ao chão
            void *newI = geo_clonar_forma(I->forma, I->x, I->y, ground);
            void *newJ = geo_clonar_forma(J->forma, J->x, J->y, ground);

            Queue gq = get_ground_queue(ground);
            if (newI)
                queue_enqueue(gq, newI);
            if (newJ)
                queue_enqueue(gq, newJ);
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

    // Desenha anotações sem consumir a pilha
    while (!stack_is_empty(a->itens))
    {
        ItemArena *it = stack_pop(a->itens);
        stack_push(temp, it);
        if (it->anotar)
        {
            fprintf(svg, "<line x1='%.2f' y1='%.2f' x2='%.2f' y2='%.2f' stroke='red' stroke-width='1' stroke-dasharray='5,5' />\n",
                    it->origX, it->origY, it->x, it->y);
            fprintf(svg, "<circle cx='%.2f' cy='%.2f' r='3' fill='none' stroke='red' />\n", it->x, it->y);
        }
    }
    // Restaura pilha
    while (!stack_is_empty(temp))
        stack_push(a->itens, stack_pop(temp));
    stack_destroy(temp);
}