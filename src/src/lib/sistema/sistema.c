#include "sistema.h"
#include <stdlib.h>
#include <string.h>

typedef struct
{
    int id;
    Stack formas;
} Loader;

typedef struct
{
    int id;
    double x, y;
    Loader *esq;
    Loader *dir;
    void *itemPronto;
} Shooter;

struct Sistema_t
{
    Shooter *shooters;
    int nShooters;
    Loader *loaders;
    int nLoaders;
};

Sistema sistema_criar()
{
    Sistema s = calloc(1, sizeof(struct Sistema_t));
    return s;
}

void sistema_destruir(Sistema s)
{
    for (int i = 0; i < s->nLoaders; i++)
        stack_destroy(s->loaders[i].formas);
    free(s->loaders);
    free(s->shooters);
    free(s);
}

void sistema_add_atirador(Sistema s, int id, double x, double y)
{
    s->shooters = realloc(s->shooters, (s->nShooters + 1) * sizeof(Shooter));
    s->shooters[s->nShooters++] = (Shooter){id, x, y, NULL, NULL, NULL};
}

void sistema_add_carregador(Sistema s, int id, int numFormas, Ground ground)
{
    s->loaders = realloc(s->loaders, (s->nLoaders + 1) * sizeof(Loader));
    Loader *l = &s->loaders[s->nLoaders++];
    l->id = id;
    l->formas = stack_create();
    Queue gq = get_ground_queue(ground);
    for (int i = 0; i < numFormas; i++)
    {
        void *f = queue_dequeue(gq);
        if (f)
            stack_push(l->formas, f);
    }
}

static Loader *find_loader(Sistema s, int id)
{
    for (int i = 0; i < s->nLoaders; i++)
        if (s->loaders[i].id == id)
            return &s->loaders[i];
    return NULL;
}

static Shooter *find_shooter(Sistema s, int id)
{
    for (int i = 0; i < s->nShooters; i++)
        if (s->shooters[i].id == id)
            return &s->shooters[i];
    return NULL;
}

void sistema_atch(Sistema s, int idAtirador, int idEsq, int idDir)
{
    Shooter *sh = find_shooter(s, idAtirador);
    if (sh)
    {
        sh->esq = find_loader(s, idEsq);
        sh->dir = find_loader(s, idDir);
    }
}

void sistema_shft(Sistema s, int idAtirador, const char *lado, int n)
{
    Shooter *sh = find_shooter(s, idAtirador);
    if (!sh)
        return;

    Loader *src = (strcmp(lado, "e") == 0) ? sh->esq : sh->dir;
    Loader *dst = (strcmp(lado, "e") == 0) ? sh->dir : sh->esq;

    for (int i = 0; i < n; i++)
    {
        if (src && !stack_is_empty(src->formas))
        {
            if (sh->itemPronto && dst)
                stack_push(dst->formas, sh->itemPronto);
            sh->itemPronto = stack_pop(src->formas);
        }
    }
}

void *sistema_preparar_disparo(Sistema s, int idAtirador, double *xOut, double *yOut)
{
    Shooter *sh = find_shooter(s, idAtirador);
    if (sh && sh->itemPronto)
    {
        *xOut = sh->x;
        *yOut = sh->y;
        void *item = sh->itemPronto;
        sh->itemPronto = NULL;
        return item;
    }
    return NULL;
}