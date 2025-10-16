#include "arg.h"
#include <string.h>

// Função para obter o valor de um argumento específico da linha de comando
// Retorna NULL se o argumento não for encontrado ou não tiver valor
const char *getArgValue(int argc, char *argv[], char *arg)
{
    // Percorre todos os argumentos a partir do índice 1 (índice 0 é o nome do programa)
    for (int i = 1; i < argc; ++i)
    {
        // Se encontrar o argumento desejado
        if (strcmp(argv[i], arg) == 0)
        {
            // Verifica se existe um valor após o argumento
            if (argv[i + 1] == NULL)
            {
                return NULL; // Não há valor para o argumento
            }
            return argv[i + 1]; // Retorna o valor do argumento
        }
    }
    return NULL; // Argumento não encontrado
}
