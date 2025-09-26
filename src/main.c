#include "lib/arg/arg.h"
#include "lib/fila/fila.h"
#include "lib/manipuladorDeArquivo/manipuladorDeArquivo.h"
#include "lib/arg/arg.h"
#include <stdio.h>

int main(int argc, char *argv[])
{

    const char *geoPath = getArgValue(argc, argv, "-f");
    const char *outputPath = getArgValue(argc, argv, "-o");
    const char *qryPath = getArgValue(argc, argv, "-q");
    printf("%s\n", qryPath);
    return 0;
}