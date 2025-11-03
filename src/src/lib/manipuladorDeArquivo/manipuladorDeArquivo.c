#include "manipuladorDeArquivo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct FileData_t
{
  Queue linesQueue;
  char *fileName;
} FileData_t;

static char *read_line(FILE *file, char *buffer, size_t size)
{
  if (fgets(buffer, size, file) != NULL)
  {
    // Remove newline if present
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n')
    {
      buffer[len - 1] = '\0';
    }
    return buffer;
  }
  return NULL;
}

FileData file_data_create(const char *filepath)
{
  /* Reuse readFile implementation to create and populate FileData from
     the provided filepath. If filepath is NULL, return NULL. */
  if (filepath == NULL)
    return NULL;

  return readFile(filepath);
}

static char *duplicate_string(const char *s)
{
  if (s == NULL)
    return NULL;

  size_t len = strlen(s) + 1;
  char *dup = malloc(len);
  if (dup != NULL)
  {
    strcpy(dup, s);
  }
  return dup;
}
static char *extract_filename_without_extension(const char *filepath)
{
  if (filepath == NULL)
    return NULL;

  const char *last_slash = strrchr(filepath, '/');
  const char *last_backslash = strrchr(filepath, '\\');
  const char *base = filepath;
  if (last_slash && last_backslash)
  {
    base = (last_slash > last_backslash) ? last_slash + 1 : last_backslash + 1;
  }
  else if (last_slash)
  {
    base = last_slash + 1;
  }
  else if (last_backslash)
  {
    base = last_backslash + 1;
  }

  size_t base_len = strlen(base);
  char *name_copy = malloc(base_len + 1);
  if (name_copy == NULL)
    return NULL;
  strcpy(name_copy, base);

  char *last_dot = strrchr(name_copy, '.');
  if (last_dot != NULL)
  {
    *last_dot = '\0';
  }
  return name_copy;
}
FileData readFile(const char *filepath)
{

  FileData_t *fileData = malloc(sizeof(FileData_t));
  if (fileData == NULL)
  {
    printf("Erro ao ler arquivo.");
    exit(1);
  }

  FILE *file = fopen(filepath, "r");
  if (file == NULL)
  {
    return NULL;
  }

  Queue lines = queue_create();
  if (lines == NULL)
  {
    printf("Erro ao criar fila.");
    exit(1);
  }
  char buffer[1024];

  while (read_line(file, buffer, sizeof(buffer)) != NULL)
  {
    queue_enqueue(lines, duplicate_string(buffer));
  }
  fclose(file);

  fileData->linesQueue = lines;
  fileData->fileName = extract_filename_without_extension(filepath);

  return (FileData)fileData;
}

Queue getLinesQueue(FileData fileData)
{
  if (fileData == NULL)
  {
    printf("Erro: filedata inválido.");
    exit(1);
  }
  FileData_t *fd = (FileData_t *)fileData;
  return fd->linesQueue;
}

void destroyFileData(FileData fileData)
{
  if (fileData == NULL)
  {
    return;
  }
  FileData_t *fd = (FileData_t *)fileData;
  if (fd->fileName != NULL)
  {
    free(fd->fileName);
  }
  free(fd);
  return;
}

char *getFileName(FileData fileData)
{
  if (fileData == NULL)
    return NULL;
  FileData_t *fd = (FileData_t *)fileData;
  return fd->fileName;
}
