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
    int tipo; // 0 = disparo, 1 = esmagamento
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
        av->tipo = 0;
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
    FILE *txt = relatorio_get_txt(r);

    if (txt)
        fprintf(txt, "\n--- Processando Arena (CALC) ---\n");

    Stack temp = stack_create();
    while (!stack_is_empty(ar->itens))
        stack_push(temp, stack_pop(ar->itens));

    double areaRound = 0.0;

    while (!stack_is_empty(temp))
    {
        ItemArena *I = stack_pop(temp);
        if (stack_is_empty(temp))
        {
            void *newI = geo_clonar_forma(I->forma, I->x, I->y, ground);
            if (newI)
                queue_enqueue(get_ground_queue(ground), newI);
            if (txt)
                fprintf(txt, "Forma solitaria (sem par) devolvida ao chao.\n");
            free(I);
            continue;
        }
        ItemArena *J = stack_pop(temp);

        void *tempShapeI = geo_clonar_forma(I->forma, I->x, I->y, NULL);
        void *tempShapeJ = geo_clonar_forma(J->forma, J->x, J->y, NULL);

        if (geo_verificar_sobreposicao(tempShapeI, tempShapeJ))
        {
            double areaI = geo_obter_area(tempShapeI);
            double areaJ = geo_obter_area(tempShapeJ);

            if (areaI < areaJ)
            { // Esmagamento
                if (txt)
                    fprintf(txt, "COLISAO: Forma I (Area %.2f) < Forma J (Area %.2f) -> I ESMAGADA.\n", areaI, areaJ);
                relatorio_incrementar_esmagados(r);
                relatorio_somar_pontuacao(r, areaI);
                areaRound += areaI;

                // Asterisco de esmagamento
                AnotacaoVisual *av = malloc(sizeof(AnotacaoVisual));
                av->tipo = 1;
                av->x = I->x;
                av->y = I->y;
                stack_push(ar->anotacoes, av);

                void *newJ = geo_clonar_forma(J->forma, J->x, J->y, ground);
                if (newJ)
                    queue_enqueue(get_ground_queue(ground), newJ);
            }
            else
            { // Clonagem
                if (txt)
                    fprintf(txt, "COLISAO: Forma I (Area %.2f) >= Forma J (Area %.2f) -> I CLONADA.\n", areaI, areaJ);
                relatorio_incrementar_clones(r);

                // Cria 3 formas resultantes conforme regra
                void *originalI = geo_clonar_forma(I->forma, I->x, I->y, ground);
                void *newJ = geo_clonar_forma(J->forma, J->x, J->y, ground);
                void *cloneI = geo_clonar_forma(I->forma, I->x, I->y, ground);

                // Aplica a troca de cores
                geo_aplicar_efeitos_clonagem(originalI, newJ, cloneI);

                if (originalI)
                    queue_enqueue(get_ground_queue(ground), originalI);
                if (newJ)
                    queue_enqueue(get_ground_queue(ground), newJ);
                if (cloneI)
                    queue_enqueue(get_ground_queue(ground), cloneI);
            }
        }
        else
        { // Sem colisão
            if (txt)
                fprintf(txt, "Sem colisao entre o par.\n");
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
    if (txt)
        fprintf(txt, "Area esmagada neste round: %.2f\n", areaRound);
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
        if (av->tipo == 0)
        { // Linha de disparo
            fprintf(svg, "<line x1='%.2f' y1='%.2f' x2='%.2f' y2='%.2f' stroke='red' stroke-width='1' stroke-dasharray='5,5' />\n", av->origX, av->origY, av->x, av->y);
            fprintf(svg, "<circle cx='%.2f' cy='%.2f' r='3' fill='none' stroke='red' />\n", av->x, av->y);
        }
        else if (av->tipo == 1)
        { // Asterisco de esmagamento
            fprintf(svg, "<text x='%.2f' y='%.2f' fill='red' font-weight='bold' font-size='20' text-anchor='middle' dominant-baseline='middle'>*</text>\n", av->x, av->y);
        }
    }
    while (!stack_is_empty(temp))
        stack_push(ar->anotacoes, stack_pop(temp));
    stack_destroy(temp);
}