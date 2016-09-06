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
  double t0;
  char *nome;
  double dt; /* quanto tempo real da CPU deve ser simulado */
  double deadline;
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
Processo *ordenar_metodo2(Processo *lista);
Processo *retirar_lista(Processo *lista);
void imprime_todos_procs();
Processo *interpreta_entrada(char *nome_arquivo);
Processo *copia_lista(Processo *lista);
Processo* sorted_merge(Processo* a, Processo* b, int mode);
void front_back_split(Processo* source, Processo** frontRef, Processo** backRef);
void merge_sort(Processo** headRef, int mode);
int compare(Processo *a, Processo *b, int mode);
/*------------------------------*/

Processo *lista_processos;
pthread_mutex_t semaforo_lista_processos = PTHREAD_MUTEX_INITIALIZER,
                semafoto_arq_saida = PTHREAD_MUTEX_INITIALIZER,
                *semaforo_processador;
pthread_t *threads;
FILE *arquivo_saida;
struct timeval tempo_inicial;
int num_procs = 0, d = 0, contador_linha_saida = 0,
    qtde_mudancas_contexto = 0, *processador_em_uso;


int main(int argc, char *argv[])
{
  int i;
  char *nome_saida;
  float tempo_decorrido;
  
  /* inicializa contador de tempo */
  gettimeofday(&tempo_inicial, NULL);
  tempo_decorrido = calcular_tempo_decorrido();

  if(argc >= 4)
  {
    if(argc ==5) d = (strcmp(argv[4], "d") == 0);

    /* le o arquivo de trace da entrada e coloca todos os processos numa
       lista encadeada (lista_processos) */
    lista_processos = interpreta_entrada(argv[2]);

    // nome_saida = argv[3];
    // arquivo_saida = cria_arquivo(nome_saida);

    // /* pega o numero de processadores e aloca um vetor de inteiros para
    //    guardar o estado atual (EM_USO ou LIVRE) de cada processador,
    //    alem de alocar um vetor de semaforos (um por processador) e um vetor de
    //    threads (uma por processador) */
    // num_procs = get_nprocs();
    // processador_em_uso = malloc_safe(num_procs * sizeof(int));
    // for(i = 0; i < num_procs; i++)
    //   processador_em_uso[i] = LIVRE;
    // semaforo_processador = malloc_safe(num_procs * sizeof(pthread_mutex_t));
    // threads = malloc_safe(num_procs * sizeof(pthread_t));

    
    // /* DEPURACAO imprime_todos_procs(lista_processos);*/

    // /* escolhe o metodo de escalonamento */
    // switch(atoi(argv[1]))
    // {
    //   case 1:
    //     first_come_first_served(lista_processos);
    //     break;

    //   case 2:
    //     shortest_remaining_time_next(lista_processos);
    //     break;

    //   case 3:
    //     escalonamento_multiplas_filas(lista_processos);
    //     break;
    // }
  }
  else {
    printf("\nArgumentos incorretos\n");
    printf("Modo de utilizacao: \n\n");
    printf("ep1 <numero do Metodo de Escalonamento> <arquivo de entrada> "
           "<arquivo de saida>\n\n");
  }

  /* espera ate que todos os processadores estejam LIVREs (ou seja, espera
     que todas as threads terminem a execucao) */
  // if(num_procs != 0)
  // {  
  //   for(i = 0; i < num_procs; i++)
  //     if(processador_em_uso[i] == EM_USO) i = 0;
    
  //   fprintf(arquivo_saida, "\n");
  //   fclose(arquivo_saida);

  //   fprintf(stderr, "Quantidade de mudanças de contexto: %d\n",
  //           qtde_mudancas_contexto);
  // }

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

  if(d)
    fprintf(stderr, "O processo %s esta usando a CPU: %d\n",
            arg->nome, (int) arg->t0);

  while((tempo_decorridoProcesso = calcular_tempo_decorrido() - tempo_inicial_processo) < arg->dt)
  {
    i += 1;
    i = i % 100000;
  }

  pthread_mutex_lock(&semafoto_arq_saida);
  /* imprime o nome do processo no arquivo saida*/
  fprintf(arquivo_saida, "%s", arg->nome);
  /* imprime o tf e o tr no arquivo */
  fprintf(arquivo_saida, " %f", calcular_tempo_decorrido());
  fprintf(arquivo_saida, " %f\n", tempo_decorridoProcesso);
  contador_linha_saida++;
  if(d) fprintf(stderr, "O processo %s terminou e esta na linha %d do arquivo de saida\n", arg->nome, contador_linha_saida);
  pthread_mutex_unlock(&semafoto_arq_saida);


  pthread_mutex_lock(&semaforo_processador[(int) arg->t0]);
  processador_em_uso[(int) arg->t0] = LIVRE;
  pthread_mutex_unlock(&semaforo_processador[(int) arg->t0]);

  if(d) fprintf(stderr, "O processo %s esta deixando a CPU: %d\n", arg->nome, (int) arg->t0);

  tempo_decorridoProcesso = calcular_tempo_decorrido();
  return NULL;


}

