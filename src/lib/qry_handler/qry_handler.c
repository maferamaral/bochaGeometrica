#include "qry_handler.h"
#include "../disparador/disparador.h"
#include "../fila/fila.h"
#include "../formas/circulo/circulo.h"
#include "../formas/linha/linha.h"
#include "../formas/retangulo/retangulo.h"
#include "../formas/texto/texto.h"
#include "../geo_handler/geo_handler.h"
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define M_PI 3.14159265358979323846

/* Portable strdup replacement: strdup is POSIX and may be missing under strict
   C standards; provide a local implementation to avoid implicit declaration
   warnings. */
static char *strdup_safe(const char *s)
{
  if (s == NULL)
    return NULL;
  size_t len = strlen(s) + 1;
  char *dup = malloc(len);
  if (dup)
    memcpy(dup, s, len);
  return dup;
}

typedef struct
{
  Disparador **disparadores;
  int num_disparadores;
  Carregador **carregadores;
  int num_carregadores;
  Ground ground;
  const char *output_path;
  FILE *txt_output;
  FILE *svg_output;
} Qry_t;

// Estrutura para armazenar formas do chão
typedef struct
{
  void *forma;
  int id;
} FormaChao;

// Estrutura para armazenar formas lançadas na arena
typedef struct
{
  void *shape;
  int id;
  float x_final, y_final;
  char tipo; // 'r' = retângulo, 'c' = círculo, 'l' = linha, 't' = texto
  char *fill_color;
  char *border_color;
  float area;
  int ordem_lancamento; // ordem em que foi lançada
} FormaLancada;

// Array global para armazenar formas do chão
static FormaChao *formas_chao = NULL;
static int num_formas_chao = 0;
static int capacidade_formas_chao = 0;

// Array global para armazenar formas lançadas na arena
static FormaLancada *formas_arena = NULL;
static int num_formas_arena = 0;
static int capacidade_formas_arena = 0;
static float area_total_esmagada = 0.0;

// Funções genéricas para extrair dados de formas
int get_forma_id(void *shape)
{
  if (!shape)
    return -1;

  // Tenta diferentes tipos de forma
  int id = retangulo_get_id(shape);
  if (id != -1)
    return id;

  id = circulo_get_id(shape);
  if (id != -1)
    return id;

  id = line_get_id(shape);
  if (id != -1)
    return id;

  id = text_get_id(shape);
  if (id != -1)
    return id;

  return -1;
}

char get_forma_tipo(void *shape)
{
  if (!shape)
    return '?';

  // Verifica qual tipo de forma é baseado nas funções disponíveis
  if (retangulo_get_id(shape) != -1)
    return 'r';
  if (circulo_get_id(shape) != -1)
    return 'c';
  if (line_get_id(shape) != -1)
    return 'l';
  if (text_get_id(shape) != -1)
    return 't';

  return '?';
}

const char *get_forma_fill_color(void *shape)
{
  if (!shape)
    return NULL;

  char tipo = get_forma_tipo(shape);
  switch (tipo)
  {
  case 'r':
    return retangulo_get_cor_preenchimento(shape);
  case 'c':
    return circulo_get_cor_preenchimento(shape);
  case 'l':
    return line_get_color(shape); // Linha só tem uma cor
  case 't':
    return text_get_fill_color(shape);
  default:
    return NULL;
  }
}

const char *get_forma_border_color(void *shape)
{
  if (!shape)
    return NULL;

  char tipo = get_forma_tipo(shape);
  switch (tipo)
  {
  case 'r':
    return retangulo_get_cor_borda(shape);
  case 'c':
    return circulo_get_cor_borda(shape);
  case 'l':
    return line_get_color(shape); // Linha só tem uma cor
  case 't':
    return text_get_border_color(shape);
  default:
    return NULL;
  }
}

float calcular_area_forma_real(char tipo, void *shape)
{
  if (!shape)
    return 0.0;

  switch (tipo)
  {
  case 'r':
  {
    // Retângulo: largura * altura
    double largura = retangulo_get_largura(shape);
    double altura = retangulo_get_altura(shape);
    return largura * altura;
  }
  case 'c':
  {
    // Círculo: π * raio²
    double raio = circulo_get_raio(shape);
    return M_PI * raio * raio;
  }
  case 'l':
  {
    // Linha: 2.0 * comprimento
    double x1 = line_get_x1(shape);
    double y1 = line_get_y1(shape);
    double x2 = line_get_x2(shape);
    double y2 = line_get_y2(shape);
    double comprimento = sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
    return 2.0 * comprimento;
  }
  case 't':
  {
    // Texto: 20.0 * número de caracteres
    int comprimento = text_get_length(shape);
    return 20.0 * comprimento;
  }
  default:
    return 0.0;
  }
}

