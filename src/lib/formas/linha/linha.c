#include "linha.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct
{
    int id;
    float x1, x2;
    float y1, y2;
    char cor[32];
} Line_t;

void createLine(int id, float x1, float x2, float y1, float y2, char *cor)
{
    if (!cor)
    {
        return NULL;
    }

    Line_t *l = (Line_t *)malloc(sizeof(Line_t));
    if (l == NULL)
    {
        printf("Erro ao alocar memória");
        exit(1);
    }
    id = l->id;
    x1 = l->x1;
    x2 = l->x2;
    y1 = l->y1;
    y2 = l->y2;

    l->cor = duplicate_string(cor);
    if (!l->cor)
    {
        free(l);
        return NULL;
    }
}

float lineLength(Line l)
{
    Line_t *linha = (Line *)l;
    if (!linha)
    {
        return -1;
    }
    float d = sqrt(pow((linha->x1 - linha->x2), 2) + pow((linha->y1 - linha->y2), 2));
    return d;
}

float lineArea(Line l)
{
    Line_t *linha = (Line *)l;
    if (!linha)
    {
        return -1;
    }
    float area = 2 * lineLength(linha);
    return area;
}

float getX1_line(Line l)
{
    Line_t *linha = (Line *)l;
    if (!linha)
    {
        return 0.0;
    }
    return linha->x1;
}

float getX2_line(Line l)
{
    Line_t *linha = (Line *)l;
    if (!linha)
    {
        return 0.0;
    }
    return linha->x2;
}

int getId_line(Line l)
{
    Line_t *linha = (Line *)l;
    if (!linha)
    {
        return -1;
    }
    return linha->id;
}

float getY1_line(Line l)
{
    Line_t *linha = (Line *)l;
    if (!linha)
    {
        return 0.0;
    }
    return linha->y1;
}

float getY2_line(Line l)
{
    Line_t *linha = (Line *)l;
    if (!linha)
    {
        return 0.0;
    }
    return linha->y2;
}

char *getCor_line(Line l)
{
    Line_t *linha = (Line *)l;
    if (!linha)
    {
        return NULL;
    }
    return linha->cor;
}

void deleteLine(Line *l)
{
    Line_t *linha = (Line_t *)l;
    if (!linha)
    {
        return;
    }
    free(linha->cor);
    free(linha);
}