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
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define MAX_SIZE_DIR 256
#define MAX_ARGUMENTOS 15

int main(int argc, char *argv[])
{
  char *linha_de_comando, *comando, *argumento[MAX_ARGUMENTOS],
       dir[MAX_SIZE_DIR], prefixo[MAX_SIZE_DIR + 3];
  int conta_args;
  long int permissao;
  pid_t pid;

  while(1)
  {
    getcwd(dir, MAX_SIZE_DIR);
    strcpy(prefixo, "(");
    strcat(prefixo, dir);
    strcat(prefixo, "): ");
    linha_de_comando = readline(prefixo);
    if(strcmp(linha_de_comando, "") == 0) continue;
    add_history(linha_de_comando);
    comando = strtok(linha_de_comando, " ");

    if(strcmp(comando, "exit") == 0)
      break;
    else if(strcmp(comando, "chmod") == 0)
    {
      permissao = strtol(strtok(NULL, " "), 0, 8);
      chmod(strtok(NULL, " "), permissao);
    }
    else if(strcmp(comando, "id") == 0)
    {
      if(strcmp(strtok(NULL, " "), "-u") == 0)
        printf("%d\n", getuid());
    }
    else
    {
      if((pid = fork()) < 0)
      {
        perror("falha na criação de processo!");
        exit(1);
      }
      else if(pid == 0)
      {
        argumento[0] = comando;
        conta_args = 1;
        while(argumento[conta_args - 1] != NULL && conta_args < MAX_ARGUMENTOS)
          argumento[conta_args++] = strtok(NULL, " ");

        if(execve(argumento[0], argumento, NULL) == -1)
        { 
          printf("falha no execve!\n");
          exit(1);
        }
      }
      else waitpid(pid, NULL, 0);
    }
  }
  free(linha_de_comando);
  return 0;
}
