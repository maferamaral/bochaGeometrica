#include "circulo.h"
#include <stdlib.h>
#include <string.h>
#include "../manipuladorDeArquivo/manipuladorDeArquivo.h"

#ifndef PI
#define PI 3.14159265358979323846
#endif

typedef struct
{
    int id;
    float x;
    float y;
    float r;
    char *corb;
    char *corp;
} Circle_t;

void *createCircle(float x, float y, float r, char *corb, char *corp, int id)
{
    if (!corb || !corp)
    {
        return NULL;
    }

    Circle_t *c = (Circle_t *)malloc(sizeof(Circle_t));
    if (c == NULL)
    {
        printf("Erro ao alocar memória.");
        return NULL;
    }

    c->y = y;
    c->x = x;
    c->r = r;
    c->id = id;

    c->corb = duplicate_string(corb);
    if (!c->corb)
    {
        free(c);
        return NULL;
    }
    c->corp = duplicate_string(corp);
    if (!c->corp)
    {
        free(c->corb);
        free(c);

        return NULL;
    }
}

float areaCircle(Circle c)
{
    Circle_t *circle = (Circle_t *)c;
    float area = circle->r * circle->r * PI;
    return area;
}

float getX_circle(Circle c)
{
    Circle_t *circle = (Circle_t *)c;
    if (!circle)
    {
        return 0.0;
    }
    return circle->x;
}

float getY_circle(Circle c)
{
    Circle_t *circle = (Circle_t *)c;
    if (!circle)
    {
        return 0.0;
    }
    return circle->y;
}

float getR_circle(Circle c)
{
    Circle_t *circle = (Circle_t *)c;
    if (!circle)
    {
        return -1;
    }
    return circle->r;
}

int getID_circle(Circle c)
{
    Circle_t *circle = (Circle_t *)c;
    if (!circle)
    {
        return -1;
    }
    return circle->id;
}

char *getCorb_circle(Circle c)
{
    Circle_t *circle = (Circle_t *)c;
    if (!circle)
    {
        return NULL;
    }
    return circle->corb;
}

char *getCorp_circle(Circle c)
{
    Circle_t *circle = (Circle_t *)c;
    if (!circle)
    {
        return NULL;
    }
    return circle->corp;
}

void deleteCircle(Circle c)
{
    Circle_t *circle = (Circle_t *)c;
    if (!circle)
    {
        return;
    }
    free(circle->corb);
    free(circle->corp);
    free(circle);
}