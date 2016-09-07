/* ********************************
    EP1 - Sistemas Operacionais
    Prof. Daniel Batista

    Danilo Aleixo Gomes de Souza
    n USP: 7972370
  
    Carlos Augusto Motta de Lima
    n USP: 7991228

********************************** */

#include "auxiliares.h"

FILE *abre_arquivo(char *nome_arquivo)
{
  FILE *entrada;
  entrada = fopen(nome_arquivo, "r");
  if(entrada == NULL)
  {
    fprintf(stderr, "Nao consegui ler o arquivo!\n");
    exit(-1);
  }

  return entrada;
}

FILE *cria_arquivo(char *nome)
{
  FILE *arq;
  arq = fopen(nome, "wt");

  if(arq == NULL)
  {
    printf("Problemas na CRIACAO do arquivo\n");
    return;
  }
  return arq;
}

/* malloc_safe: testa o ponteiro devolvido por malloc */
void *malloc_safe(size_t n)
{
  void *pt;
  pt = malloc(n);
  if(pt == NULL) {
    printf("ERRO na alocacao de memoria.\n\n");
    exit(-1);
  }
  return pt;
}

float tempo_decorrido()
{
  struct timeval tempo_atual;
  gettimeofday(&tempo_atual, NULL);
  if(tempo_atual.tv_usec < tempo_inicial.tv_usec)
  {
    tempo_atual.tv_usec += 1000000;
    tempo_atual.tv_sec -= 1;
  }
  return tempo_atual.tv_sec - tempo_inicial.tv_sec +
         (tempo_atual.tv_usec - tempo_inicial.tv_usec)/1e6;
}

void inicializa_relogio() {
  gettimeofday(&tempo_inicial, NULL);
}
