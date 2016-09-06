/* ********************************
    EP1 - Sistemas Operacionais
    Prof. Daniel Batista

    Danilo Aleixo Gomes de Souza
    n USP: 7972370
  
    Carlos Augusto Motta de Lima
    n USP: 7991228

********************************** */

#include "auxiliares.h"

char **split(char *linha, int tamanhoLinha, char separador, int numeroDeEspacos)
{
  int inicio = 0, fim = 0;
  int i = 0, j = 0;
  char **palavras = malloc_safe(numeroDeEspacos * sizeof(char*));
  for(i = 0; i < numeroDeEspacos; i++)
  {
    palavras[i] = malloc_safe(32 * sizeof(char));
  }


  for(i = 0; i < tamanhoLinha; i++)
  {
    if(linha[i] == separador)
    {
      fim = i;
      /* depuracao printf("%d %d \n", inicio, fim);*/
      memcpy( palavras[j++], &linha[inicio], fim - inicio);
      palavras[j-1][fim - inicio] = '\0';
      /* depuracao printf("palavra %s\n\n", palavras[j - 1]);*/
      inicio = fim + 1;
    }   
  }
  fim = tamanhoLinha;
  memcpy(palavras[j++], &linha[inicio], fim - inicio);
  palavras[j-1][fim - inicio] = '\0';

  return palavras;
}


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

/*
  malloc_safe: testa o ponteiro devolvido por malloc
 */
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

char *read_line(FILE *entrada)
{
    char *line, *nLine;
    int n, ch, size;

    n = 0;
    size = 128;
    line = malloc(size * sizeof(char) + 1);
    while((ch = getc(entrada)) != '\n' && ch != EOF)
      line[n++] = ch;
    if(n == 0 && ch == EOF)
    {
      free(line);
      return NULL;
    }
    line[n] = '\0';
    nLine = (char *) malloc(n * sizeof(char) + 1);
    strcpy(nLine, line);
    free(line);
    return nLine;
}
