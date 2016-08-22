/* ********************************
    EP1 - Sistemas Operacionais
    Prof. Daniel Batista

    Danilo Aleixo Gomes de Souza
    n USP: 7972370
  
    Carlos Augusto Motta de Lima
    n USP: 7991228

********************************** */

#include "StringOps.h"
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
  int prioridade;
  struct processo *prox;
} Processo;

/* funcoes de threads */
void *thread_function(Processo *arg);

/* funcoes de escalonamento */
void firstComeFirstServed(Processo *lista);
//void shortestJobFirst(Processo *lista);
void shortestRemainTimeNext(Processo *lista);
//void roundRobin(Processo *lista);
//void escalonamentoPrioridade(Processo *lista);

/* funcoes auxliares*/
float calcularTempoDecorrido();
Processo *ordenarMetodo1(Processo *lista);
Processo *ordenarMetodo2(Processo *lista);
Processo *retirarLista(Processo *lista);
void imprimeTodosProcs();
Processo *interpretaEntrada(FILE *entrada);
Processo *copiaLista(Processo *lista);
Processo* SortedMerge(Processo* a, Processo* b, int mode);
void FrontBackSplit(Processo* source,Processo** frontRef, Processo** backRef);
void MergeSort(Processo** headRef, int mode);
int compare(Processo *a, Processo *b, int mode);
/*------------------------------*/

int numeroMetodoEscalonamento = 0, numProcs = 0;
pthread_mutex_t naoPodeAcessarProcessos = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t *processadoresSendoUsados;
pthread_mutex_t semafArqSaida = PTHREAD_MUTEX_INITIALIZER;
int *flagProcessadoresEmUso;
Processo *listaProcessos;
pthread_t *threads;
FILE *saida;
struct timeval tempoInicial;
char d = 0;
int contadorLinhaSaida = 0;
int quantMudancasContexto = 0;


int main(int argc, char *argv[])
{
  int i;
  FILE *trace; 
  char *nomeSaida;
  float tempoDecorrido;
  
  /* inicializa contador de tempo */
  gettimeofday(&tempoInicial, NULL);
  tempoDecorrido = calcularTempoDecorrido();

  if(argc == 4 || argc == 5 )
  {

    if(argc == 5)
    {
      if(argv[4][0] == 'd')
        d = TRUE;
    }

    /* pega os parametros */
    numeroMetodoEscalonamento = atoi(argv[1]);
    trace = leEntrada(argv[2]);
    nomeSaida = argv[3];
    saida = criaArquivo(nomeSaida);

    /* numero de processadores */
    numProcs = get_nprocs();

    /* inicializamos o vetor de processadores e os semaforos */
    flagProcessadoresEmUso = mallocSafe(numProcs * sizeof(int));
    for(i = 0; i < numProcs; i++)
      flagProcessadoresEmUso[i] = LIVRE;
    processadoresSendoUsados = mallocSafe(numProcs * sizeof(pthread_mutex_t));
    threads = mallocSafe(numProcs * sizeof(pthread_t));

    /* pega o processo do arquivo de texto e coloca em uma struct */
    listaProcessos = interpretaEntrada(trace);
    /* DEPURACAO imprimeTodosProcs(listaProcessos);*/

    /* escolhe o metodo de escalonamento */
    switch(numeroMetodoEscalonamento)
    {
      case 1:
        firstComeFirstServed(listaProcessos); break;

      case 2:
        shortestRemainTimeNext(listaProcessos); break;

      case 3:
        //Multilpas filas
    }
  }else
  {
    printf("argumentos incorretos\n");
  }

  /* esperar as threads liberarem */
  if(numProcs != 0)
  {  
    for(i = 0; i < numProcs; i++)
      if(flagProcessadoresEmUso[i] == EM_USO) i = 0;
    
    fprintf(saida, "\n");
    fclose(saida); 

    fprintf(stderr, "Quantidade de mudanças de contexto: %d\n", quantMudancasContexto);
  }

  return 0;
}

/* *****************************************

            funcoes de threads

  ***************************************** */