/* *****************************************

                funcoes de escalonamento

  ***************************************** */

// void first_come_first_served(Processo *lista_processos)
// {
//   Processo *processo_atual, *copiaDaLista;
//   int i, threadID = 0, contadorLinhaTrace = 0;

//   listaProcessos = ordenarMetodo1(listaProcessos);
//   /* DEPURACAO * printf("DEPOIS DA ORDENACAO\n"); imprimeTodosProcs(listaProcessos);*/

//   /*
//       COMEÇAR AS THREADS 
//   */

//   /* retirar elemento da lista */
//   pthread_mutex_lock(&naoPodeAcessarProcessos);
//   processo_atual = retiraPrimeiroElementoDaLista(listaProcessos);

//   /* atualizar a lista para o proximo elemento */
//   copiaDaLista = listaProcessos->prox;
//   /* DEPURACAO */ printf("LISTA DEVERIA ESTAR SEM O ELEMENTO 0\n"); imprimeTodosProcs(copiaDaLista);
//   /* DEPURACAO */ printf("AQUI DEVERIA ESTAR O ELEMRNTO Q FOI TIRADO: nome %s\n", processo_atual->nome);
//   /*listaProcessos = copiaDaLista;*/
//   pthread_mutex_unlock(&naoPodeAcessarProcessos);

  
//   contadorLinhaTrace++;
//   if(d) fprintf(stderr, "Chegada do processo: %s - na linha %d do trace\n", processo_atual->nome, contadorLinhaTrace);

//   /* DEPIRACAO */ printf("VAos entrar no lopp FCFS\n");
//   while(processo_atual != NULL)
//   {
//     /* Enquanto o t0 do processo nao entra o sistema espera por isso */
//     /* DEPURACAO* printf("processo_atual - t0: %d - copiaDaLista: %s - %d\n", processo_atualoSistema - tempoInicialSistema, copiaDaLista->nome, copiaDaLista->t0);*/
//     while(calcularTempoDecorrido() < processo_atual->t0)
//     {
//       /* DEPURACAO printf("Estou esperando processo entrar\n"); */
//       usleep(100);
//     }

//     /* procura o proximo processador disponivel */
//     for(threadID = 0; flagProcessadoresEmUso[threadID] != LIVRE; threadID = (threadID + 1) % numProcs);
       

//     /* Criacao da thread */
//     /* sinalizamos que o processador esta sendo usado */
//     pthread_mutex_lock(&processadoresSendoUsados[threadID]);
//     flagProcessadoresEmUso[threadID] =  EM_USO;
//     /* DEPRICAO */ printf("Processador %d esta em uso, rodando agora o processo: %s\n", threadID, processo_atual->nome);
//     pthread_mutex_unlock(&processadoresSendoUsados[threadID]);

//     /* usaremos a variavel t0 agora para guardar a ID da thread - e nao mais o tempo inicial */
//     processo_atual->t0 = threadID;
//     /* criamos a thread de fato e comecamos a executar na funcao thread_function */
//     if(pthread_create(&threads[threadID], NULL, thread_function, processo_atual))
//     {
//       printf("error creating thread.");
//       abort();
//     }

//     /* 
//         volta a tirar um elemento da lista pra manter a condição de loop 
//     */
//     pthread_mutex_lock(&naoPodeAcessarProcessos);
//     /* retirar elemento da lista*/
//     if(copiaDaLista != NULL)
//     {
//       processo_atual = retiraPrimeiroElementoDaLista(copiaDaLista);
//       copiaDaLista = copiaDaLista->prox;

