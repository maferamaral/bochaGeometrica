// #include "linha.h"
// #include <stdlib.h>
// #include <string.h>
// #include <math.h>

// typedef struct line
// {
//     int id;
//     float x1, x2;
//     float y1, y2;
//     char cor[32];
// } Line;

// void createLine(int id, float x1, float x2, float y1, float y2, char *cor)
// {
//     Line *l = (Line *)malloc(sizeof(Line));
//     if (l == NULL)
//     {
//         printf("Erro ao alocar memória");
//         exit(1);
//     }
//     id = l->id;
//     x1 = l->x1;
//     x2 = l->x2;
//     y1 = l->y1;
//     y2 = l->y2;

//     strcpy(l->cor, cor);
// }

// float lineLength(Line *l)
// {
//     float d = sqrt(pow((l->x1 - l->x2), 2) + pow((l->y1 - l->y2), 2));
//     return d;
// }

// float lineArea(Line *l)
// {
//     float area = 2 * lineLength(l);
//     return area;
// }

// float getX1_line(Line *l)
// {
//     return l->x1;
// }

// float getX2_line(Line *l)
// {
//     return l->x2;
// }

// int getId_line(Line *l)
// {
//     return l->id;
// }

// float getY1_line(Line *l)
// {
//     return l->y1;
// }

// float getY2_line(Line *l)
// {
//     return l->y2;
// }

// char *getCor_line(Line *l)
// {
//     return l->cor;
// }

// void deleteLine(Line *l)
// {
//     free(l);
// }