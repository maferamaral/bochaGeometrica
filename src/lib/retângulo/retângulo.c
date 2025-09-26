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
} Rectangle;

void createRectangle(float x, float y, float w, float h, char *corb, char *corp, int id)
{
    Rectangle *r = (Rectangle *)malloc(sizeof(Rectangle));
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

    strcpy(r->corp, corp);

    strcpy(r->corb, corb);
}

float areaRectangle(Rectangle *r)
{
    float area = r->w * r->h;
    return area;
}

float getX_rec(Rectangle *r)
{
    return r->x;
}

float getY_rec(Rectangle *r)
{
    return r->y;
}

void deleteRectangle(Rectangle *r)
{
    free(r);
}

int getID_rec(Rectangle *r)
{
    return r->id;
}

char *getCorb_rec(Rectangle *r)
{
    return r->corb;
}

char *getCorp_rec(Rectangle *r)
{
    return r->corp;
}