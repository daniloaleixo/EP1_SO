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

FILE *abre_arquivo(char *nome_arquivo);
FILE *cria_arquivo(char *nome);
void *malloc_safe(size_t n);

struct timeval tempo_inicial;
void inicializa_relogio();
float tempo_decorrido();