//       contadorLinhaTrace++;
//       if(d) fprintf(stderr, "Chegada do processo: %s - na linha %d do trace\n", processo_atual->nome, contadorLinhaTrace);
//     }
//     else processo_atual = NULL;
//     pthread_mutex_unlock(&naoPodeAcessarProcessos);
//   }


//   // ak

//   /*
//       COMEÇAR AS THREADS 
//   */

//   /* retirar elemento da lista */
//   pthread_mutex_lock(&semaforo_lista_processos);
//   temp = lista_processos->prox;
//   copia = retirar_lista(lista_processos);
//   lista_processos = temp;
//   pthread_mutex_unlock(&semaforo_lista_processos);
  
//   if(d)
//   {
//     linha_arquivo_trace++;
//     fprintf(stderr, "Chegada do processo: %s - na linha %d do trace\n",
//             copia->nome, linha_arquivo_trace);
//   }    

//   while(copia != NULL)
//   {
//     /* Enquanto o t0 do processo nao entra o sistema espera por isso */
//     /* DEPURACAO printf("temp - t0: %d - copia: %s - %d\n", tempoSistema - tempo_inicialSistema, copia->nome, copia->t0);*/
//     while(calcular_tempo_decorrido() < copia->t0)
//     {
//       usleep(100);
//     }

//     /* procura o proximo processador disponivel */
//     for(threadID = 0; processador_em_uso[threadID] != LIVRE; threadID = (threadID + 1) % num_procs);
       

//     /* Criacao da thread */
//     /* sinalizamos que o processador esta sendo usado */
//     pthread_mutex_lock(&semaforo_processador[threadID]);
//     processador_em_uso[threadID] =  EM_USO;
//     pthread_mutex_unlock(&semaforo_processador[threadID]);

//     /* usaremos a variavel t0 agora para guardar a ID da thread - e nao mais o tempo inicial */
//     copia->t0 = threadID;
//     /* criamos a thread de fato e comecamos a executar na funcao thread_function */
//     if(pthread_create(&threads[threadID], NULL, thread_function, copia))
//     {
//       printf("error creating thread.");
//       abort();
//     }

//     /* 
//         volta a tirar um elemento da lista pra manter a condição de loop 
//     */
//     pthread_mutex_lock(&semaforo_lista_processos);
//     /* retirar elemento da lista */
//     if(lista_processos != NULL)
//     {
//       temp = lista_processos->prox;
//       copia = retirar_lista(lista_processos);
//       lista_processos = temp;

//       if(d)
//       {
//         linha_arquivo_trace++;
//         fprintf(stderr, "Chegada do processo: %s - na linha %d do trace\n",
//                 copia->nome, linha_arquivo_trace);
//       }
//     }
//     else copia = NULL;
//     pthread_mutex_unlock(&semaforo_lista_processos);
//   }
// }

void shortest_remaining_time_next(Processo *lista)
{
  merge_sort(&lista, 3);
  /*return lista;*/
}

void escalonamento_multiplas_filas(Processo *lista)
{
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

/* le o arquivo de entrada (trace) e devolve uma lista ligada de processos */
Processo *interpreta_entrada(char *nome_arquivo)
{
  Processo *lista, *proc, *novo_proc;
  FILE *arquivo_entrada = abre_arquivo(nome_arquivo);
  double t0;

  proc = malloc_safe(sizeof(Processo));
  lista = proc;

  proc->nome = malloc_safe(64 * sizeof(char));
  fscanf(arquivo_entrada,"%lf", &(proc->t0));
  fscanf(arquivo_entrada,"%s", proc->nome);
  fscanf(arquivo_entrada,"%lf", &(proc->dt));
  fscanf(arquivo_entrada,"%lf", &(proc->deadline));


  while(fscanf(arquivo_entrada, "%lf", &t0) == 1)
  {
    novo_proc = malloc_safe(sizeof(Processo));
    novo_proc->nome = malloc_safe(64 * sizeof(char));
    novo_proc->t0 = t0;
    fscanf(arquivo_entrada,"%s", novo_proc->nome);
    fscanf(arquivo_entrada,"%lf", &(novo_proc->dt));
    fscanf(arquivo_entrada,"%lf", &(novo_proc->deadline));
    proc->prox = novo_proc;
    proc = novo_proc;
  }

  return lista;
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
