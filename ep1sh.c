/* ********************************
    EP1 - Sistemas Operacionais
    Prof. Daniel Batista

    Danilo Aleixo Gomes de Souza
    n USP: 7972370
  
    Carlos Augusto Motta de Lima
    n USP: 7991228

********************************** */

#include "StringOps.h"
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/wait.h>

#define MAX_SIZE_DIR 128
#define MAX_ARGUMENTOS 15

int main(int argc, char *argv[])
{
  char *comando, *token, **argv_chamada, *temp[MAX_ARGUMENTOS],
        dir[MAX_SIZE_DIR], prefixo[MAX_SIZE_DIR + 3];
  pid_t pid;
  int i, conta_tokens;

  while(1)
  {
    getcwd(dir, MAX_SIZE_DIR);
    strcpy(prefixo, "(");
    strcat(prefixo, dir);
    strcat(prefixo, "): ");

    comando = readline(prefixo);
    
    if(strcmp(comando, ""))
    {
      add_history(comando);
      /* pega o primeiro token do comando */
      token = strtok(comando, " ");
    }
    else
      token = "";

    /* depuracao printf(">>>>%s\n", comando); */

    if(strcmp(token, "exit") == 0) break;
    else if(strcmp(token, "chmod") == 0)
    {
      temp[0] = strtok(NULL, " ");
      temp[1] = strtok(NULL, " ");

      long int permissao = strtol(temp[0], 0, 8);

      chmod(temp[1], permissao);
    }
    else if(strcmp(token, "") != 0)
    {
      if((pid = fork()) < 0)
      {
        perror("falha na criação de processo!");
        exit(1);
      }
      else if(pid == 0)
      {
        for(conta_tokens = 0; token != NULL; conta_tokens++)
        {
          temp[conta_tokens] = token;
          token = strtok(NULL, " ");
        }
        argv_chamada = mallocSafe((conta_tokens + 1) * sizeof(char *));
        
        for(i = 0; i < conta_tokens; i++)
          argv_chamada[i] = temp[i];
        argv_chamada[i] = NULL;

        if(execve(argv_chamada[0], argv_chamada, NULL) == -1)
        { 
          printf("falha no execve!\n");
          exit(1);
        }
      }
      else
      {
        waitpid(pid, NULL, 0);
      }
    }
  }
  free(comando);
  return 0;
}
