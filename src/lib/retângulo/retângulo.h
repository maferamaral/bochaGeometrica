#ifndef RETANGULO_H
#define RETANGULO_H

typedef struct Rectangle Rectangle;

// Cria um novo retângulo e retorna um ponteiro para ele
void createRectangle(float x, float y, float w, float h, char *corb, char *corp, int id);

// Calcula a área de um retângulo
float areaRectangle(Rectangle *r);

// Retorna a coordenada X do retângulo
float getX_rec(Rectangle *r);

// Retorna a coordenada Y do retângulo
float getY_rec(Rectangle *r);

// Libera a memória de um retângulo
void deleteRectangle(Rectangle *r);

// Retorna o id do retângulo
int getID_rec(Rectangle *r);

// Retorna a cor de borda do retângulo
char *getCorb_rec(Rectangle *r);

// Retorna a cor de preenchimento do retângulo
char *getCorp_rec(Rectangle *r);

#endif // RETANGULO_H
