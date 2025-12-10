#include "lib/arg/arg.h"
#include "lib/fila/fila.h"
#include "lib/geo_handler/geo_handler.h"
#include "lib/manipuladorDeArquivo/manipuladorDeArquivo.h"
#include "lib/qry_handler/qry_handler.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
  if (argc > 10)
  { // program -e path -f .geo -o output -q .qry suffix
    printf("Erro: muitos argumentos\n");
    exit(1);
  }

  // Pegar argumentos
  const char *geoPath = getArgValue(argc, argv, "-f");
  const char *outputPath = getArgValue(argc, argv, "-o");
  const char *prefixPath = getArgValue(argc, argv, "-e");
  const char *qryPath = getArgValue(argc, argv, "-q");

  // Verificar argumentos necessários
  if (geoPath == NULL || outputPath == NULL)
  {
    printf("Erro ao abrir arquivo geo");
    exit(1);
  }

  // Preparar caminhos completos com prefixo se existir
  char *fullGeoPath = (char *)geoPath;
  if (prefixPath != NULL)
  {
    size_t prefixLen = strlen(prefixPath);
    size_t geoLen = strlen(geoPath);
    fullGeoPath = malloc(prefixLen + geoLen + 2); // +2 para possível / e \0
    strcpy(fullGeoPath, prefixPath);

    // Adicionar / se necessário
    if (prefixPath[prefixLen - 1] != '/' && geoPath[0] != '/')
    {
      strcat(fullGeoPath, "/");
    }
    strcat(fullGeoPath, geoPath);
  }

  // Colocar as linhas do arquivo na fila
  FileData geo_file = readFile(fullGeoPath);
  if (geo_file == NULL)
  {
    printf("Erro ao abrir arquivo geo.");
    if (fullGeoPath != geoPath)
      free(fullGeoPath);
    exit(1);
  }

  if (fullGeoPath != geoPath)
    free(fullGeoPath);

  // Executar essas linhas
  Ground ground = execute_geo_commands(geo_file, outputPath, NULL);

  if (qryPath != NULL)
  {
    char *fullQryPath = (char *)qryPath;
    if (prefixPath != NULL)
    {
      size_t prefixLen = strlen(prefixPath);
      size_t qryLen = strlen(qryPath);
      fullQryPath = malloc(prefixLen + qryLen + 2); // +2 para possível / e \0
      strcpy(fullQryPath, prefixPath);

      // Adicionar / se necessário
      if (prefixPath[prefixLen - 1] != '/' && qryPath[0] != '/')
      {
        strcat(fullQryPath, "/");
      }
      strcat(fullQryPath, qryPath);
    }

    FileData qry_file = file_data_create(fullQryPath);
    if (!qry_file)
    {
      printf("Erro ao criar arquivo .qry\n");
      if (fullQryPath != qryPath)
        free(fullQryPath);
      destroy_geo_waste(ground);
      exit(1);
    }
    Qry qry = executar_comandos_qry(qry_file, geo_file, ground, outputPath);

    if (fullQryPath != qryPath)
      free(fullQryPath);
    destroyFileData(qry_file);
    destroy_qry_waste(qry);
  }

  destroyFileData(geo_file);
  destroy_geo_waste(ground);

  return 0;
}