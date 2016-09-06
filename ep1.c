/* ********************************
    EP1 - Sistemas Operacionais
    Prof. Daniel Batista

    Danilo Aleixo Gomes de Souza
    n USP: 7972370
  
    Carlos Augusto Motta de Lima
    n USP: 7991228

********************************** */

#include "auxiliares.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/sysinfo.h>
#include <time.h>

#define EM_USO 1
#define LIVRE 0

#define TRUE 1
#define FALSE 0

typedef struct processo {
  float t0;
  char *nome;
  float dt; /* quanto tempo real da CPU deve ser simulado */
  float deadline;
  struct processo *prox;
} Processo;

/* funcoes de threads */
void *thread_function(Processo *arg);

/* funcoes de escalonamento */
void first_come_first_served(Processo *lista);
void shortest_remaining_time_next(Processo *lista);
void escalonamento_multiplas_filas(Processo *lista);

/* funcoes auxliares*/
float calcular_tempo_decorrido();
Processo *ordenar_metodo1(Processo *lista);
Processo *ordenar_metodo2(Processo *lista);
Processo *retirar_lista(Processo *lista);
void imprime_todos_procs();
Processo *interpreta_entrada(FILE *entrada);
Processo *copia_lista(Processo *lista);
Processo* sorted_merge(Processo* a, Processo* b, int mode);
void front_back_split(Processo* source, Processo** frontRef, Processo** backRef);
void merge_sort(Processo** headRef, int mode);
int compare(Processo *a, Processo *b, int mode);
/*------------------------------*/

Processo *lista_processos;
pthread_mutex_t nao_pode_acessar_processos = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t *processadores_sendo_usados;
pthread_mutex_t semaf_arq_saida = PTHREAD_MUTEX_INITIALIZER;
pthread_t *threads;
FILE *saida;
struct timeval tempo_inicial;
int *flag_processadores_em_uso;
int numero_metodo_escalonamento = 0, num_procs = 0, d = 0,
    contador_linha_saida = 0, qtde_mudancas_contexto = 0;


int main(int argc, char *argv[])
{
  int i;
  FILE *trace; 
  char *nome_saida;
  float tempo_decorrido;
  
  /* inicializa contador de tempo */
  gettimeofday(&tempo_inicial, NULL);
  tempo_decorrido = calcular_tempo_decorrido();

  if(argc >= 4)
  {
    d = (strcmp(argv[4], "d") == 0);

    /* pega os parametros */
    numero_metodo_escalonamento = atoi(argv[1]);
    trace = le_entrada(argv[2]);
    nome_saida = argv[3];
    saida = cria_arquivo(nome_saida);

    /* numero de processadores */
    num_procs = get_nprocs();

    /* inicializamos o vetor de processadores e os semaforos */
    flag_processadores_em_uso = malloc_safe(num_procs * sizeof(int));
    for(i = 0; i < num_procs; i++)
      flag_processadores_em_uso[i] = LIVRE;
    processadores_sendo_usados = malloc_safe(num_procs * sizeof(pthread_mutex_t));
    threads = malloc_safe(num_procs * sizeof(pthread_t));

    /* pega o processo do arquivo de texto e coloca em uma struct */
    lista_processos = interpreta_entrada(trace);
    /* DEPURACAO imprime_todos_procs(lista_processos);*/

    /* escolhe o metodo de escalonamento */
    switch(numero_metodo_escalonamento)
    {
      case 1:
        first_come_first_served(lista_processos); break;

      case 3:
        shortest_remaining_time_next(lista_processos); break;

      case 5:
        escalonamento_multiplas_filas(lista_processos); break;
    }
  }
  else
    printf("argumentos incorretos\n");

  /* esperar as threads liberarem */
  if(num_procs != 0)
  {  
    for(i = 0; i < num_procs; i++)
      if(flag_processadores_em_uso[i] == EM_USO) i = 0;
    
    fprintf(saida, "\n");
    fclose(saida); 

    fprintf(stderr, "Quantidade de mudanças de contexto: %d\n", qtde_mudancas_contexto);
  }

  return 0;
}

/* *****************************************

            funcoes de threads

  ***************************************** */

void *thread_function(Processo *arg)
{
  int i;
  float tempo_inicial_processo, tempo_decorridoProcesso;
  
  tempo_inicial_processo = calcular_tempo_decorrido();

  if(d) fprintf(stderr, "O processo %s esta usando a CPU: %d\n", arg->nome, (int) arg->t0);


  while((tempo_decorridoProcesso = calcular_tempo_decorrido() - tempo_inicial_processo) < arg->dt)
  {
    i += 1;
    i = i % 100000;
  }

  pthread_mutex_lock(&semaf_arq_saida);
  /* imprime o nome do processo no arquivo saida*/
  fprintf(saida, "%s", arg->nome);
  /* imprime o tf e o tr no arquivo */
  fprintf(saida, " %f", calcular_tempo_decorrido());
  fprintf(saida, " %f\n", tempo_decorridoProcesso);
  contador_linha_saida++;
  if(d) fprintf(stderr, "O processo %s terminou e esta na linha %d do arquivo de saida\n", arg->nome, contador_linha_saida);
  pthread_mutex_unlock(&semaf_arq_saida);


  pthread_mutex_lock(&processadores_sendo_usados[(int) arg->t0]);
  flag_processadores_em_uso[(int) arg->t0] = LIVRE;
  pthread_mutex_unlock(&processadores_sendo_usados[(int) arg->t0]);

  if(d) fprintf(stderr, "O processo %s esta deixando a CPU: %d\n", arg->nome, (int) arg->t0);

  tempo_decorridoProcesso = calcular_tempo_decorrido();
  return NULL;


}

