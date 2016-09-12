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
  double tempo_restante; /* quanto tempo real de CPU o processo já consumiu */
  int processador;
  int linha_no_arquivo_trace;
  struct processo *prox;
} Processo;

/* funcoes de threads */
void *thread_function_fcfs(void *arg);
void *thread_function_srtn(void *arg);

/* funcoes de escalonamento */
void first_come_first_served();
void shortest_remaining_time_next();
void escalonamento_multiplas_filas();

/* funcoes auxliares*/
void conta_mudancas_de_contexto(int *pids_em_execucao);
void imprime_todos_procs(Processo *lista);
Processo *ordenar_metodo2(Processo *lista);
Processo *retira_primeiro_elemento_da_lista();
Processo *interpreta_entrada(char *nome_arquivo);
Processo *sorted_merge(Processo *a, Processo *b, int mode);
void front_back_split(Processo *source, Processo **frontRef,
                      Processo **backRef);
void merge_sort(Processo **headRef, int mode);
int compare(Processo *a, Processo *b, int mode);
void insere_na_lista_espera(Processo *proc);
void ordenar_fila_espera();
Processo *retira_primeiro_elemento_da_lista_espera();
void insere_na_lista_execucao(Processo *proc);
void insere_na_lista_processos(Processo *proc);
/*------------------------------*/

Processo *lista_processos = NULL, *lista_execucao = NULL, *lista_espera = NULL;
pthread_mutex_t semaforo_lista_processos = PTHREAD_MUTEX_INITIALIZER,
                semaforo_lista_espera = PTHREAD_MUTEX_INITIALIZER,
                semaforo_lista_execucao = PTHREAD_MUTEX_INITIALIZER,
                semaforo_arq_saida = PTHREAD_MUTEX_INITIALIZER,
                *semaforo_processador;
pthread_t *threads;
FILE *arquivo_saida;
int num_procs = 0, depurar = FALSE, linha_arquivo_saida = 0,
    qtde_mudancas_contexto = 0, *estado_processador,
    *tempo_restante_do_pid, contador_deadlines_estourados = 0;
double tempo_de_execucao = 0;

int verbose = FALSE;


int main(int argc, char *argv[])
{
  int i;
  char *nome_saida;
  
  inicializa_relogio();

  if(argc >= 4)
  {
    depurar = (argc == 5 && strcmp(argv[4], "d") == 0);

    /* le o arquivo de trace da entrada e coloca todos os processos numa
       lista encadeada (lista_processos) */
    lista_processos = interpreta_entrada(argv[2]);

    nome_saida = argv[3];
    arquivo_saida = cria_arquivo(nome_saida);

    /* pega o numero de processadores e aloca um vetor de inteiros para
       guardar o estado atual (EM_USO ou LIVRE) de cada processador,
       alem de alocar um vetor de semaforos (um por processador) e um vetor de
       threads (uma por processador) */
    num_procs = get_nprocs();
    estado_processador = malloc_safe(num_procs * sizeof(int));
    for(i = 0; i < num_procs; i++)
      estado_processador[i] = LIVRE;
    /* inicializamos os semaforos das threads */
    semaforo_processador = malloc_safe(num_procs * sizeof(pthread_mutex_t));
    for (i = 0; i < num_procs; i++) 
      pthread_mutex_init(&semaforo_processador[i], NULL);
    threads = malloc_safe(num_procs * sizeof(pthread_t));

    /* escolhe o metodo de escalonamento */
    switch(atoi(argv[1]))
    {
      case 1:
        first_come_first_served();
        break;

      case 2:
        shortest_remaining_time_next();
        break;

      case 3:
        escalonamento_multiplas_filas(lista_processos);
        break;
    }
  }
  else
    printf("Argumentos incorretos. Modo de utilizacao:\n\n"
           "\tep1 <numero do Metodo de Escalonamento> <arquivo de entrada> "
           "<arquivo de saida>\n\n");

  /* espera ate que todos os processadores estejam LIVREs (ou seja, espera
     que todas as threads terminem a execucao) */
  if(num_procs != 0)
  {  
    /*for(i = 0; i < num_procs; i++){
      if(estado_processador[i] == EM_USO) i = 0;            Tirei porque tava travando
	/* DEPURACAO * printf("estou em uso\n");*                  coloquei o join ao inves
	}*/
    for(i = 0; i < num_procs; i++)
    	pthread_join(threads[i], NULL);
    
    fprintf(arquivo_saida, "\n");
    fclose(arquivo_saida);

    if(depurar)
      fprintf(stderr, "Quantidade de mudanças de contexto: %d\n",
              qtde_mudancas_contexto);
    printf("%d\n", qtde_mudancas_contexto);
  }

  return 0;
}

