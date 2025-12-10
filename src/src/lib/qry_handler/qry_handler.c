#include "qry_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../relatorio/relatorio.h"
#include "../sistema/sistema.h"
#include "../arena/arena.h"
#include "../manipuladorDeArquivo/manipuladorDeArquivo.h"
#include "../geo_handler/geo_handler.h"

Qry executar_comandos_qry(FileData qryFile, FileData geoFile, Ground ground, const char *outPath)
{
  char *geoName = getFileName(geoFile);
  char *qryName = getFileName(qryFile);

  Relatorio relatorio = relatorio_criar(geoName, qryName, outPath);
  FILE *txt = relatorio_get_txt(relatorio); // Obtem ponteiro para escrever no TXT

  Sistema sistema = sistema_criar();
  Arena arena = arena_criar();

  Queue linhas = getLinesQueue(qryFile);
  int qtdCmds = 0;

  while (!queue_is_empty(linhas))
  {
    char *linha = queue_dequeue(linhas);

    // Reporta o comando lido no TXT
    if (txt)
      fprintf(txt, "\n[*] %s\n", linha);

    char *cmd = strtok(linha, " \t\n");
    if (!cmd)
    {
      free(linha);
      continue;
    }

    qtdCmds++;

    if (strcmp(cmd, "pd") == 0)
    {
      int id = atoi(strtok(NULL, " "));
      double x = atof(strtok(NULL, " "));
      double y = atof(strtok(NULL, " "));
      sistema_add_atirador(sistema, id, x, y);
    }
    else if (strcmp(cmd, "lc") == 0)
    {
      int id = atoi(strtok(NULL, " "));
      int qtd = atoi(strtok(NULL, " "));
      sistema_add_carregador(sistema, id, qtd, ground);
    }
    else if (strcmp(cmd, "atch") == 0)
    {
      int idA = atoi(strtok(NULL, " "));
      int idE = atoi(strtok(NULL, " "));
      int idD = atoi(strtok(NULL, " "));
      sistema_atch(sistema, idA, idE, idD);
    }
    else if (strcmp(cmd, "shft") == 0)
    {
      int id = atoi(strtok(NULL, " "));
      char *lado = strtok(NULL, " ");
      int n = atoi(strtok(NULL, " "));
      sistema_shft(sistema, id, lado, n);
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

        // Reporta dados no TXT
        if (txt)
          fprintf(txt, "-> Disparo realizado. Dados da forma:\n");
        geo_imprimir_forma_txt(forma, txt);
        if (txt)
          fprintf(txt, "   Posicao Final: (%.2f, %.2f)\n", sx + dx, sy + dy);

        // Apenas anota no SVG se o parâmetro for 'v'
        int anotar = (anot && strcmp(anot, "v") == 0);
        arena_receber_disparo(arena, forma, sx + dx, sy + dy, sx, sy, anotar);
      }
      else
      {
        if (txt)
          fprintf(txt, "-> Falha no disparo: sem municao.\n");
      }
    }
    else if (strcmp(cmd, "rjd") == 0)
    {
      int id = atoi(strtok(NULL, " "));
      char *lado = strtok(NULL, " ");
      double dx = atof(strtok(NULL, " "));
      double dy = atof(strtok(NULL, " "));
      double incX = atof(strtok(NULL, " "));
      double incY = atof(strtok(NULL, " "));

      if (txt)
        fprintf(txt, "-> Rajada iniciada (Disp: %d, Lado: %s):\n", id, lado);

      int k = 0;
      while (1)
      {
        // 1. Puxa munição (shift 1)
        sistema_shft(sistema, id, lado, 1);

        // 2. Prepara disparo
        double sx, sy;
        void *f = sistema_preparar_disparo(sistema, id, &sx, &sy);
        if (!f)
          break; // Acabou a munição

        relatorio_incrementar_disparos(relatorio);

        double finalX = sx + dx + (k * incX);
        double finalY = sy + dy + (k * incY);

        // Reporta cada item da rajada no TXT
        geo_imprimir_forma_txt(f, txt);

        // Adiciona na arena com anotação DESLIGADA (0) para não poluir o SVG
        arena_receber_disparo(arena, f, finalX, finalY, sx, sy, 0);
        k++;
      }
      if (txt)
        fprintf(txt, "-> Fim da Rajada. Total: %d formas.\n", k);
    }
    else if (strcmp(cmd, "calc") == 0)
    {
      arena_processar_calc(arena, ground, relatorio);
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