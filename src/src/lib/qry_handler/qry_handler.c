#include "qry_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../relatorio/relatorio.h"
#include "../sistema/sistema.h"
#include "../arena/arena.h"
#include "../manipuladorDeArquivo/manipuladorDeArquivo.h"

Qry executar_comandos_qry(FileData qryFile, FileData geoFile, Ground ground, const char *outPath)
{
  char *geoName = getFileName(geoFile);
  char *qryName = getFileName(qryFile);

  Relatorio relatorio = relatorio_criar(geoName, qryName, outPath);
  Sistema sistema = sistema_criar();
  Arena arena = arena_criar();

  Queue linhas = getLinesQueue(qryFile);
  int qtdCmds = 0;

  while (!queue_is_empty(linhas))
  {
    char *linha = queue_dequeue(linhas);
    char linhaCopy[256];
    strcpy(linhaCopy, linha);
    char *cmd = strtok(linha, " \t\n");
    if (!cmd)
      continue;

    qtdCmds++;

    if (strcmp(cmd, "pd") == 0)
    {
      int id = atoi(strtok(NULL, " "));
      double x = atof(strtok(NULL, " "));
      double y = atof(strtok(NULL, " "));
      sistema_add_atirador(sistema, id, x, y);
      relatorio_registrar_comando(relatorio, "pd", linhaCopy);
    }
    else if (strcmp(cmd, "lc") == 0)
    {
      int id = atoi(strtok(NULL, " "));
      int qtd = atoi(strtok(NULL, " "));
      sistema_add_carregador(sistema, id, qtd, ground);
      relatorio_registrar_comando(relatorio, "lc", linhaCopy);
    }
    else if (strcmp(cmd, "atch") == 0)
    {
      int idA = atoi(strtok(NULL, " "));
      int idE = atoi(strtok(NULL, " "));
      int idD = atoi(strtok(NULL, " "));
      sistema_atch(sistema, idA, idE, idD);
      relatorio_registrar_comando(relatorio, "atch", linhaCopy);
    }
    else if (strcmp(cmd, "shft") == 0)
    {
      int id = atoi(strtok(NULL, " "));
      char *lado = strtok(NULL, " ");
      int n = atoi(strtok(NULL, " "));
      sistema_shft(sistema, id, lado, n);
      relatorio_registrar_comando(relatorio, "shft", linhaCopy);
    }
    else if (strcmp(cmd, "dsp") == 0)
    {
      int id = atoi(strtok(NULL, " "));
      double dx = atof(strtok(NULL, " "));
      double dy = atof(strtok(NULL, " "));
      char *anot = strtok(NULL, " ");
      double sx, sy;
      void *forma = sistema_preparar_disparo(sistema, id, &sx, &sy);
      if (forma)
      {
        relatorio_incrementar_disparos(relatorio);
        arena_receber_disparo(arena, forma, sx + dx, sy + dy, sx, sy, (anot && strcmp(anot, "v") == 0));
      }
      relatorio_registrar_comando(relatorio, "dsp", linhaCopy);
    }
    else if (strcmp(cmd, "rjd") == 0)
    {
      int id = atoi(strtok(NULL, " "));
      char *lado = strtok(NULL, " ");
      double dx = atof(strtok(NULL, " "));
      double dy = atof(strtok(NULL, " "));
      double incX = atof(strtok(NULL, " "));
      double incY = atof(strtok(NULL, " "));

      int k = 0;
      while (1)
      {
        sistema_shft(sistema, id, lado, 1);
        double sx, sy;
        void *f = sistema_preparar_disparo(sistema, id, &sx, &sy);
        if (!f)
          break;

        relatorio_incrementar_disparos(relatorio);

        double finalX = sx + dx + (k * incX);
        double finalY = sy + dy + (k * incY);

        // CORREÇÃO AQUI: Passar 1 para ativar o desenho da linha vermelha
        arena_receber_disparo(arena, f, finalX, finalY, sx, sy, 1);
        k++;
      }
      relatorio_registrar_comando(relatorio, "rjd", linhaCopy);
    }
    else if (strcmp(cmd, "calc") == 0)
    {
      arena_processar_calc(arena, ground, relatorio);
      relatorio_registrar_comando(relatorio, "calc", "");
    }
    free(linha);
  }

  relatorio_escrever_final(relatorio, qtdCmds);
  relatorio_gerar_svg(relatorio, ground, arena);
  sistema_destruir(sistema);
  arena_destruir(arena);
  return (Qry)relatorio;
}

void destroy_qry_waste(Qry qry)
{
  relatorio_destruir((Relatorio)qry);
}