void *thread_function_fcfs(void *arg)
{
  float t0_processo = tempo_decorrido(), tf,
        tempo_decorrido_processo = 0;
  int i;
  Processo *proc = (Processo *) arg;

  if(depurar)
    fprintf(stderr, "%8.4fs | O processo %s esta usando a CPU: %d\n",
            tempo_decorrido(), proc->nome, proc->processador);

  while(tempo_decorrido_processo < proc->dt)
  {
    tempo_decorrido_processo = tempo_decorrido() - t0_processo;
    i = (i + 1) % 100000;
  }

  pthread_mutex_lock(&semaforo_arq_saida);
  fprintf(arquivo_saida, "%s %f %f\n",
          proc->nome, tf = tempo_decorrido(), tf - proc->t0);
  if(depurar)
    fprintf(stderr, "%8.4fs | O processo %s terminou e esta na linha %d do "
                    "arquivo de saida\n", tempo_decorrido(), proc->nome,
                    ++linha_arquivo_saida);
  pthread_mutex_unlock(&semaforo_arq_saida);

  if(tempo_decorrido() > proc->deadline) contador_deadlines_estourados++;


  pthread_mutex_lock(&semaforo_processador[proc->processador]);
  estado_processador[proc->processador] = LIVRE;
  pthread_mutex_unlock(&semaforo_processador[proc->processador]);

  if(depurar)
    fprintf(stderr, "%8.4fs | O processo %s esta deixando a CPU: %d\n",
            tempo_decorrido(), proc->nome, proc->processador);
  return NULL;
}

void *thread_function_srtn(void *arg)
{
  float t0_processo = tempo_decorrido(), tf,
        tempo_decorrido_processo = 0;
  int i;
  Processo *proc = (Processo *) arg, *aux;

  if(depurar)
    fprintf(stderr, "%8.4fs | O processo %s esta usando a CPU: %d\n",
            tempo_decorrido(), proc->nome, proc->processador); 

  /* gasta tempo de CPU */
  while(tempo_decorrido_processo < tempo_de_execucao)
  {
    tempo_decorrido_processo = tempo_decorrido() - t0_processo;
    i = (i + 1) % 100000;
  }

  /* processo terminou */
  if(proc->tempo_restante - tempo_de_execucao <= 0)
  {
    pthread_mutex_lock(&semaforo_arq_saida);
    fprintf(arquivo_saida, "%s %f %f\n",
            proc->nome, tf = tempo_decorrido(), tf - proc->t0);
    if(depurar)
      fprintf(stderr, "%8.4fs | O processo %s terminou e esta na linha %d do "
                      "arquivo de saida\n", tempo_decorrido(), proc->nome,
                      ++linha_arquivo_saida);
    pthread_mutex_unlock(&semaforo_arq_saida);

    /* Verificar se a thread terminou */
    if(tempo_decorrido() > proc->deadline) contador_deadlines_estourados++; 
  } 


  if(depurar)
    fprintf(stderr, "%8.4fs | O processo %s esta deixando a CPU: %d\n",
            tempo_decorrido(), proc->nome, proc->processador);


  /* Atualizamos o tempo restante do processo */
  proc->tempo_restante = proc->tempo_restante - tempo_de_execucao;
  tempo_restante_do_pid[proc->linha_no_arquivo_trace] = proc->tempo_restante;
}

/* *****************************************

                funcoes de escalonamento

  ***************************************** */

