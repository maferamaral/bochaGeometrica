#ifndef CIRCULO_H
#define CIRCULO_H

#include <stdio.h>

// Estrutura opaca para Circle
typedef struct Circle Circle;

// Cria um novo círculo e retorna um ponteiro para ele
void createCircle(float x, float y, float r, char *corb, char *corp, int id);

// Calcula a área de um círculo
float areaCircle(Circle *c);

// Retorna a coordenada X do círculo
float getX_circle(Circle *c);

// Retorna a coordenada Y do círculo
float getY_circle(Circle *c);

// Libera a memória de um círculo
void deleteCircle(Circle *c);

// Retorna o id do círculo
int getID_circle(Circle *c);

// Retorna a cor de borda do círculo
char *getCorb_circle(Circle *c);

// Retorna a cor de preenchimento do círculo
char *getCorp_circle(Circle *c);

#endif // CIRCULO_H
