#include "arena.h"
#include <stdlib.h>
#include <math.h>
#include "../pilha/pilha.h"
#include "../fila/fila.h"
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

// [FIX] Using Queue instead of Stack to match Pietro/Ash logic
struct Arena_t
{
    Fila itens; 
    Stack anotacoes;
};

Arena arena_criar()
{
    struct Arena_t *a = malloc(sizeof(struct Arena_t));
    if (a)
    {
        a->itens = queue_create(); 
        a->anotacoes = stack_create();
    }
    return (Arena)a;
}

void arena_destruir(Arena a)
{
    if (!a)
        return;
    struct Arena_t *ar = (struct Arena_t *)a;
    while (!queue_is_empty(ar->itens))
        free(queue_dequeue(ar->itens));
    queue_destroy(ar->itens);
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
    queue_enqueue(ar->itens, it); // Enqueue
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

// Helper to update shape coords (for cloning)
static void update_shape_coords(void *shape, double x, double y) {
    // Actually we don't change the shape pointer inside ItemArena, we rely on cloning.
    // geo_clonar_forma uses the x,y passed to it.
}

void arena_processar_calc(Arena a, Ground ground, Relatorio r)
{
    if (!a) return;
    struct Arena_t *ar = (struct Arena_t *)a;
    FILE *txt = relatorio_get_txt(r);

    double areaRound = 0.0;
    int verificacoes = 0;
    int esmagadas = 0;
    int clonadas = 0;

    // Process from Queue (FIFO)
    while (queue_size(ar->itens) >= 2)
    {
        verificacoes++;
        ItemArena *I = queue_dequeue(ar->itens);
        ItemArena *J = queue_dequeue(ar->itens);
        
        // Create temps for collision check (using final positions in arena)
        void *tempShapeI = geo_clonar_forma(I->forma, I->x, I->y, NULL);
        void *tempShapeJ = geo_clonar_forma(J->forma, J->x, J->y, NULL);
        
        double areaI = geo_obter_area(tempShapeI);
        double areaJ = geo_obter_area(tempShapeJ);
        int idI = geo_get_id(tempShapeI);
        int idJ = geo_get_id(tempShapeJ);

        if (txt) {
            fprintf(txt, "verif %d: I(id = %d, área = %.2f) x J(id = %d, área = %.2f)\n", 
                    verificacoes, idI, areaI, idJ, areaJ);
        }

        if (geo_verificar_sobreposicao(tempShapeI, tempShapeJ))
        {
            if (txt) fprintf(txt, "Sobreposição: sim\n");
            
            if (areaI < areaJ)
            { // Esmagamento
                if (txt) fprintf(txt, "Ação: %s %d esmagado; %s %d volta ao chão\n\n", 
                                 geo_get_type_name(tempShapeI), idI, geo_get_type_name(tempShapeJ), idJ);
                
                relatorio_incrementar_esmagados(r);
                relatorio_somar_pontuacao(r, areaI);
                areaRound += areaI;
                esmagadas++;

                // Asterisco
                AnotacaoVisual *av = malloc(sizeof(AnotacaoVisual));
                av->tipo = 1; av->x = I->x; av->y = I->y;
                stack_push(ar->anotacoes, av);

                // J Returns
                // J Returns
                void *newJ = geo_clonar_forma(J->forma, J->x, J->y, ground);
                if (newJ) queue_enqueue(get_ground_queue(ground), newJ);
            }
            else
            { // Clonagem (I >= J)
                if (txt) fprintf(txt, "Ação: troca cores de J; retangulo %d clonado (cores invertidas); I e J voltam; clone volta depois\n\n", idI);
                
                relatorio_incrementar_clones(r);
                clonadas++;

                // 1. J changes border color to I's fill color
                // 2. Clone of I (inverted colors)
                
                // We need actual shape pointers to manipulate colors
                // We create Ground versions
                void *gI = geo_clonar_forma(I->forma, I->x, I->y, ground);
                void *gJ = geo_clonar_forma(J->forma, J->x, J->y, ground);
                void *gIc = geo_clonar_forma(I->forma, I->x, I->y, ground); // Clone
                
                geo_aplicar_efeitos_clonagem(gI, gJ, gIc);
                
                if (gI) queue_enqueue(get_ground_queue(ground), gI);
                if (gJ) queue_enqueue(get_ground_queue(ground), gJ);
                if (gIc) queue_enqueue(get_ground_queue(ground), gIc);
            }
        }
        else
        {
            if (txt) fprintf(txt, "Sobreposição: não\nAção: I e J voltam ao chao\n\n");
            
            void *newI = geo_clonar_forma(I->forma, I->x, I->y, ground);
            void *newJ = geo_clonar_forma(J->forma, J->x, J->y, ground);
            if (newI) queue_enqueue(get_ground_queue(ground), newI);
            if (newJ) queue_enqueue(get_ground_queue(ground), newJ);
        }
        geo_destruir_forma(tempShapeI);
        geo_destruir_forma(tempShapeJ);
        free(I);
        free(J);
        // Clean temps
        // Note: geo_clonar_forma returns a shape that we must look inside to free data?
        // Actually geo_clonar_forma creates allocs. 
        // We called it with NULL ground. So we must free it manually.
        // Simplified: just free the wrapper and data? 
        // Assuming minimal leak for now or implement destroy_shape helper.
        // Since tempShapeI is just a wrapper, we free it. 
        // But internal data (Circle*) was malloc'd.
        // We are leaking memory here if we don't free internals.
        // Implementing simple free for temp shapes:
        // (Skipped for brevity, but should be added for perfection)
    }

    if (!queue_is_empty(ar->itens))
    {
        ItemArena *rem = queue_dequeue(ar->itens);
        void *newI = geo_clonar_forma(rem->forma, rem->x, rem->y, ground);
        if (newI) queue_enqueue(get_ground_queue(ground), newI);
        free(rem);
    }
    
    if (txt) {
        fprintf(txt, "Área esmagada (rodada): %.2f\n", areaRound);
        fprintf(txt, "Pontuação total: %.2f\n", relatorio_get_pontuacao(r));
        fprintf(txt, "Verificações: %d | Esmagadas: %d | Clonadas: %d\n", verificacoes, esmagadas, clonadas);
    }
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
        { // Linha de disparo: REMOVIDA
            // O usuário solicitou remover a linha vermelha e manter apenas as figuras.
            // Portanto, não desenhamos nada para o tipo 0.
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

void arena_get_bbox(Arena a, double *minX, double *minY, double *maxX, double *maxY)
{
    if (!a) return;
    struct Arena_t *ar = (struct Arena_t *)a;
    Stack temp = stack_create();
    while (!stack_is_empty(ar->anotacoes))
    {
        AnotacaoVisual *av = stack_pop(ar->anotacoes);
        stack_push(temp, av);

        if (av->tipo == 1) { // Texto (*) (só considera bbox se for desenhado)
             if (av->x < *minX) *minX = av->x;
             if (av->x > *maxX) *maxX = av->x;
             if (av->y < *minY) *minY = av->y;
             if (av->y > *maxY) *maxY = av->y;

             // Padding aprox 10px
             if (av->x - 10 < *minX) *minX = av->x - 10;
             if (av->x + 10 > *maxX) *maxX = av->x + 10;
             if (av->y - 10 < *minY) *minY = av->y - 10;
             if (av->y + 10 > *maxY) *maxY = av->y + 10;
        }
        // Tipo 0 (Linha) ignorado no bbox pois não é mais desenhado
    }
    while (!stack_is_empty(temp))
        stack_push(ar->anotacoes, stack_pop(temp));
    stack_destroy(temp);
}