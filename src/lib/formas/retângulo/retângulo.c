#include "retângulo.h"
#include <stdlib.h>
#include <string.h>

typedef struct
{
    int id;
    float x;
    float y;
    float w;
    float h;
    char corb[32];
    char corp[32];
} Rectangle_t;

void createRectangle(float x, float y, float w, float h, char *corb, char *corp, int id)
{

    if (!corb || !corp)
    {
        return NULL;
    }
    Rectangle_t *r = (Rectangle_t *)malloc(sizeof(Rectangle_t));
    if (r == NULL)
    {
        printf("Erro ao alocar memória.");
        exit(1);
    }

    r->y = y;
    r->x = x;
    r->w = w;
    r->h = h;
    r->id = id;

    r->corb = duplicate_string(corb);
    if (!r->corb)
    {
        free(r);
        return NULL;
    }
    r->corp = duplicate_string(corp);
    if (!r->corp)
    {
        free(r->corb);
        free(r);

        return NULL;
    }
}

float areaRectangle(Rectangle r)
{
    Rectangle_t *retangulo = (Rectangle *)r;
    if (!retangulo)
    {
        return -1;
    }
    float area = retangulo->w * retangulo->h;
    return area;
}

float getW_rec(Rectangle r)
{
    Rectangle_t *retangulo = (Rectangle *)r;
    if (!retangulo)
    {
        return -1;
    }
    return retangulo->w;
}

float getH_rec(Rectangle r)
{
    Rectangle_t *retangulo = (Rectangle *)r;
    if (!retangulo)
    {
        return -1;
    }
    return retangulo->h;
}

float getX_rec(Rectangle r)
{
    Rectangle_t *retangulo = (Rectangle *)r;
    if (!retangulo)
    {
        return 0.0;
    }
    return retangulo->x;
}

float getY_rec(Rectangle r)
{
    Rectangle_t *retangulo = (Rectangle *)r;
    if (!retangulo)
    {
        return 0.0;
    }
    return retangulo->y;
}

int getID_rec(Rectangle r)
{
    Rectangle_t *retangulo = (Rectangle *)r;
    if (!retangulo)
    {
        return -1;
    }
    return retangulo->id;
}

char *getCorb_rec(Rectangle r)
{
    Rectangle_t *retangulo = (Rectangle *)r;
    if (!retangulo)
    {
        return NULL;
    }
    return retangulo->corb;
}

char *getCorp_rec(Rectangle r)
{
    Rectangle_t *retangulo = (Rectangle *)r;
    if (!retangulo)
    {
        return NULL;
    }
    return retangulo->corp;
}

void deleteRectangle(Rectangle r)
{
    Rectangle_t *retangulo = (Rectangle_t *)c;
    if (!retangulo)
    {
        return;
    }
    free(retangulo->corb);
    free(retangulo->corp);
    free(retangulo);
}