#ifndef ARG_H
#define ARG_H

// Função que busca e retorna o valor associado a um argumento específico da linha de comando
// argc: número total de argumentos
// argv: array com os argumentos da linha de comando
// arg: argumento cuja valor se deseja obter
// Retorna: valor do argumento se encontrado, NULL caso contrário
const char *getArgValue(int argc, char *argv[], char *arg);

#endif // ARG_H