void *thread_function(Processo *arg)
{
  int i;
  float tempoInicialProcesso, tempoDecorridoProcesso;
  
  tempoInicialProcesso = calcularTempoDecorrido();

  if(d) fprintf(stderr, "O processo %s esta usando a CPU: %d\n", arg->nome, (int) arg->t0);


  while((tempoDecorridoProcesso = calcularTempoDecorrido() - tempoInicialProcesso) < arg->dt)
  {
    i += 1;
    i = i % 100000;
  }

  pthread_mutex_lock(&semafArqSaida);
  /* imprime o nome do processo no arquivo saida*/
  fprintf(saida, "%s", arg->nome);
  /* imprime o tf e o tr no arquivo */
  fprintf(saida, " %f", calcularTempoDecorrido());
  fprintf(saida, " %f\n", tempoDecorridoProcesso);
  contadorLinhaSaida++;
  if(d) fprintf(stderr, "O processo %s terminou e esta na linha %d do arquivo de saida\n", arg->nome, contadorLinhaSaida);
  pthread_mutex_unlock(&semafArqSaida);


  pthread_mutex_lock(&processadoresSendoUsados[(int) arg->t0]);
  flagProcessadoresEmUso[(int) arg->t0] = LIVRE;
  pthread_mutex_unlock(&processadoresSendoUsados[(int) arg->t0]);

  if(d) fprintf(stderr, "O processo %s esta deixando a CPU: %d\n", arg->nome, (int) arg->t0);

  tempoDecorridoProcesso = calcularTempoDecorrido();
  return NULL;


}

/* *****************************************

                funcoes de escalonamento

  ***************************************** */

void firstComeFirstServed(Processo *listaProcessos)
{
  Processo *temp, *copia;
  int i;
  int threadID = 0;
  int contadorLinhaTrace = 0;

  listaProcessos = ordenarMetodo1(listaProcessos);

  /*
      COMEÇAR AS THREADS 
  */

  /* retirar elemento da lista */
  pthread_mutex_lock(&naoPodeAcessarProcessos);
  temp = listaProcessos->prox;
  copia = retirarLista(listaProcessos);
  listaProcessos = temp;
  pthread_mutex_unlock(&naoPodeAcessarProcessos);
  contadorLinhaTrace++;
  if(d) fprintf(stderr, "Chegada do processo: %s - na linha %d do trace\n", copia->nome, contadorLinhaTrace);

  while(copia != NULL)
  {
    /* Enquanto o t0 do processo nao entra o sistema espera por isso */
    /* DEPURACAO printf("temp - t0: %d - copia: %s - %d\n", tempoSistema - tempoInicialSistema, copia->nome, copia->t0);*/
    while(calcularTempoDecorrido() < copia->t0)
    {
      usleep(100);
    }

    /* procura o proximo processador disponivel */
    for(threadID = 0; flagProcessadoresEmUso[threadID] != LIVRE; threadID = (threadID + 1) % numProcs);
       

    /* Criacao da thread */
    /* sinalizamos que o processador esta sendo usado */
    pthread_mutex_lock(&processadoresSendoUsados[threadID]);
    flagProcessadoresEmUso[threadID] =  EM_USO;
    pthread_mutex_unlock(&processadoresSendoUsados[threadID]);

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
    pthread_mutex_lock(&naoPodeAcessarProcessos);
    /* retirar elemento da lista */
    if(listaProcessos != NULL)
    {
      temp = listaProcessos->prox;
      copia = retirarLista(listaProcessos);
      listaProcessos = temp;

      contadorLinhaTrace++;
      if(d) fprintf(stderr, "Chegada do processo: %s - na linha %d do trace\n", copia->nome, contadorLinhaTrace);
    }
    else copia = NULL;
    pthread_mutex_unlock(&naoPodeAcessarProcessos);
  }
}


void shortestRemainTimeNext(Processo *lista)
{
  lista = ordenarMetodo1(lista);
  MergeSort(&lista, 3);
  /*return lista;*/
}

/* *****************************************

                funcoes auxiliares 

  ***************************************** */
float calcularTempoDecorrido()
{
  struct timeval tempoAtual;
  gettimeofday(&tempoAtual, NULL);
  if(tempoAtual.tv_usec < tempoInicial.tv_usec)
  {
    tempoAtual.tv_usec += 1000000;
    tempoAtual.tv_sec -= 1;
  }
  return (float)((tempoAtual.tv_sec - tempoInicial.tv_sec) + ((float)(tempoAtual.tv_usec - tempoInicial.tv_usec)/1e6));;
}

Processo *ordenarMetodo1(Processo *lista)
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

Processo *ordenarMetodo2(Processo *lista)
{
  MergeSort(&lista, 2);
  return lista;
}