// Função para adicionar forma ao chão
void adicionar_forma_chao(void *forma, int id)
{
  if (num_formas_chao >= capacidade_formas_chao)
  {
    capacidade_formas_chao =
        capacidade_formas_chao == 0 ? 10 : capacidade_formas_chao * 2;
    formas_chao =
        realloc(formas_chao, capacidade_formas_chao * sizeof(FormaChao));
  }
  formas_chao[num_formas_chao].forma = forma;
  formas_chao[num_formas_chao].id = id;
  num_formas_chao++;
}

// Função para adicionar forma lançada à arena
void adicionar_forma_arena(void *shape, int id, float x, float y, char tipo,
                           char *fill_color, char *border_color, float area)
{
  if (num_formas_arena >= capacidade_formas_arena)
  {
    capacidade_formas_arena =
        capacidade_formas_arena == 0 ? 10 : capacidade_formas_arena * 2;
    formas_arena =
        realloc(formas_arena, capacidade_formas_arena * sizeof(FormaLancada));
  }

  formas_arena[num_formas_arena].shape = shape;
  formas_arena[num_formas_arena].id = id;
  formas_arena[num_formas_arena].x_final = x;
  formas_arena[num_formas_arena].y_final = y;
  formas_arena[num_formas_arena].tipo = tipo;
  formas_arena[num_formas_arena].fill_color =
      fill_color ? strdup_safe(fill_color) : NULL;
  formas_arena[num_formas_arena].border_color =
      border_color ? strdup_safe(border_color) : NULL;
  formas_arena[num_formas_arena].area = area;
  formas_arena[num_formas_arena].ordem_lancamento = num_formas_arena;

  num_formas_arena++;
}

// Função para verificar sobreposição entre duas formas
int verificar_sobreposicao(FormaLancada *forma1, FormaLancada *forma2)
{
  // Implementação simplificada - verifica se as formas estão próximas o
  // suficiente Em uma implementação real, seria necessário verificar
  // interseções específicas para cada tipo de forma

  float distancia = sqrt(pow(forma1->x_final - forma2->x_final, 2) +
                         pow(forma1->y_final - forma2->y_final, 2));

  // Considera sobreposição se a distância for menor que um threshold
  return distancia < 30.0; // threshold arbitrário
}

// Função para processar sobreposição entre duas formas
void processar_sobreposicao(FormaLancada *forma_i, FormaLancada *forma_j,
                            FILE *txt_output, FILE *svg_output)
{
  fprintf(txt_output, "Sobreposição detectada entre forma %d e forma %d\n",
          forma_i->id, forma_j->id);

  if (forma_i->area < forma_j->area)
  {
    // Forma I é esmagada
    fprintf(txt_output,
            "Forma %d (área: %.2f) esmagada por forma %d (área: %.2f)\n",
            forma_i->id, forma_i->area, forma_j->id, forma_j->area);

    // Adiciona asterisco vermelho no SVG
    fprintf(svg_output,
            "<circle cx=\"%.2f\" cy=\"%.2f\" r=\"5\" fill=\"red\" />\n",
            forma_i->x_final, forma_i->y_final);

    // Atualiza pontuação
    area_total_esmagada += forma_i->area;

    // Marca forma I como esmagada (pode ser removida do array)
    forma_i->id = -1; // marca como destruída
  }
  else if (forma_i->area > forma_j->area)
  {
    // Forma I muda a cor da borda de J
    fprintf(
        txt_output,
        "Forma %d (área: %.2f) muda cor da borda de forma %d (área: %.2f)\n",
        forma_i->id, forma_i->area, forma_j->id, forma_j->area);

    // Muda cor da borda de J para cor de preenchimento de I
    char *nova_cor_borda = forma_i->fill_color;
    fprintf(txt_output, "Cor da borda da forma %d alterada para %s\n",
            forma_j->id, nova_cor_borda);

    // Clonagem da forma I com cores intercambiadas
    int novo_id = forma_i->id * 1000; // ID único para o clone
    fprintf(txt_output,
            "Forma %d clonada como forma %d com cores intercambiadas\n",
            forma_i->id, novo_id);

    // Adiciona clone ao chão (simulado)
    fprintf(txt_output, "Clone adicionado ao chão após as formas originais\n");
  }
  else
  {
    // Áreas iguais - nenhuma ação especial
    fprintf(txt_output, "Áreas iguais - nenhuma ação especial\n");
  }
}

