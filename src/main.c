#include "lib/arg/arg.h"
#include "lib/fila/fila.h"
#include "lib/geo_handler/geo_handler.h"
#include "lib/manipuladorDeArquivo/manipuladorDeArquivo.h"
#include "lib/qry_handler/qry_handler.h"
#include <stdio.h>

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
  const char *qryPath = getArgValue(argc, argv, "-q");

  // Verificar argumentos necessários
  if (geoPath == NULL || outputPath == NULL)
  {
    printf("Erro ao abrir arquivo geo");
    exit(1);
  }

  // Colocar as linhas do arquivo na fila
  FileData geo_file = readFile(geoPath);
  if (geo_file == NULL)
  {
    printf("Erro ao abrir arquivo geo.");
    exit(1);
  }

  // Executar essas linhas
  Ground ground = execute_geo_commands(geo_file, outputPath, NULL);

  if (qryPath != NULL)
  {
    FileData qry_file = file_data_create(qryPath);
    if (!qry_file)
    {
      printf("Erro ao criar arquivo .qry\n");
      destroy_geo_waste(ground);
      exit(1);
    }
    Qry qry = execute_qry_commands(qry_file, geo_file, ground, outputPath);
    destroyFileData(qry_file);
    destroy_qry_waste(qry);
  }

  destroyFileData(geo_file);
  destroy_geo_waste(ground);

  return 0;
}