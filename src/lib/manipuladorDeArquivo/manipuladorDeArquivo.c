#include "../fila/fila.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct FileData {
    Queue *linesQueue;
} FileData;

static char *read_line(FILE *file, char *buffer, size_t size) {
  if (fgets(buffer, size, file) != NULL) {
    // Remove newline if present
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
      buffer[len - 1] = '\0';
    }
    return buffer;
  }
  return NULL;
}

static char *duplicate_string(const char *s) {
  if (s == NULL)
    return NULL;

  size_t len = strlen(s) + 1;
  char *dup = malloc(len);
  if (dup != NULL) {
    strcpy(dup, s);
  }
  return dup;
}
FileData* readFile(const char* filepath){

    FileData* fileData = malloc(sizeof(FileData));
    if(fileData==NULL){
        printf("Erro ao ler arquivo.");
        exit(1);
    }

    FILE* file = fopen(filepath, "r");
    if(file==NULL){
        return NULL;
    }

    Queue* lines = createQueue();
    if(lines == NULL){
        printf("Erro ao criar fila.");
        exit(1);
    }
    char buffer[1024];

    while (read_line(file, buffer, sizeof(buffer)) != NULL) {
    enqueue(lines, duplicate_string(buffer));
  }
    fclose(file);

    fileData->linesQueue = lines;

    return fileData;
}

Queue* getLinesQueue(FileData* fileData){
    if(fileData==NULL){
        printf("Erro: filedata inválido.");
        exit(1);
    }
    return fileData->linesQueue;
}

void destroyFileData(FileData* fileData){
    if(fileData==NULL){
        return;
    }
    free(fileData);
    return;
}