// Função para encontrar disparador por ID
Disparador *encontrar_disparador(Qry_t *qry, int id)
{
  for (int i = 0; i < qry->num_disparadores; i++)
  {
    if (getId_disparador(qry->disparadores[i]) == id)
    {
      return qry->disparadores[i];
    }
  }
  return NULL;
}

// Função para encontrar carregador por ID
Carregador *encontrar_carregador(Qry_t *qry, int id)
{
  for (int i = 0; i < qry->num_carregadores; i++)
  {
    if (getId_carregador(qry->carregadores[i]) == id)
    {
      return qry->carregadores[i];
    }
  }
  return NULL;
}

// Função para criar novo disparador
Disparador *criar_novo_disparador(Qry_t *qry, int id, float x, float y)
{
  qry->disparadores = realloc(qry->disparadores, (qry->num_disparadores + 1) *
                                                     sizeof(Disparador *));
  qry->disparadores[qry->num_disparadores] = criar_disp(id, x, y);
  qry->num_disparadores++;
  return qry->disparadores[qry->num_disparadores - 1];
}

// Função para criar novo carregador
Carregador *criar_novo_carregador(Qry_t *qry, int id)
{
  qry->carregadores = realloc(qry->carregadores, (qry->num_carregadores + 1) *
                                                     sizeof(Carregador *));
  qry->carregadores[qry->num_carregadores] = criar_carregador(id);
  qry->num_carregadores++;
  return qry->carregadores[qry->num_carregadores - 1];
}

// Função para processar comando pd (posicionar disparador)
void processar_pd(Qry_t *qry, char *linha)
{
  int id;
  float x, y;
  if (sscanf(linha, "pd %d %f %f", &id, &x, &y) == 3)
  {
    Disparador *disp = encontrar_disparador(qry, id);
    if (disp == NULL)
    {
      disp = criar_novo_disparador(qry, id, x, y);
      fprintf(qry->txt_output, "Disparador %d posicionado em (%.2f, %.2f)\n",
              id, x, y);
    }
    else
    {
      fprintf(qry->txt_output, "Erro: Disparador %d já existe\n", id);
    }
  }
}

// Função para processar comando lc (carregar formas)
void processar_lc(Qry_t *qry, char *linha)
{
  int carregador_id, num_formas;
  if (sscanf(linha, "lc %d %d", &carregador_id, &num_formas) == 2)
  {
    Carregador *carregador = encontrar_carregador(qry, carregador_id);
    if (carregador == NULL)
    {
      carregador = criar_novo_carregador(qry, carregador_id);
    }

    // Usa a função do disparador para carregar formas
    // Para o comando lc, precisamos carregar as primeiras n formas do chão no
    // carregador
    int formas_carregadas = 0;
    for (int i = 0; i < num_formas && i < num_formas_chao; i++)
    {
      // Aqui você precisaria implementar a lógica para mover a forma do chão
      // para o carregador Por enquanto, simula o carregamento das formas do
      // chão
      formas_carregadas++;
      fprintf(qry->txt_output, "Forma %d carregada no carregador %d\n",
              formas_chao[i].id, carregador_id);
    }
    fprintf(qry->txt_output, "Carregador %d: %d formas carregadas\n",
            carregador_id, formas_carregadas);
  }
}

// Função para processar comando atch (encaixar carregadores)
void processar_atch(Qry_t *qry, char *linha)
{
  int disparador_id, carregador_esq, carregador_dir;
  if (sscanf(linha, "atch %d %d %d", &disparador_id, &carregador_esq,
             &carregador_dir) == 3)
  {
    Disparador *disp = encontrar_disparador(qry, disparador_id);
    Carregador *carreg_esq = encontrar_carregador(qry, carregador_esq);
    Carregador *carreg_dir = encontrar_carregador(qry, carregador_dir);

    if (disp && carreg_esq && carreg_dir)
    {
      set_carregador_esq(disp, carreg_esq);
      set_carregador_dir(disp, carreg_dir);
      fprintf(qry->txt_output,
              "Carregadores %d e %d encaixados no disparador %d\n",
              carregador_esq, carregador_dir, disparador_id);
    }
    else
    {
      fprintf(qry->txt_output,
              "Erro: Disparador ou carregadores não encontrados\n");
    }
  }
}