/* *****************************************

                funcoes de escalonamento

  ***************************************** */

void first_come_first_served(Processo *lista_processos)
{
  Processo *temp, *copia;
  int i;
  int threadID = 0;
  int contadorLinhaTrace = 0;

  lista_processos = ordenar_metodo1(lista_processos);

  /*
      COMEÇAR AS THREADS 
  */

  /* retirar elemento da lista */
  pthread_mutex_lock(&nao_pode_acessar_processos);
  temp = lista_processos->prox;
  copia = retirar_lista(lista_processos);
  lista_processos = temp;
  pthread_mutex_unlock(&nao_pode_acessar_processos);
  contadorLinhaTrace++;
  if(d) fprintf(stderr, "Chegada do processo: %s - na linha %d do trace\n", copia->nome, contadorLinhaTrace);

  while(copia != NULL)
  {
    /* Enquanto o t0 do processo nao entra o sistema espera por isso */
    /* DEPURACAO printf("temp - t0: %d - copia: %s - %d\n", tempoSistema - tempo_inicialSistema, copia->nome, copia->t0);*/
    while(calcular_tempo_decorrido() < copia->t0)
    {
      usleep(100);
    }

    /* procura o proximo processador disponivel */
    for(threadID = 0; flag_processadores_em_uso[threadID] != LIVRE; threadID = (threadID + 1) % num_procs);
       

    /* Criacao da thread */
    /* sinalizamos que o processador esta sendo usado */
    pthread_mutex_lock(&processadores_sendo_usados[threadID]);
    flag_processadores_em_uso[threadID] =  EM_USO;
    pthread_mutex_unlock(&processadores_sendo_usados[threadID]);

    /* usaremos a variavel t0 agora para guardar a ID da thread - e nao mais o tempo inicial */
    copia->t0 = threadID;
    /* criamos a thread de fato e comecamos a executar na funcao thread_function */
    if(pthread_create(&threads[threadID], NULL, thread_function, copia))
    {
      printf("error creating thread.");
      abort();
    }

    /* 
        volta a tirar um elemento da lista pra manter a condição de loop 
    */
    pthread_mutex_lock(&nao_pode_acessar_processos);
    /* retirar elemento da lista */
    if(lista_processos != NULL)
    {
      temp = lista_processos->prox;
      copia = retirar_lista(lista_processos);
      lista_processos = temp;

      contadorLinhaTrace++;
      if(d) fprintf(stderr, "Chegada do processo: %s - na linha %d do trace\n", copia->nome, contadorLinhaTrace);
    }
    else copia = NULL;
    pthread_mutex_unlock(&nao_pode_acessar_processos);
  }
}

void shortest_remaining_time_next(Processo *lista)
{
  lista = ordenar_metodo1(lista);
  merge_sort(&lista, 3);
  /*return lista;*/
}

void escalonamento_multiplas_filas(Processo *lista)
{
  lista = ordenar_metodo1(lista);
  merge_sort(&lista, 5);
  /*return lista;*/
}

/* *****************************************

                funcoes auxiliares 

  ***************************************** */
float calcular_tempo_decorrido()
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

Processo *ordenar_metodo1(Processo *lista)
{
  Processo *t, *y, *r;
  y = lista;
  r = NULL;

  /* colocar a lista pela ordem do arquivo */
  while (y != NULL)
  {
    t = y->prox; 
    y->prox = r; 
    r = y; 
    y = t; 
  }
  return r;   
}

Processo *ordenar_metodo2(Processo *lista)
{
  merge_sort(&lista, 2);
  return lista;
}

/* retirar da lista */
Processo *retirar_lista(Processo *lista)
{
  Processo *result = lista;
  if(lista != NULL)
    lista = lista->prox;
  else
    return NULL;
  result->prox = NULL;
  return result;
}

/* imprime todos os processos da lista encadeada de processos */
void imprime_todos_procs(Processo *process)
{
  Processo *aux, *inicial;
  inicial = process;
  int i = 0;

  for(aux = process; (aux != NULL); aux = aux->prox)
  {
    if((i++ != 0) && (aux == inicial) ) break;
    printf("%s\n",aux->nome);
  }
}

/* a funcao pega a linha com o processo e suas infos do arquivo de texto
   e retorna o struct processo com todas as infos */