void first_come_first_served()
{
  Processo *processo_atual;
  int i;

  /* pega da lista o primeiro processo */
  pthread_mutex_lock(&semaforo_lista_processos);
  processo_atual = retira_primeiro_elemento_da_lista();
  pthread_mutex_unlock(&semaforo_lista_processos);

  while(processo_atual != NULL)
  {
    if(depurar)
      fprintf(stderr, "%8.4fs | Chegada do processo %s - linha %d no trace\n",
              tempo_decorrido(), processo_atual->nome,
              processo_atual->linha_no_arquivo_trace);

    /* Espera até que o processo chegue (t >= t0) */
    while(tempo_decorrido() < processo_atual->t0) usleep(100);

    /* procura um processador disponivel */
    for(i = 0; estado_processador[i] != LIVRE; i = (i + 1) % num_procs);
       
    /* marca o processador encontrado como EM_USO e, no processo a ser rodado,
       o processador que ele está usando como i+1 */
    pthread_mutex_lock(&semaforo_processador[i]);
    estado_processador[i] = EM_USO;
    processo_atual->processador = i;
    pthread_mutex_unlock(&semaforo_processador[i]);

    /* cria uma thread para o processo_atual e roda ela durante
       processo_atual->dt segundos (com consumo de CPU) */
    if(pthread_create(&threads[i], NULL, thread_function_fcfs, processo_atual))
    {
      printf("Erro na criacao da thread.\n");
      abort();
    }
    
    /* tira o proximo processo da lista */
    pthread_mutex_lock(&semaforo_lista_processos);
    processo_atual = retira_primeiro_elemento_da_lista();
    pthread_mutex_unlock(&semaforo_lista_processos);
  }
}

