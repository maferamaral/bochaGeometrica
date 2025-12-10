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

typedef struct
{
    double x, y;
    double origX, origY;
} AnotacaoVisual;

struct Arena_t
{
    Stack itens;
    Stack anotacoes;
};

Arena arena_criar()
{
    struct Arena_t *a = malloc(sizeof(struct Arena_t));
    if (a)
    {
        a->itens = stack_create();
        a->anotacoes = stack_create();
    }
    return (Arena)a;
}

void arena_destruir(Arena a)
{
    if (!a)
        return;
    struct Arena_t *ar = (struct Arena_t *)a;
    while (!stack_is_empty(ar->itens))
        free(stack_pop(ar->itens));
    stack_destroy(ar->itens);
    while (!stack_is_empty(ar->anotacoes))
        free(stack_pop(ar->anotacoes));
    stack_destroy(ar->anotacoes);
    free(ar);
}

void arena_receber_disparo(Arena a, void *forma, double x, double y, double shooterX, double shooterY, int anotar)
{
    struct Arena_t *ar = (struct Arena_t *)a;
    ItemArena *it = malloc(sizeof(ItemArena));
    it->forma = forma;
    it->x = x;
    it->y = y;
    it->origX = shooterX;
    it->origY = shooterY;
    it->anotar = anotar;
    stack_push(ar->itens, it);
    if (anotar)
    {
        AnotacaoVisual *av = malloc(sizeof(AnotacaoVisual));
        av->x = x;
        av->y = y;
        av->origX = shooterX;
        av->origY = shooterY;
        stack_push(ar->anotacoes, av);
    }
}

void arena_processar_calc(Arena a, Ground ground, Relatorio r)
{
    if (!a)
        return;
    struct Arena_t *ar = (struct Arena_t *)a;
    Stack temp = stack_create();
    while (!stack_is_empty(ar->itens))
        stack_push(temp, stack_pop(ar->itens));

    while (!stack_is_empty(temp))
    {
        ItemArena *I = stack_pop(temp);
        if (stack_is_empty(temp))
        {
            void *newI = geo_clonar_forma(I->forma, I->x, I->y, ground);
            if (newI)
                queue_enqueue(get_ground_queue(ground), newI);
            free(I);
            continue;
        }
        ItemArena *J = stack_pop(temp);

        // Clona temporariamente para verificar colisão com as coordenadas corretas
        void *tempShapeI = geo_clonar_forma(I->forma, I->x, I->y, NULL);
        void *tempShapeJ = geo_clonar_forma(J->forma, J->x, J->y, NULL);

        // USANDO AS FUNÇÕES REAIS AGORA
        if (geo_verificar_sobreposicao(tempShapeI, tempShapeJ))
        {
            double areaI = geo_obter_area(tempShapeI);
            double areaJ = geo_obter_area(tempShapeJ);

            if (areaI < areaJ)
            { // Esmagamento
                relatorio_incrementar_esmagados(r);
                relatorio_somar_pontuacao(r, areaI);
                void *newJ = geo_clonar_forma(J->forma, J->x, J->y, ground);
                if (newJ)
                    queue_enqueue(get_ground_queue(ground), newJ);
            }
            else
            { // Clonagem
                relatorio_incrementar_clones(r);
                void *newI = geo_clonar_forma(I->forma, I->x, I->y, ground);
                void *newJ = geo_clonar_forma(J->forma, J->x, J->y, ground);
                if (newI)
                    queue_enqueue(get_ground_queue(ground), newI);
                if (newJ)
                    queue_enqueue(get_ground_queue(ground), newJ);
            }
        }
        else
        { // Sem colisão
            void *newI = geo_clonar_forma(I->forma, I->x, I->y, ground);
            void *newJ = geo_clonar_forma(J->forma, J->x, J->y, ground);
            if (newI)
                queue_enqueue(get_ground_queue(ground), newI);
            if (newJ)
                queue_enqueue(get_ground_queue(ground), newJ);
        }
        free(tempShapeI);
        free(tempShapeJ);
        free(I);
        free(J);
    }
    stack_destroy(temp);
}

void arena_desenhar_svg_anotacoes(Arena a, FILE *svg)
{
    if (!a || !svg)
        return;
    struct Arena_t *ar = (struct Arena_t *)a;
    Stack temp = stack_create();
    while (!stack_is_empty(ar->anotacoes))
    {
        AnotacaoVisual *av = stack_pop(ar->anotacoes);
        stack_push(temp, av);
        fprintf(svg, "<line x1='%.2f' y1='%.2f' x2='%.2f' y2='%.2f' stroke='red' stroke-width='1' stroke-dasharray='5,5' />\n",
                av->origX, av->origY, av->x, av->y);
        fprintf(svg, "<circle cx='%.2f' cy='%.2f' r='3' fill='none' stroke='red' />\n", av->x, av->y);
    }
    while (!stack_is_empty(temp))
        stack_push(ar->anotacoes, stack_pop(temp));
    stack_destroy(temp);
}