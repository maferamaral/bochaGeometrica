/*
 * General utility functions
 */
#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <string.h>

/**
 * Duplicates a string using malloc
 * @param s Source string to duplicate
 * @return New string or NULL on error
 */
char *duplicate_string(const char *s);

/**
 * Calcula a distância euclidiana entre dois pontos (x1,y1) e (x2,y2)
 * @return distância (double)
 */
double distancia(double x1, double y1, double x2, double y2);

/**
 * Inverte uma cor (troca preto por branco e vice-versa)
 * @param color Cor original
 * @return Nova cor invertida ou NULL em caso de erro
 */
char *invert_color(const char *color);

#endif // UTILS_H