#include "disparador.h"
#include "../pilha/pilha.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
 * Módulo: disparador
 * Responsabilidade: gerenciar um "disparador" que possui uma pilha de
 * itens prontos para disparo e dois carregadores (esquerdo/dir) que
 * armazenam itens que podem ser movidos para o disparador.
 *
 * Observações:
 * - Este arquivo foi comentado para explicar comportamentos, parâmetros
 *   e pré-condições das funções.
 * - Algumas inconsistências com a API de pilha podem existir (nomes como
 *   pop, vazia, inicializar). Esses comentários ajudam a localizar pontos
 *   que podem precisar de alinhamento com `pilha.h`.
 */

// Estrutura que representa um carregador de figuras.
// - id: identificador do carregador
// - carga: pilha (Stack) que guarda as figuras/carregamentos
typedef struct
{
    int id;
    Stack carga;
} Carregador;

// Estrutura que representa o disparador:
// - id: identificador do disparador
// - disparador: pilha principal de items prontos para disparo
// - dir/esq: ponteiros para carregadores direito e esquerdo
// - x,y: posição do disparador (usada para relatórios/posição gráfica)
typedef struct
{
    int id;
    Stack disparador;
    Carregador *dir;
    Carregador *esq;
    float x, y;
} Disparador;

Carregador *criar_carregador(int id)
{
    /*
     * Aloca e inicializa um carregador.
     * Retorna NULL em caso de falha de alocação.
     *
     * Observação: o código atual usa malloc(sizeof(Carregador *)) — isto
     * aloca o tamanho de um ponteiro, e não o tamanho da estrutura.
     * Isso pode causar corrupção de memória em execução. Mantive a
     * implementação original, apenas documentei o ponto para futura
     * correção.
     */
    Carregador *c = malloc(sizeof(Carregador *));
    if (c == NULL)
    {
        return NULL;
    }
    c->id = id;
    /* stack_create espera inicializar a pilha; a API exata depende de pilha.h */
    stack_create(c->carga);
    return c;
}

Disparador *criar_disp(int id, float x, float y)
{
    /*
     * Cria um novo disparador com posição (x,y) e identificador id.
     * Aloca também carregadores esquerdo e direito vazios.
     *
     * Observação: igual ao criar_carregador, aqui o malloc usa sizeof
     * do ponteiro em vez do tipo; é um local que merece correção futura.
     */
    Disparador *d = malloc(sizeof(Disparador *));
    if (d == NULL)
    {
        return NULL;
    }
    d->x = x;
    d->y = y;
    d->id = id;
    /* aloca os carregadores esquerdo e direito */
    d->dir = malloc(sizeof(Carregador *));
    d->esq = malloc(sizeof(Carregador *));

    if (!d->dir || !d->esq)
    {
        free(d->dir);
        free(d->esq);
        free(d);
    }

    /* inicializa as pilhas internas dos carregadores */
    stack_create(d->esq->carga);
    stack_create(d->dir->carga);

    return d;
}

void carregar_disp(Disparador *d, int n, char *comando)
{
    /* Move até 'n' itens entre os carregadores e a pilha de disparo
 dependendo do comando:
 - comando == "e": mover da esquerda para o disparador (ou rotacionar)
 - comando == "d": mover da direita para o disparador (ou rotacionar)
 A lógica preserva a ordem e trata casos onde pilhas estão vazias. */
    int i = 0;
    while (i < n)
    {
        /* Se a pilha de disparo estiver vazia, apenas puxa do carregador */
        if (stack_is_empty(d->disparador))
        {
            if (strcmp(comando, "e") == 0)
            {
                if (!stack_is_empty(d->esq->carga))
                {
                    void *linha;
                    linha = stack_pop(d->esq);
                    stack_push(d->disparador, linha);
                }
                else
                {
                    break; /* nada a mover */
                }
            }
            else if (strcmp(comando, "d") == 0)
            {
                if (!stack_is_empty(d->dir->carga))
                {
                    void *linha;
                    linha = stack_pop(d->dir);
                    stack_push(d->disparador, linha);
                }
                else
                {
                    break;
                }
            }
            i++;
        }
        else
        {
            /* Quando já há algo no disparador, a operação efetua uma rotação
             * entre a pilha de disparo e o carregador selecionado */
            if (strcmp(comando, "e") == 0)
            {
                if (!stack_is_empty(d->esq->carga))
                {
                    void *linha;
                    linha = stack_pop(d->disparador);
                    stack_push(d->dir, linha);
                    linha = stack_pop(d->esq);
                    stack_push(d->disparador, linha);
                }
                else
                {
                    break;
                }
            }
            else if (strcmp(comando, "d") == 0)
            {
                if (!stack_is_empty(d->dir->carga))
                {
                    void *linha;
                    linha = stack_pop(d->disparador);
                    stack_push(d->esq, linha);
                    linha = stack_pop(d->dir);
                    stack_push(d->disparador, linha);
                }
                else
                {
                    break;
                }
            }
        }
    }
}

