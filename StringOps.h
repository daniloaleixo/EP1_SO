#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE *leEntrada(char *nomeArquivo);
FILE *criaArquivo(char *nome);
void *mallocSafe(size_t n);
char *concatenaStrings(char *original, char *add);
char *readLine(FILE *entrada);
char **split(char *linha, int tamanholinha, char separador, int numeroDeEspacos);