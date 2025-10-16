#ifndef RETANGULO_H
#define RETANGULO_H

// Estrutura opaca para representar um retângulo
typedef void *Rectangle;

// Cria um novo retângulo com as dimensões e cores especificadas
// Parâmetros:
//   x, y: coordenadas do ponto superior esquerdo
//   w, h: largura e altura do retângulo
//   corb: cor da borda
//   corp: cor do preenchimento
//   id: identificador único do retângulo
// Retorna: ponteiro para o retângulo criado ou NULL em caso de erro
Rectangle createRectangle(float x, float y, float w, float h, char *corb, char *corp, int id);

// Calcula a área do retângulo (largura * altura)
// Parâmetros:
//   r: ponteiro para o retângulo
// Retorna: área do retângulo em unidades quadradas ou -1 em caso de erro
float areaRectangle(Rectangle r);

// Retorna a coordenada X do ponto superior esquerdo do retângulo
// Retorna 0.0 se o ponteiro for inválido
float getX_rec(Rectangle r);

// Retorna a coordenada Y do ponto superior esquerdo do retângulo
// Retorna 0.0 se o ponteiro for inválido
float getY_rec(Rectangle r);

// Retorna a largura do retângulo
// Retorna -1 se o ponteiro for inválido
float getW_rec(Rectangle r);

// Retorna a altura do retângulo
// Retorna -1 se o ponteiro for inválido
float getH_rec(Rectangle r);

// Libera toda a memória alocada para o retângulo
void deleteRectangle(Rectangle r);

// Retorna o identificador único do retângulo
// Retorna -1 se o ponteiro for inválido
int getID_rec(Rectangle r);

// Retorna a cor da borda do retângulo
// O valor retornado não deve ser modificado ou liberado
// Retorna NULL se o ponteiro for inválido
char *getCorb_rec(Rectangle r);

// Retorna a cor de preenchimento do retângulo
// O valor retornado não deve ser modificado ou liberado
// Retorna NULL se o ponteiro for inválido
char *getCorp_rec(Rectangle r);

#endif // RETANGULO_H
