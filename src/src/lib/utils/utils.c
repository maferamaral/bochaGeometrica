#include "utils.h"
#include <math.h>

/**
 * Duplicates a string using malloc
 * @param s Source string to duplicate
 * @return New string or NULL on error
 */
char *duplicate_string(const char *s)
{
    if (s == NULL)
        return NULL;

    size_t len = strlen(s) + 1;
    char *dup = malloc(len);
    if (dup != NULL)
    {
        strcpy(dup, s);
    }
    return dup;
}

/**
 * Calcula a distância euclidiana entre dois pontos (x1,y1) e (x2,y2)
 */
double distancia(double x1, double y1, double x2, double y2)
{
    double dx = x2 - x1;
    double dy = y2 - y1;
    return sqrt(dx * dx + dy * dy);
}

/**
 * Inverte uma cor (troca preto por branco e vice-versa)
 */
char *invert_color(const char *color)
{
    if (color == NULL)
        return NULL;

    char *inverted = duplicate_string(color);
    if (inverted == NULL)
        return NULL;

    // Troca preto por branco e vice-versa
    if (strcmp(color, "black") == 0)
    {
        strcpy(inverted, "white");
    }
    else if (strcmp(color, "white") == 0)
    {
        strcpy(inverted, "black");
    }
    else if (strcmp(color, "#000000") == 0)
    {
        strcpy(inverted, "#FFFFFF");
    }
    else if (strcmp(color, "#FFFFFF") == 0)
    {
        strcpy(inverted, "#000000");
    }
    // Para outras cores, mantém a original

    return inverted;
}