Processo *interpreta_entrada(FILE *entrada)
{
  char *linha;
  char **palavras;
  int tamanho_linha;
  int i = 0;
  Processo *ant, *x;

  linha = read_line(entrada);
  tamanho_linha = strlen(linha);
  palavras = malloc_safe(5 * sizeof(char*));
  for(i = 0; i < 5; i++)
    palavras[i] = malloc_safe(32 * sizeof(char));
  x = malloc_safe(sizeof(Processo));

  /* split */
  palavras = split(linha, tamanho_linha, ' ', 5);

	x->t0 = atof(palavras[0]);
	x->nome = palavras[1];
	x->dt = atof(palavras[2]);
	x->deadline = atof(palavras[3]);
  x->prox = NULL;

  ant = x;
  while((linha = read_line(entrada)) != NULL)
  {
    if(strlen(linha) > 5)
    {
      tamanho_linha = strlen(linha);
      palavras = malloc_safe(5 * sizeof(char*));
      for(i = 0; i < 5; i++)
        palavras[i] = malloc_safe(32 * sizeof(char));
      x = malloc_safe(sizeof(Processo));

      /* split */
      palavras = split(linha, tamanho_linha, ' ', 5);

      /* depuracao * for(i = 0; i < 5; i++)
      {
        printf("%s\n", palavras[i]);
      }*/

      x->t0 = atoi(palavras[0]);
      x->nome = palavras[1];
      x->dt = atoi(palavras[2]);
      x->deadline = atoi(palavras[3]);
      x->prox = ant;
      ant = x;
    }
    
  }
  /* depuracao printf("%d\n", x->t0);
  printf("%d\n", x->deadline);*/

  return ant;
}

Processo *copia_lista(Processo *lista)
{
  Processo *aux, *x, *copia, *ant;

  x = malloc_safe(sizeof(Processo));
  x->t0 = lista->t0;
  x->nome = lista->nome;
  x->dt = lista->dt;
  x->deadline = lista->deadline;
  x->prox = lista->prox;
  copia = x;
  ant = x;

  for(aux = lista->prox; aux != NULL; aux = aux->prox)
  {

    x = malloc_safe(sizeof(Processo));
    ant->prox = x;
    x->t0 = aux->t0;
    x->nome = aux->nome;
    x->dt = aux->dt;
    x->deadline = aux->deadline;
    ant = x;

  }

  return copia;
}

/*          MERGE SORT

FONTE: http://www.geeksforgeeks.org/merge-sort-for-linked-list/
*/
 
/* sorts the linked list by changing next pointers (not data) */
void merge_sort(Processo** headRef, int mode)
{
  Processo* head = *headRef;
  Processo* a;
  Processo* b;
 

  /*printf("head: %p\n", head);*/
  /* Base case -- length 0 or 1 */
  if ((head == NULL) || (head->prox == NULL))
  {
    return;
  }
  /*printf("passei\n");*/
 
  /* Split head into 'a' and 'b' sublists */
  front_back_split(head, &a, &b);
 
  /* Recursively sort the sublists */
  merge_sort(&a, mode);
  merge_sort(&b, mode);
 
  /* answer = merge the two sorted lists together */
  *headRef = sorted_merge(a, b, mode);
}
 
/* See http://geeksforgeeks.org/?p=3622 for details of this
   function */
Processo* sorted_merge(Processo* a, Processo* b, int mode)
{
  Processo* result = NULL;
 
  /* Base cases */
  if (a == NULL)
     return(b);
  else if (b==NULL)
     return(a);
 
  /* Pick either a or b, and recur */
  if (compare(a, b, mode))
  {
     result = a;
     result->prox = sorted_merge(a->prox, b, mode);
  }
  else
  {
     result = b;
     result->prox = sorted_merge(a, b->prox, mode);
  }
  return(result);
}
 
/* UTILITY FUNCTIONS */
/* Split the nodes of the given list into front and back halves,
     and return the two lists using the reference parameters.
     If the length is odd, the extra node should go in the front list.
     Uses the fast/slow pointer strategy.  */
void front_back_split(Processo* source,
          Processo** frontRef, Processo** backRef)
{
  Processo* fast;
  Processo* slow;
  if (source==NULL || source->prox==NULL)
  {
    /* length < 2 cases */
    *frontRef = source;
    *backRef = NULL;
  }
  else
  {
    slow = source;
    fast = source->prox;
 
    /* Advance 'fast' two nodes, and advance 'slow' one node */
    while (fast != NULL)
    {
      fast = fast->prox;
      if (fast != NULL)
      {
        slow = slow->prox;
        fast = fast->prox;
      }
    }
 
    /* 'slow' is before the midpoint in the list, so split it in two
      at that point. */
    *frontRef = source;
    *backRef = slow->prox;
    slow->prox = NULL;
  }
}

int compare(Processo *a, Processo *b, int mode)
{
  if(mode == 2)
    return (a->dt <= b->dt ? 1 : 0);
  else if(mode == 3)
    return (a->deadline <= b->deadline ? 1 : 0);
}
