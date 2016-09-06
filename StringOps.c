#include "StringOps.h"


char **split(char *linha, int tamanhoLinha, char separador, int numeroDeEspacos)
{
  int inicio = 0, fim = 0;
  int i = 0, j = 0;
  char **palavras = mallocSafe(numeroDeEspacos * sizeof(char*));
  for(i = 0; i < numeroDeEspacos; i++)
  {
    palavras[i] = mallocSafe(32 * sizeof(char));
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


FILE *leEntrada (char *nomeArquivo)
{
    FILE *entrada;
    entrada = fopen(nomeArquivo, "r");
    if(entrada == NULL)
    {
      fprintf(stderr, "Nao consegui ler o arquivo!\n");
      exit(-1);
    }

    return entrada;
}

FILE *criaArquivo(char *nome)
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
  mallocSafe: testa o ponteiro devolvido por malloc
 */
void *mallocSafe(size_t n)
{
  void *pt;
  pt = malloc(n);
  if(pt == NULL) {
    printf("ERRO na alocacao de memoria.\n\n");
    exit(-1);
  }
  return pt;
}

char *readLine(FILE *entrada)
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