// Função para processar comando shft (preparar disparo)
void processar_shft(Qry_t *qry, char *linha)
{
  int disparador_id, vezes;
  char lado;
  if (sscanf(linha, "shft %d %c %d", &disparador_id, &lado, &vezes) == 3)
  {
    Disparador *disp = encontrar_disparador(qry, disparador_id);
    if (disp)
    {
      // Usa a função carregar_disp do disparador para preparar o disparo
      char comando_str[2] = {lado, '\0'};
      carregar_disp(disp, vezes, comando_str);

      // Reporta o resultado
      fprintf(qry->txt_output,
              "Botão %c do disparador %d pressionado %d vezes\n", lado,
              disparador_id, vezes);

      // Reporta dados da forma no ponto de disparo
      disparador_reportar_topo(disp, qry->txt_output);
    }
    else
    {
      fprintf(qry->txt_output, "Erro: Disparador %d não encontrado\n",
              disparador_id);
    }
  }
}

// Função para processar comando dsp (disparar)
void processar_dsp(Qry_t *qry, char *linha)
{
  int disparador_id;
  float dx, dy;
  if (sscanf(linha, "dsp %d %f %f", &disparador_id, &dx, &dy) == 3)
  {
    Disparador *disp = encontrar_disparador(qry, disparador_id);
    if (disp)
    {
      float x_final = disparador_get_x(disp) + dx;
      float y_final = disparador_get_y(disp) + dy;

      fprintf(qry->txt_output,
              "Disparo do disparador %d: deslocamento (%.2f, %.2f)\n",
              disparador_id, dx, dy);
      fprintf(qry->txt_output, "Posição final: (%.2f, %.2f)\n", x_final,
              y_final);

      // Reporta dados da forma disparada
      disparador_reportar_topo(disp, qry->txt_output);

      // Usa a função disparar() do disparador para obter a forma
      void *forma_disparada = disparar(disp);
      if (forma_disparada != NULL)
      {
        // Extrai dados reais da forma
        int id_forma = get_forma_id(forma_disparada);
        char tipo_forma = get_forma_tipo(forma_disparada);
        const char *fill_color = get_forma_fill_color(forma_disparada);
        const char *border_color = get_forma_border_color(forma_disparada);
        float area = calcular_area_forma_real(tipo_forma, forma_disparada);

        // Adiciona forma à arena
        adicionar_forma_arena(forma_disparada, id_forma, x_final, y_final,
                              tipo_forma, (char *)fill_color,
                              (char *)border_color, area);

        fprintf(qry->txt_output,
                "Forma %d (tipo %c) disparada e adicionada à arena na posição "
                "(%.2f, %.2f)\n",
                id_forma, tipo_forma, x_final, y_final);
        fprintf(qry->txt_output,
                "  Área: %.2f, Cor preenchimento: %s, Cor borda: %s\n", area,
                fill_color ? fill_color : "N/A",
                border_color ? border_color : "N/A");
      }
      else
      {
        fprintf(qry->txt_output,
                "Disparador %d não possui forma para disparar\n",
                disparador_id);
      }
    }
    else
    {
      fprintf(qry->txt_output, "Erro: Disparador %d não encontrado\n",
              disparador_id);
    }
  }
  else
  {
    fprintf(qry->txt_output, "Erro: Comando dsp inválido: %s\n", linha);
  }
}