void shortest_remaining_time_next(Processo *lista)
{
  double primeiro_t0, proximo_t0;
  int i, *pids_em_execucao;
  Processo *primeiro_processo, *processo_atual, *proximo_processo;

  pids_em_execucao = malloc_safe(num_procs * sizeof(int));

  /* PRIMEIRA ITERACAO */

  /* pega da lista o primeiro processo */
  pthread_mutex_lock(&semaforo_lista_processos);
  primeiro_processo = retira_primeiro_elemento_da_lista();
  pthread_mutex_unlock(&semaforo_lista_processos);

  insere_na_lista_espera(primeiro_processo);
  
  /* Pegamos da lista de processos todos os processos que chegam junto com
     o primeiro processo, inserindo todos na lista de espera */
  primeiro_t0 = primeiro_processo->t0;
  for(processo_atual = retira_primeiro_elemento_da_lista();
      processo_atual != NULL && processo_atual->t0 <= primeiro_t0;
      processo_atual = retira_primeiro_elemento_da_lista())
    insere_na_lista_espera(processo_atual);

  /* Se o utlimo processo que verificamos nao e nulo e so saiu do for por causa
     do seu t0, colocamos ele de novo na lista_procesos */
  if(processo_atual != NULL) insere_na_lista_processos(processo_atual);

  /* Espera até que o processo chegue (t >= t0) */
  while(tempo_decorrido() < primeiro_t0) usleep(100);

  /* nas linhas abaixo, se verifica quanto tempo de execucao daremos para os
     processos */
  proximo_processo = lista_processos;
  if(proximo_processo != NULL)
  {
    proximo_t0 = proximo_processo->t0;
    tempo_de_execucao = proximo_t0 - primeiro_t0;
  }

  ordenar_fila_espera();

  /* verificamos se algum processo vai terminar antes de que se rode os
     processos durante tempo_de_execucao */
  for(processo_atual = lista_espera, i = 0;
      i < num_procs && processo_atual != NULL;
      i++, processo_atual = processo_atual->prox)
    if(processo_atual->tempo_restante < tempo_de_execucao ||
       tempo_de_execucao == 0)
      tempo_de_execucao = processo_atual->tempo_restante;

  /* Se so existe um processo que precisa ser rodado na fila de espera e nao
     existe processos que irao entrar entao executamos ele ate o fim */
  if(lista_processos == NULL &&
     lista_espera != NULL &&
     lista_espera->prox == NULL)
    tempo_de_execucao = lista_espera->tempo_restante;

  /* executamos os processos que estao em primeiro lugar na fila, de acordo com
     seu remaining time */
  for(processo_atual = retira_primeiro_elemento_da_lista_espera(), i = 0;
      i < num_procs && processo_atual != NULL;
      processo_atual = retira_primeiro_elemento_da_lista_espera(), i++)
  {
    processo_atual->processador = i;
    /* cria uma thread para o processo_atual e roda ela durante
       tempo_de_execucao segundos (com consumo de CPU) */
    if(pthread_create(&threads[i], NULL, thread_function_srtn, processo_atual))
    {
      printf("Erro na criacao da thread.\n");
      abort();
    }
    pids_em_execucao[i] = processo_atual->linha_no_arquivo_trace;
    insere_na_lista_execucao(processo_atual);
  }

  /* Se o utlimo processo que verificamos nao e nulo e so saiu porque acabou os
     processdaores disponvieis, colocamos ele de novo na lista_procesos */
  if(processo_atual != NULL) insere_na_lista_espera(processo_atual);

  /* esperamos os processos terminarem */
  for(i = 0; i < num_procs; ++i) pthread_join(threads[i], NULL);

  /* vamos verificar os processos que ja acabaram a execucao e tira-los da
     lista de execucao */
  for(processo_atual = lista_execucao; processo_atual != NULL;
      processo_atual = processo_atual->prox)
    if(processo_atual->tempo_restante <= 0)
      lista_execucao = processo_atual->prox;

  /* retiramos todos os procesos da lista de execucao e colocamos na lista de
     espera. */
  if(lista_espera != NULL)
  {
    for(processo_atual = lista_espera;
        processo_atual->prox != NULL;
        processo_atual = processo_atual->prox);
    processo_atual->prox = lista_execucao;
  }
  else 
    lista_espera = lista_execucao;

  /* E reseta a lista de execucao */
  lista_execucao = NULL;

  /* TERMINADA A PRIMEIRA ITERACAO, A REPETIMOS NO LOOP ABAIXO ATE QUE SE
     TERMINEM TODOS OS PROCESSOS */

  while(lista_espera != NULL || lista_processos != NULL)
  {
    if(lista_processos != NULL)
    {
      /* pega da lista o primeiro processo */
      pthread_mutex_lock(&semaforo_lista_processos);
      primeiro_processo = retira_primeiro_elemento_da_lista();
      pthread_mutex_unlock(&semaforo_lista_processos);

      insere_na_lista_espera(primeiro_processo);
      
      /* Pegamos da lista de processos todos os processos que chegam junto com
         o primeiro processo, inserindo todos na lista de espera */
      primeiro_t0 = primeiro_processo->t0;
      for(processo_atual = retira_primeiro_elemento_da_lista();
          processo_atual != NULL && processo_atual->t0 <= primeiro_t0;
          processo_atual = retira_primeiro_elemento_da_lista()){
        insere_na_lista_espera(processo_atual);
      }
      /* Se o utlimo processo que verificamos nao e nulo e so saiu do for por
         causa do seu t0, colocamos ele de novo na lista_procesos */
      if(processo_atual != NULL) insere_na_lista_processos(processo_atual);

      /* nas linhas abaixo, se verifica quanto tempo de execucao daremos para os
         processos */
      proximo_processo = retira_primeiro_elemento_da_lista();
      if(proximo_processo != NULL)
      {
        proximo_t0 = proximo_processo->t0;
        tempo_de_execucao = proximo_t0 - primeiro_t0;
      }
    } /* fim do if lista_proecssos != NULL*/

    ordenar_fila_espera();

    /* verificamos se ocorrera' algum evento (fim de um processo) antes de t0
       que estipulamos que termina quando um processo for entrar no sistema */
    for(processo_atual = lista_espera, i = 0;
        i < num_procs && processo_atual != NULL;
        i++, processo_atual = processo_atual->prox)
      if(processo_atual->tempo_restante < tempo_de_execucao ||
         tempo_de_execucao == 0)
        tempo_de_execucao = processo_atual->tempo_restante;  

    /* Se so existe um processo que precisa ser rodado na fila de espera e
       nao existe processos que irao entrar entao executamos ele ate o fim */
    if(lista_processos == NULL &&
       lista_espera != NULL &&
       lista_espera->prox == NULL)
      tempo_de_execucao = lista_espera->tempo_restante;

    /* executamos os processos que estao em primeiro lugar na fila,
       de acordo com seu remaining time */
    processo_atual = retira_primeiro_elemento_da_lista_espera();
    for(i = 0; i < num_procs && processo_atual != NULL; i++)
    {
      processo_atual->processador = i;
      /* cria uma thread para o processo_atual e roda ela durante
         processo_atual->dt segundos (com consumo de CPU) */
      if(pthread_create(&threads[i], NULL, thread_function_srtn, processo_atual))
      {
        printf("Erro na criacao da thread.\n");
        abort();
      }
      insere_na_lista_execucao(processo_atual);
      processo_atual = retira_primeiro_elemento_da_lista_espera();
    }
    /* contamos quantas mudancas de contexto houve e atualizamos o vetor de
       controle pids_em_execucao*/
    conta_mudancas_de_contexto(pids_em_execucao);
    for(i = 0, processo_atual = lista_execucao; i < num_procs; i++)
    {
      if(processo_atual != NULL)
      {
        pids_em_execucao[i] = processo_atual->linha_no_arquivo_trace;
        processo_atual = processo_atual->prox;
      }
      else
        pids_em_execucao[i] = -1;
    }

    /* Se o ultimo processo que verificamos nao e nulo e so saiu porque acabou
       os processdaores disponvieis, colocamos ele de novo na lista_processos */
    if(processo_atual != NULL) insere_na_lista_espera(processo_atual);

    /* esperamos os processos rodarem o tempo estipulado */
    for(i = 0; i < num_procs; ++i) pthread_join(threads[i], NULL);

    /* vamos verificar os processos que ja acabaram a execucao e tira-los
       da lista de execucao */
    for(processo_atual = lista_execucao;
        processo_atual != NULL && processo_atual->tempo_restante <= 0; 
        processo_atual = processo_atual->prox)
      lista_execucao = processo_atual->prox;

    /* retiramos todos os procesos da lista de execucao e colocamos na lista
       de espera */
    if(lista_espera != NULL)
    {
      for(processo_atual = lista_espera;
          processo_atual->prox != NULL;
          processo_atual = processo_atual->prox);
      processo_atual->prox = lista_execucao;
    }
    else
      lista_espera = lista_execucao;

    /* E reseta a lista de execucao */
    lista_execucao = NULL;
  }
}