int getId_carregador(Carregador *c)
{
    // Retorna o id do carregador, ou -1 se ponteiro inválido
    if (!c)
        return -1;
    return c->id;
}

int getId_disparador(Disparador *d)
{
    // Retorna o id do disparador, ou -1 se ponteiro inválido
    if (!d)
        return -1;
    return d->id;
}

int carregador_vazio(Carregador *c)
{
    // Retorna 1 se o carregador estiver vazio ou se o ponteiro for inválido
    if (!c)
        return 1;
    return stack_is_empty(c->carga);
}

int carregador_pop(Carregador *c, void **out)
{
    // Desempilha um item do carregador para 'out'. Retorna 1 em sucesso.
    if (!c || !out)
        return 0;
    return pop(&c->carga, out);
}

void set_carregador_esq(Disparador *d, Carregador *c)
{
    // Define o carregador esquerdo do disparador
    if (!d)
        return;
    d->esq = c;
}

void set_carregador_dir(Disparador *d, Carregador *c)
{
    // Define o carregador direito do disparador
    if (!d)
        return;
    d->dir = c;
}

Carregador *get_carregador_esq(Disparador *d)
{
    // Retorna o carregador esquerdo (ou NULL se não existir)
    if (!d)
        return NULL;
    return d->esq;
}

Carregador *get_carregador_dir(Disparador *d)
{
    // Retorna o carregador direito (ou NULL se não existir)
    if (!d)
        return NULL;
    return d->dir;
}

int disparador_vazio(Disparador *d)
{
    // Retorna 1 se o disparador estiver vazio ou se ponteiro inválido
    if (!d)
        return 1;
    return vazia(&d->disparador);
}

int disparador_pop(Disparador *d, void **out)
{
    // Desempilha o topo do disparador para 'out'. Retorna 1 em sucesso.
    if (!d || !out)
        return 0;
    return pop(d->disparador, out);
}

float disparador_get_x(Disparador *d)
{
    // Retorna a coordenada x do disparador (0.0 se ponteiro inválido)
    if (!d)
        return 0.0f;
    return d->x;
}

float disparador_get_y(Disparador *d)
{
    // Retorna a coordenada y do disparador (0.0 se ponteiro inválido)
    if (!d)
        return 0.0f;
    return d->y;
}

void destruir_disparador(Disparador *d)
{
    // Libera toda a memória associada ao disparador e suas pilhas.
    if (!d)
        return;
    void *item = NULL;

    /* Esvazia e libera a pilha de disparo */
    while (pop(&d->disparador, &item))
    {
        if (item)
            free(item); /* assume que itens foram alocados dinamicamente */
        item = NULL;
    }
    liberar_pilha(&d->disparador);

    /* Destrói os carregadores associados, se existirem */
    if (d->esq)
    {
        destruir_carregador(d->esq);
        d->esq = NULL;
    }
    if (d->dir)
    {
        destruir_carregador(d->dir);
        d->dir = NULL;
    }

    free(d);
}

void destruir_carregador(Carregador *c)
{
    // Libera todos os itens do carregador e em seguida libera a estrutura
    if (!c)
        return;
    void *item;
    while (!vazia(&c->carga))
    {
        pop(&c->carga, &item);
        free(item); // Assumindo que os itens são alocados dinamicamente
    }
    free(c);
}