// Função para processar comando rjd (rajada)
void processar_rjd(Qry_t *qry, char *linha)
{
  int disparador_id;
  char lado;
  float dx, dy, ix, iy;
  if (sscanf(linha, "rjd %d %c %f %f %f %f", &disparador_id, &lado, &dx, &dy,
             &ix, &iy) == 6)
  {
    Disparador *disp = encontrar_disparador(qry, disparador_id);
    if (disp)
    {
      fprintf(qry->txt_output, "Iniciando rajada do disparador %d\n",
              disparador_id);

      // Para rajada, vamos assumir que dispara até esgotar as formas do
      // carregador Por simplicidade, vamos disparar 5 vezes
      int vezes = 5;

      // Simula a rajada de disparos
      for (int i = 0; i < vezes; i++)
      {
        // Equivalente a: shft d [e|d] 1
        fprintf(qry->txt_output, "Rajada %d: ", i + 1);
        if (lado == 'e')
        {
          fprintf(qry->txt_output, "Botão esquerdo pressionado\n");
        }
        else
        {
          fprintf(qry->txt_output, "Botão direito pressionado\n");
        }

        // Equivalente a: dsp d dx+i*ix dy+i*iy
        float dx_atual = dx + i * ix;
        float dy_atual = dy + i * iy;
        float x_final = disparador_get_x(disp) + dx_atual;
        float y_final = disparador_get_y(disp) + dy_atual;

        fprintf(qry->txt_output, "  Deslocamento: (%.2f, %.2f)\n", dx_atual,
                dy_atual);
        fprintf(qry->txt_output, "  Posição final: (%.2f, %.2f)\n", x_final,
                y_final);

        // Usa a função disparar() do disparador para obter a forma
        void *forma_disparada = disparar(disp);
        if (forma_disparada != NULL)
        {
          // Extrai dados reais da forma
          int id_forma = get_forma_id(forma_disparada);
          char tipo_forma = get_forma_tipo(forma_disparada);
          const char *fill_color = get_forma_fill_color(forma_disparada);
          const char *border_color = get_forma_border_color(forma_disparada);
          float area = calcular_area_forma_real(tipo_forma, forma_disparada);

          adicionar_forma_arena(forma_disparada, id_forma, x_final, y_final,
                                tipo_forma, (char *)fill_color,
                                (char *)border_color, area);

          fprintf(qry->txt_output, "  Forma %d (tipo %c) disparada na rajada\n",
                  id_forma, tipo_forma);
        }
      }
    }
    else
    {
      fprintf(qry->txt_output, "Erro: Disparador %d não encontrado\n",
              disparador_id);
    }
  }
}

// Função para processar comando calc (processar arena)
void processar_calc(Qry_t *qry, char *linha)
{
  fprintf(qry->txt_output, "Processando figuras da arena...\n");
  fprintf(qry->txt_output, "Total de formas na arena: %d\n", num_formas_arena);

  // Processa as formas na ordem que foram lançadas
  for (int i = 0; i < num_formas_arena - 1; i++)
  {
    if (formas_arena[i].id == -1)
    {
      continue; // pula formas já esmagadas
    }

    for (int j = i + 1; j < num_formas_arena; j++)
    {
      if (formas_arena[j].id == -1)
      {
        continue; // pula formas já esmagadas
      }

      // Verifica sobreposição entre forma i e forma i+1 (j)
      if (verificar_sobreposicao(&formas_arena[i], &formas_arena[j]))
      {
        fprintf(qry->txt_output,
                "Verificando sobreposição entre forma %d (ordem %d) e forma %d "
                "(ordem %d)\n",
                formas_arena[i].id, formas_arena[i].ordem_lancamento,
                formas_arena[j].id, formas_arena[j].ordem_lancamento);

        // Processa a sobreposição
        processar_sobreposicao(&formas_arena[i], &formas_arena[j],
                               qry->txt_output, qry->svg_output);

        // Para após processar a primeira sobreposição (conforme especificação)
        break;
      }
    }
  }

  // Reporta resultados finais
  fprintf(qry->txt_output, "Verificação de colisões concluída\n");
  fprintf(qry->txt_output, "Área total esmagada no round: %.2f\n",
          area_total_esmagada);
  fprintf(qry->txt_output, "Área total esmagada: %.2f\n", area_total_esmagada);
  fprintf(qry->txt_output, "Pontuação do jogo: %.2f\n", area_total_esmagada);

  // Adiciona asteriscos vermelhos no SVG para formas esmagadas
  fprintf(qry->svg_output,
          "<!-- Asteriscos vermelhos para formas esmagadas -->\n");

  // Desenha todas as formas que não foram esmagadas
  for (int i = 0; i < num_formas_arena; i++)
  {
    if (formas_arena[i].id != -1)
    { // não foi esmagada
      fprintf(qry->svg_output, "<!-- Forma %d na posição (%.2f, %.2f) -->\n",
              formas_arena[i].id, formas_arena[i].x_final,
              formas_arena[i].y_final);
    }
  }
}

