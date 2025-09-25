#include "lib/arg/arg.h"
#include "lib/fila/fila.h"
#include "lib/manipuladorDeArquivo/manipuladorDeArquivo.h"

int main(int argc, char *argv[]){
    FileData* file = readFile("Makefile");
    Queue* lines = getLinesQueue(file);
    char* line = dequeue(lines);
    printf("%s", line);
    destroyFileData(file);
    return 0;
}