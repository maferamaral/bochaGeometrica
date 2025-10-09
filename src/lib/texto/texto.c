#include "texto.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

struct TextStyle
{
    char *fFamily;
    char *fWeight;
    char *fSize;
};

struct Texto
{
    int id;
    int x;
    int y;
    char a;
    char *corb;
    char *corp;
    char *txto;
    TextStyle *textStyle;
};

// Helper: duplica string com alocação
static char *safe_strdup(const char *s)
{
    if (s == NULL)
        return NULL;
    size_t len = strlen(s) + 1;
    char *d = (char *)malloc(len);
    if (d == NULL)
    {
        printf("Erro ao alocar memória.\n");
        exit(1);
    }
    memcpy(d, s, len);
    return d;
}

Texto *createText(int id, int x, int y, char a, const char *corb, const char *corp, const char *txto, const char *fFamily, const char *fWeight, const char *fSize)
{
    Texto *t = (Texto *)malloc(sizeof(Texto));
    if (t == NULL)
    {
        printf("Erro ao alocar memória.\n");
        exit(1);
    }

    t->id = id;
    t->x = x;
    t->y = y;
    t->a = a;

    t->corb = safe_strdup(corb);
    t->corp = safe_strdup(corp);
    t->txto = safe_strdup(txto);

    t->textStyle = (TextStyle *)malloc(sizeof(TextStyle));
    if (t->textStyle == NULL)
    {
        printf("Erro ao alocar memória.\n");
        // liberar campos já alocados
        free(t->corb);
        free(t->corp);
        free(t->txto);
        free(t);
        exit(1);
    }

    t->textStyle->fFamily = safe_strdup(fFamily);
    t->textStyle->fWeight = safe_strdup(fWeight);
    t->textStyle->fSize = safe_strdup(fSize);

    return t;
}

int getID_txt(Texto *t)
{
    return t ? t->id : -1;
}

int getX_txt(Texto *t)
{
    return t ? t->x : 0;
}

int getY_txt(Texto *t)
{
    return t ? t->y : 0;
}

char getA_txt(Texto *t)
{
    return t ? t->a : '\0';
}

char *getCorb_txt(Texto *t)
{
    return t ? t->corb : NULL;
}

char *getCorp_txt(Texto *t)
{
    return t ? t->corp : NULL;
}

char *getTxto_txt(Texto *t)
{
    return t ? t->txto : NULL;
}

char *getFF_txt(Texto *t)
{
    return (t && t->textStyle) ? t->textStyle->fFamily : NULL;
}

char *getFW_txt(Texto *t)
{
    return (t && t->textStyle) ? t->textStyle->fWeight : NULL;
}

char *getFS_txt(Texto *t)
{
    return (t && t->textStyle) ? t->textStyle->fSize : NULL;
}

void deleteTxt(Texto *t)
{
    if (t == NULL)
        return;
    if (t->textStyle)
    {
        free(t->textStyle->fFamily);
        free(t->textStyle->fWeight);
        free(t->textStyle->fSize);
        free(t->textStyle);
    }
    free(t->corb);
    free(t->corp);
    free(t->txto);
    free(t);
}