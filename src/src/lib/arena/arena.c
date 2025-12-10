#include "arena.h"
#include <stdlib.h>
#include <math.h>
#include "../pilha/pilha.h"
#include "../formas/formas.h"
#include "../utils/utils.h"
#include "../geo_handler/geo_handler.h"

// Estrutura para os itens lógicos (usados para o cálculo de física/colisão)
typedef struct
{
    void *forma;
    double x, y;
    double origX, origY;
    int anotar;
} ItemArena;

// NOVA ESTRUTURA: Apenas para guardar os dados do desenho (persiste após o calc)
typedef struct
{
    double x, y;
    double origX, origY;
} AnotacaoVisual;

struct Arena_t
{
    Stack itens;     // Pilha processada e esvaziada pelo calc
    Stack anotacoes; // Pilha persistente para o SVG final
};

// Funções auxiliares simples para cálculo de área e colisão (Stubs funcionais)
// Nota: Em uma implementação completa, estas chamariam funções do geo_handler ou formas
double get_area_stub(void *forma) { return 100.0; }
int check_overlap_stub(ItemArena *a, ItemArena *b) { return 0; }

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

    // Limpa itens lógicos
    while (!stack_is_empty(ar->itens))
        free(stack_pop(ar->itens));
    stack_destroy(ar->itens);

    // Limpa anotações visuais
    while (!stack_is_empty(ar->anotacoes))
        free(stack_pop(ar->anotacoes));
    stack_destroy(ar->anotacoes);

    free(ar);
}

void arena_receber_disparo(Arena a, void *forma, double x, double y, double shooterX, double shooterY, int anotar)
{
    struct Arena_t *ar = (struct Arena_t *)a;

    // 1. Cria o item lógico para o jogo (será consumido no calc)
    ItemArena *it = malloc(sizeof(ItemArena));
    it->forma = forma;
    it->x = x;
    it->y = y;
    it->origX = shooterX;
    it->origY = shooterY;
    it->anotar = anotar;
    stack_push(ar->itens, it);

    // 2. Se for para anotar, guarda uma cópia na pilha de visualização (NÃO será consumida no calc)
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

    // Inverter pilha para processar na ordem correta (FIFO)
    Stack temp = stack_create();
    while (!stack_is_empty(ar->itens))
        stack_push(temp, stack_pop(ar->itens));

    while (!stack_is_empty(temp))
    {
        ItemArena *I = stack_pop(temp);

        // Se I for o último elemento (sem par para colidir)
        if (stack_is_empty(temp))
        {
            // Clona I na posição final e devolve ao Ground para ser desenhado
            void *newI = geo_clonar_forma(I->forma, I->x, I->y, ground);
            if (newI)
                queue_enqueue(get_ground_queue(ground), newI);

            free(I);
            continue;
        }

        ItemArena *J = stack_pop(temp);

        // Aqui entraria a lógica real de colisão. Usando stub para compilar.
        if (check_overlap_stub(I, J))
        {
            double areaI = get_area_stub(I->forma);
            double areaJ = get_area_stub(J->forma);

            if (areaI < areaJ)
            {
                relatorio_incrementar_esmagados(r);
                relatorio_somar_pontuacao(r, areaI);
                // J sobrevive
                void *newJ = geo_clonar_forma(J->forma, J->x, J->y, ground);
                if (newJ)
                    queue_enqueue(get_ground_queue(ground), newJ);
            }
            else
            {
                relatorio_incrementar_clones(r);
                // Ambos sobrevivem
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
            if (newI)
                queue_enqueue(get_ground_queue(ground), newI);
            if (newJ)
                queue_enqueue(get_ground_queue(ground), newJ);
        }
        free(I);
        free(J);
    }
    stack_destroy(temp);

    // NOTA IMPORTANTE: A pilha ar->anotacoes NÃO foi tocada aqui, logo o desenho das linhas vermelhas sobreviverá!
}

void arena_desenhar_svg_anotacoes(Arena a, FILE *svg)
{
    if (!a || !svg)
        return;
    struct Arena_t *ar = (struct Arena_t *)a;

    Stack temp = stack_create();

    // AGORA ITERAMOS SOBRE A PILHA 'anotacoes', que contem o histórico visual
    while (!stack_is_empty(ar->anotacoes))
    {
        AnotacaoVisual *av = stack_pop(ar->anotacoes);
        stack_push(temp, av);

        // Desenha a linha tracejada e o ponto final
        fprintf(svg, "<line x1='%.2f' y1='%.2f' x2='%.2f' y2='%.2f' stroke='red' stroke-width='1' stroke-dasharray='5,5' />\n",
                av->origX, av->origY, av->x, av->y);
        fprintf(svg, "<circle cx='%.2f' cy='%.2f' r='3' fill='none' stroke='red' />\n", av->x, av->y);

        // Guias opcionais (roxas) para dimensões
        fprintf(svg, "<line x1='%.2f' y1='%.2f' x2='%.2f' y2='%.2f' stroke='purple' stroke-dasharray='2,2' stroke-width='0.5'/>\n",
                av->origX, av->origY, av->x, av->origY); // Horizontal
        fprintf(svg, "<line x1='%.2f' y1='%.2f' x2='%.2f' y2='%.2f' stroke='purple' stroke-dasharray='2,2' stroke-width='0.5'/>\n",
                av->x, av->origY, av->x, av->y); // Vertical
    }

    // Restaura a pilha (caso fosse necessário usar novamente)
    while (!stack_is_empty(temp))
        stack_push(ar->anotacoes, stack_pop(temp));
    stack_destroy(temp);
}