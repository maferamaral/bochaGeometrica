// #include "circulo.h"
// #include <stdlib.h>
// #include <string.h>

// #ifndef PI
// #define PI 3.14159265358979323846
// #endif

// typedef struct
// {
//     int id;
//     float x;
//     float y;
//     float r;
//     char *corb;
//     char *corp;
// } Circle;

// void createCircle(float x, float y, float r, char *corb, char *corp, int id)
// {
//     Circle *c = (Circle *)malloc(sizeof(Circle));
//     if (c == NULL)
//     {
//         printf("Erro ao alocar memória.");
//         exit(1);
//     }

//     c->y = y;
//     c->x = x;
//     c->r = r;
//     c->id = id;

//     strcpy(c->corp, corp);

//     strcpy(c->corb, corb);
// }

// float areaCircle(Circle *c)
// {
//     float area = c->r * c->r * PI;
//     return area;
// }

// float getX_circle(Circle *c)
// {
//     return c->x;
// }

// float getY_circle(Circle *c)
// {
//     return c->y;
// }

// int getID_circle(Circle *c)
// {
//     return c->id;
// }

// char *getCorb_circle(Circle *c)
// {
//     return c->corb;
// }

// char *getCorp_circle(Circle *c)
// {
//     return c->corp;
// }

// void deleteCircle(Circle *c)
// {
//     free(c);
// }