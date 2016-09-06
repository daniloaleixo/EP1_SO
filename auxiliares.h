/* ********************************
    EP1 - Sistemas Operacionais
    Prof. Daniel Batista

    Danilo Aleixo Gomes de Souza
    n USP: 7972370
  
    Carlos Augusto Motta de Lima
    n USP: 7991228

********************************** */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE *le_entrada(char *nomeArquivo);
FILE *cria_arquivo(char *nome);
void *malloc_safe(size_t n);
char *read_line(FILE *entrada);
char **split(char *linha, int tamanholinha, char separador, int numeroDeEspacos);