void disparador_reportar_topo(Disparador *d, FILE *txt)
{
    /*
     * Reporta o item do topo do disparador no arquivo 'txt' sem consumir o item.
     * Se estiver vazio, escreve uma mensagem apropriada.
     */
    if (!d || !txt)
        return;
    if (vazia(&d->disparador))
    {
        fprintf(txt, "Topo do disparador %d: vazio\n", d->id);
        return;
    }

    Stack temp;
    inicializar(&temp);
    void *item = NULL;

    // Desempilha apenas o topo (preservando-o em temp) e imprime
    if (pop(&d->disparador, &item))
    {
        if (item)
        {
            // Garante que a linha tem newline ao final no TXT
            fprintf(txt, "Top do disparador %d: %s", d->id, (char *)item);
        }
        else
        {
            fprintf(txt, "Top do disparador %d: (item nulo)\n", d->id);
        }
        push(&temp, item);
    }

    // Restaura os itens da pilha temporária de volta ao disp
    while (!vazia(&temp))
    {
        void *t = NULL;
        pop(&temp, &t);
        push(&d->disparador, t);
    }
    liberar_pilha(&temp);
}

/* Reporta no arquivo 'txt' todas as figuras carregadas no Carregador,
 do topo para a base, sem consumir os itens.*/
void carregador_reportar_figuras(Carregador *c, FILE *txt)
{
    /*
Reporta todas as figuras no carregador (do topo para a base), sem
consumir os itens. O procedimento usa uma pilha temporária para
permitir contar e imprimir, restaurando a pilha original no final.
     */
    if (!c || !txt)
        return;

    Stack temp;
    inicializar(&temp);
    void *item = NULL;

    // Se a pilha estiver vazia, reporta
    if (vazia(&c->carga))
    {
        fprintf(txt, "Carregador %d: vazio\n", c->id);
        liberar_pilha(&temp);
        return;
    }

    // 1) Transfere todos os itens para 'temp' apenas para contar
    int count = 0;
    while (pop(&c->carga, &item))
    {
        push(&temp, item);
        count++;
        item = NULL;
    }

    // 2) Imprime cabeçalho com número de itens
    fprintf(txt, "Carregador %d: %d itens\n", c->id, count);

    // 3) Restaura da 'temp' para a pilha original e imprime cada item
    while (!vazia(&temp))
    {
        void *t = NULL;
        pop(&temp, &t); // t contém o próximo item do topo->base
        if (t)
            fprintf(txt, "  - %s", (char *)t);
        push(&c->carga, t);
    }
    liberar_pilha(&temp);
}

void *disparar(Disparador *d)
{
    /*
     * Tenta obter um item para disparo com a seguinte prioridade:
     * 1) pilha de disparo principal do próprio Disparador
     * 2) carregador esquerdo
     * 3) carregador direito
     * Retorna o ponteiro para o item ou NULL se não houver nada.
     */
    if (d == NULL)
    {
        return NULL; // Validação de segurança
    }

    void *item = NULL;

    // 1. Tenta obter da pilha de disparo principal
    if (!disparador_disp_vazia(d))
    {
        disparador_pop_disp(d, &item);
    }

    // 2. Se não conseguiu (item ainda é NULL), tenta obter do carregador esquerdo
    if (item == NULL)
    {
        Carregador *carregador_esq = disparador_get_carregador_esq(d);
        if (carregador_esq != NULL && !carregador_vazia(carregador_esq))
        {
            carregador_pop(carregador_esq, &item);
        }
    }

    // 3. Se ainda não conseguiu, tenta obter do carregador direito
    if (item == NULL)
    {
        Carregador *carregador_dir = disparador_get_carregador_dir(d);
        if (carregador_dir != NULL && !carregador_vazia(carregador_dir))
        {
            carregador_pop(carregador_dir, &item);
        }
    }

    // 4. Retorna o item encontrado, ou NULL se nada foi encontrado
    return item;
}