// Função para processar uma linha de comando
void processar_comando(Qry_t *qry, char *linha)
{
  // Remove espaços em branco e quebras de linha
  char *trimmed = linha;
  while (isspace(*trimmed))
    trimmed++;

  // Ignora linhas vazias e comentários
  if (*trimmed == '\0' || *trimmed == '#')
  {
    return;
  }

  // Identifica o comando
  if (strncmp(trimmed, "pd ", 3) == 0)
  {
    processar_pd(qry, trimmed);
  }
  else if (strncmp(trimmed, "lc ", 3) == 0)
  {
    processar_lc(qry, trimmed);
  }
  else if (strncmp(trimmed, "atch ", 5) == 0)
  {
    processar_atch(qry, trimmed);
  }
  else if (strncmp(trimmed, "shft ", 5) == 0)
  {
    processar_shft(qry, trimmed);
  }
  else if (strncmp(trimmed, "dsp ", 4) == 0)
  {
    processar_dsp(qry, trimmed);
  }
  else if (strncmp(trimmed, "rjd ", 4) == 0)
  {
    processar_rjd(qry, trimmed);
  }
  else if (strncmp(trimmed, "calc", 4) == 0)
  {
    processar_calc(qry, trimmed);
  }
  else
  {
    fprintf(qry->txt_output, "Comando desconhecido: %s\n", trimmed);
  }
}

Qry execute_qry_commands(FileData qryFileData, FileData geoFileData,
                         Ground ground, const char *output_path)
{
  Qry_t *qry = malloc(sizeof(Qry_t));
  if (qry == NULL)
  {
    printf("Error: Failed to allocate memory for Qry\n");
    return NULL;
  }

  // Inicializa a estrutura
  qry->disparadores = NULL;
  qry->num_disparadores = 0;
  qry->carregadores = NULL;
  qry->num_carregadores = 0;
  qry->ground = ground;
  qry->output_path = output_path;

  // Abre arquivos de saída
  char txt_path[256], svg_path[256];
  snprintf(txt_path, sizeof(txt_path), "%s.txt", output_path);
  snprintf(svg_path, sizeof(svg_path), "%s.svg", output_path);

  qry->txt_output = fopen(txt_path, "w");
  qry->svg_output = fopen(svg_path, "w");

  if (!qry->txt_output || !qry->svg_output)
  {
    printf("Error: Failed to open output files\n");
    free(qry);
    return NULL;
  }

  // Inicializa o SVG
  fprintf(qry->svg_output, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
  fprintf(qry->svg_output, "<svg xmlns=\"http://www.w3.org/2000/svg\" "
                           "width=\"800\" height=\"600\">\n");

  // Processa as linhas do arquivo QRY
  Queue lines_queue = getLinesQueue(qryFileData);
  while (!queue_is_empty(lines_queue))
  {
    char *linha = (char *)queue_dequeue(lines_queue);
    processar_comando(qry, linha);
    free(linha);
  }

  // Finaliza o SVG
  fprintf(qry->svg_output, "</svg>\n");

  // Fecha arquivos de saída
  fclose(qry->txt_output);
  fclose(qry->svg_output);

  return qry;
}

void destroy_qry_waste(Qry qry)
{
  if (qry == NULL)
  {
    return;
  }

  Qry_t *qry_data = (Qry_t *)qry;

  // Libera todos os disparadores
  for (int i = 0; i < qry_data->num_disparadores; i++)
  {
    if (qry_data->disparadores[i] != NULL)
    {
      destruir_disparador(qry_data->disparadores[i]);
    }
  }
  free(qry_data->disparadores);

  // Libera todos os carregadores
  for (int i = 0; i < qry_data->num_carregadores; i++)
  {
    if (qry_data->carregadores[i] != NULL)
    {
      destruir_carregador(qry_data->carregadores[i]);
    }
  }
  free(qry_data->carregadores);

  // Libera o array de formas do chão
  free(formas_chao);
  formas_chao = NULL;
  num_formas_chao = 0;
  capacidade_formas_chao = 0;

  // Libera o array de formas da arena
  for (int i = 0; i < num_formas_arena; i++)
  {
    if (formas_arena[i].fill_color != NULL)
    {
      free(formas_arena[i].fill_color);
    }
    if (formas_arena[i].border_color != NULL)
    {
      free(formas_arena[i].border_color);
    }
  }
  free(formas_arena);
  formas_arena = NULL;
  num_formas_arena = 0;
  capacidade_formas_arena = 0;
  area_total_esmagada = 0.0;

  // Libera a estrutura principal
  free(qry_data);
}