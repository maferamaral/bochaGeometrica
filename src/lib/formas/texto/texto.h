#ifndef TEXTO_H
#define TEXTO_H

// Estruturas opacas
typedef struct Texto Texto;
typedef struct TextStyle TextStyle;

// Cria um novo Texto alocado dinamicamente e retorna o ponteiro.
// Propriedade das strings passadas é copiada internamente.
Texto *createText(int id, int x, int y, char a, const char *corb, const char *corp, const char *txto, const char *fFamily, const char *fWeight, const char *fSize);

int getID_txt(Texto *t);
int getX_txt(Texto *t);
int getY_txt(Texto *t);
char getA_txt(Texto *t);
char *getCorb_txt(Texto *t);
char *getCorp_txt(Texto *t);
char *getTxto_txt(Texto *t);

// Funções para TextStyle
char *getFF_txt(Texto *t);
char *getFW_txt(Texto *t);
char *getFS_txt(Texto *t);

void deleteTxt(Texto *t);

#endif // TEXTO_H