/* retirar da lista */
Processo *retirarLista(Processo *lista)
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
void imprimeTodosProcs(Processo *process)
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
Processo *interpretaEntrada(FILE *entrada)
{
  char *linha;
  char **palavras;
  int tamanholinha;
  int i = 0;
  Processo *ant, *x;

  linha = readLine(entrada);
  tamanholinha = strlen(linha);
  palavras = mallocSafe(5 * sizeof(char*));
  for(i = 0; i < 5; i++)
  {
    palavras[i] = mallocSafe(32 * sizeof(char));
  }
  x = mallocSafe(sizeof(Processo));

  /* split */
  palavras = split(linha, tamanholinha, ' ', 5);

  /* depuracao for(i = 0; i < 5; i++)
  {
    printf("%s\n", palavras[i]);
  }*/

	x->t0 = atof(palavras[0]);
	x->nome = palavras[1];
	x->dt = atof(palavras[2]);
	x->deadline = atof(palavras[3]);
	x->prioridade = atoi(palavras[4]);
  x->prox = NULL;

  ant = x;
  while((linha = readLine(entrada)) != NULL)
  {
    if(strlen(linha) > 5)
    {
      tamanholinha = strlen(linha);
      palavras = mallocSafe(5 * sizeof(char*));
      for(i = 0; i < 5; i++)
      {
        palavras[i] = mallocSafe(32 * sizeof(char));
      }
      x = mallocSafe(sizeof(Processo));

      /* split */
      palavras = split(linha, tamanholinha, ' ', 5);

      /* depuracao * for(i = 0; i < 5; i++)
      {
        printf("%s\n", palavras[i]);
      }*/

      x->t0 = atoi(palavras[0]);
      x->nome = palavras[1];
      x->dt = atoi(palavras[2]);
      x->deadline = atoi(palavras[3]);
      x->prioridade = atoi(palavras[4]);
      x->prox = ant;
      ant = x;
    }
    
  }
  /* depuracao printf("%d\n", x->t0);
  printf("%d\n", x->prioridade);
  printf("%d\n", x->deadline);*/

  return ant;
}

Processo *copiaLista(Processo *lista)
{
  Processo *aux, *x, *copia, *ant;

  x = mallocSafe(sizeof(Processo));
  x->t0 = lista->t0;
  x->nome = lista->nome;
  x->dt = lista->dt;
  x->deadline = lista->deadline;
  x->prioridade = lista->prioridade;
  x->prox = lista->prox;
  copia = x;
  ant = x;

  for(aux = lista->prox; aux != NULL; aux = aux->prox)
  {

    x = mallocSafe(sizeof(Processo));
    ant->prox = x;
    x->t0 = aux->t0;
    x->nome = aux->nome;
    x->dt = aux->dt;
    x->deadline = aux->deadline;
    x->prioridade = aux->prioridade;
    ant = x;

  }

  return copia;
}



/*          MERGE SORT

FONTE: http://www.geeksforgeeks.org/merge-sort-for-linked-list/
*/
 
/* sorts the linked list by changing next pointers (not data) */
void MergeSort(Processo** headRef, int mode)
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
  FrontBackSplit(head, &a, &b);
 
  /* Recursively sort the sublists */
  MergeSort(&a, mode);
  MergeSort(&b, mode);
 
  /* answer = merge the two sorted lists together */
  *headRef = SortedMerge(a, b, mode);
}
 
/* See http://geeksforgeeks.org/?p=3622 for details of this
   function */
Processo* SortedMerge(Processo* a, Processo* b, int mode)
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
     result->prox = SortedMerge(a->prox, b, mode);
  }
  else
  {
     result = b;
     result->prox = SortedMerge(a, b->prox, mode);
  }
  return(result);
}
 
/* UTILITY FUNCTIONS */
/* Split the nodes of the given list into front and back halves,
     and return the two lists using the reference parameters.
     If the length is odd, the extra node should go in the front list.
     Uses the fast/slow pointer strategy.  */
void FrontBackSplit(Processo* source,
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
  {
    if (a->dt <= b->dt)
      return 1;
    else
      return 0;
  }
  else if(mode == 3)
  {
    if (a->deadline <= b->deadline)
      return 1;
    else
      return 0;
  }
  else if(mode == 5)
  {
    if (a->prioridade >= b->prioridade)
      return 1;
    else
      return 0;
  }

}