void escalonamento_multiplas_filas(Processo *lista)
{
  printf("nao fui implementado.\n");
  /* merge_sort(&lista, 5);
     return lista;*/
}

/* funcoes auxiliares */

void conta_mudancas_de_contexto(int *pids_em_execucao)
{
  
  int i, pid, candidato_mudanca;
  Processo *proc;
  for(i = 0; i < num_procs; i++)
  {
    pid = pids_em_execucao[i];
    candidato_mudanca = TRUE;
    if(pid == -1) continue;
    for(proc = lista_execucao; proc != NULL; proc = proc->prox)
      if(pid == proc->linha_no_arquivo_trace)
        candidato_mudanca = FALSE;
    if(candidato_mudanca == TRUE && tempo_restante_do_pid[pid] > 0)
      qtde_mudancas_contexto++;
  }
}

Processo *retira_primeiro_elemento_da_lista()
{
  if(lista_processos == NULL) return NULL;
  Processo *elemento = lista_processos;
  lista_processos = lista_processos->prox;
  elemento->prox = NULL;
  return elemento;
}

Processo *ordenar_metodo2(Processo *lista)
{
  merge_sort(&lista, 2);
  return lista;
}

/* imprime todos os processos da lista encadeada de processos */
void imprime_todos_procs(Processo *lista)
{
  Processo *proc;
  if(proc == NULL) printf(">>Esta vazia\n"); 
  for(proc = lista; proc != NULL; proc = proc->prox)
    printf(">>%s :::tempo_restante%lf :::dt%lf\n", proc->nome, proc->tempo_restante, proc->dt);
}

