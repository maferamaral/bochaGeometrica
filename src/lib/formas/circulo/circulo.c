
#include "circulo.h"
#include "../../commons/utils/utils.h"
#include <stdlib.h>
#include <string.h>
/**
 * Internal Circle structure
 */
struct Circulo
{
    int id;
    double x;
    double y;
    double radius;
    char *border_color;
    char *fill_color;
};

void *circulo_criar(int id, double x, double y, double raio,
                    const char *cor_borda, const char *cor_preenchimento)
{
    if (!cor_borda || !cor_preenchimento)
    {
        return NULL;
    }

    struct Circulo *circulo = malloc(sizeof(struct Circulo));
    if (!circulo)
    {
        return NULL;
    }

    circulo->id = id;
    circulo->x = x;
    circulo->y = y;
    circulo->radius = raio;

    circulo->border_color = duplicate_string(cor_borda);
    if (!circulo->border_color)
    {
        free(circulo);
        return NULL;
    }

    circulo->fill_color = duplicate_string(cor_preenchimento);
    if (!circulo->fill_color)
    {
        free(circulo->border_color);
        free(circulo);
        return NULL;
    }

    return circulo;
}

void circulo_destruir(void *circulo)
{
    if (!circulo)
        return;

    struct Circulo *c = (struct Circulo *)circulo;
    free(c->border_color);
    free(c->fill_color);
    free(c);
}

int circulo_get_id(void *circulo)
{
    if (!circulo)
        return -1;
    return ((struct Circulo *)circulo)->id;
}

double circulo_get_x(void *circulo)
{
    if (!circulo)
        return 0.0;
    return ((struct Circulo *)circulo)->x;
}

double circulo_get_y(void *circulo)
{
    if (!circulo)
        return 0.0;
    return ((struct Circulo *)circulo)->y;
}

double circulo_get_raio(void *circulo)
{
    if (!circulo)
        return 0.0;
    return ((struct Circulo *)circulo)->radius;
}

const char *circulo_get_cor_borda(void *circulo)
{
    if (!circulo)
        return NULL;
    return ((struct Circulo *)circulo)->border_color;
}

const char *circulo_get_cor_preenchimento(void *circulo)
{
    if (!circulo)
        return NULL;
    return ((struct Circulo *)circulo)->fill_color;
}
