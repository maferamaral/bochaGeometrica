#include "lib/arg/arg.h"
#include "lib/fila/fila.h"
#include "lib/manipuladorDeArquivo/manipuladorDeArquivo.h"
#include "lib/arg/arg.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
    // Pegar argumentos
    const char *geoPath = getArgValue(argc, argv, "-f");
    const char *outputPath = getArgValue(argc, argv, "-o");
    const char *qryPath = getArgValue(argc, argv, "-q");

    // Verificar argumentos necessários
    if (geoPath == NULL || outputPath == NULL)
    {
        printf("Erro ao alocar memoria");
        exit(1);
    }

    // Colocar as linhas do arquivo na fila
    FileData geo_file = readFile(geoPath);

    // Executar essas linhas

    return 0;
}