/* le o arquivo de entrada (trace) e devolve uma lista ligada de processos */
Processo *interpreta_entrada(char *nome_arquivo)
{
  Processo *lista, *proc, *novo_proc;
  FILE *arquivo_entrada = abre_arquivo(nome_arquivo);
  double t0;
  int i = 1;

  proc = malloc_safe(sizeof(Processo));
  lista = proc;

  proc->nome = malloc_safe(64 * sizeof(char));
  proc->processador = -1;
  proc->linha_no_arquivo_trace = i++;
  fscanf(arquivo_entrada,"%lf", &(proc->t0));
  fscanf(arquivo_entrada,"%s", proc->nome);
  fscanf(arquivo_entrada,"%lf", &(proc->dt));
  fscanf(arquivo_entrada,"%lf", &(proc->deadline));
  proc->tempo_restante = proc->dt;

  while(fscanf(arquivo_entrada, "%lf", &t0) == 1)
  {
    novo_proc = malloc_safe(sizeof(Processo));
    novo_proc->nome = malloc_safe(64 * sizeof(char));
    novo_proc->t0 = t0;
    novo_proc->processador = -1;
    novo_proc->linha_no_arquivo_trace = i++;
    fscanf(arquivo_entrada,"%s", novo_proc->nome);
    fscanf(arquivo_entrada,"%lf", &(novo_proc->dt));
    fscanf(arquivo_entrada,"%lf", &(novo_proc->deadline));
    novo_proc->tempo_restante = novo_proc->dt;
    proc->prox = novo_proc;
    proc = novo_proc;
  }

  tempo_restante_do_pid = malloc_safe(i * sizeof(int));
  for(proc = lista; proc != NULL; proc = proc->prox)
    tempo_restante_do_pid[proc->linha_no_arquivo_trace] = proc->tempo_restante;

  return lista;
}


void insere_na_lista_espera(Processo *proc)
{
  if(lista_espera == NULL)
    lista_espera = proc;
  else
  {
    proc->prox = lista_espera;
    lista_espera = proc;
  }
}

void insere_na_lista_processos(Processo *proc)
{
  if(lista_processos == NULL)
    lista_processos = proc;
  else
  {
    proc->prox = lista_processos;
    lista_processos = proc;
  }
}

void ordenar_fila_espera()
{
  merge_sort(&lista_espera, 2);
}

Processo *retira_primeiro_elemento_da_lista_espera()
{ 
  if(lista_espera == NULL) return NULL;
  Processo *elemento = lista_espera;
  lista_espera = lista_espera->prox;
  elemento->prox = NULL;
  return elemento;
}

void insere_na_lista_execucao(Processo *proc)
{
  Processo *aux;

  if(lista_execucao == NULL)
    lista_execucao = proc;
  else {
    for(aux = lista_execucao; aux->prox != NULL; aux = aux->prox);
    aux->prox = proc;
    proc->prox = NULL;
  }
}


/* MERGE SORT
   FONTE: http://www.geeksforgeeks.org/merge-sort-for-linked-list/
*/
 
/* sorts the linked list by changing next pointers (not data) */
void merge_sort(Processo** headRef, int mode)
{
  Processo* head = *headRef;
  Processo* a;
  Processo* b;

  /* Base case -- length 0 or 1 */
  if((head == NULL) || (head->prox == NULL)) return;
 
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
  if(a == NULL)
     return(b);
  else if(b==NULL)
     return(a);
 
  /* Pick either a or b, and recur */
  if(compare(a, b, mode))
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
    return (a->tempo_restante <= b->tempo_restante ? 1 : 0);
  else if(mode == 3)
    return (a->deadline <= b->deadline ? 1 : 0);
}
