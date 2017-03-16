# EP1 - Sistemas Operacionais #

## Problema ## 

A tarefa neste EP é implementar um shell bem simples, para permitir a interação do usuário com o
sistema operacional, e um simulador de processos com diversos algoritmos de escalonamento para esses
processos. Todos os códigos devem ser escritos em C para serem executados no GNU/Linux.

### O Shell ep1sh
O shell, chamado de ep1sh, a ser desenvolvido, deve permitir a invocação (execução) dos 3 binários abaixo com exatamente os argumentos abaixo. Não há necessidade de testar o shell para outros programas e nem para os binários abaixo com outros argumentos:
* /bin/ls -l
* /bin/date
* ./ep1 <argumentos do EP1>
O shell também precisa ter os 2 comandos abaixo embutidos nele, que devem obrigatoriamente ser implementados usando chamadas de sistema do Linux. Esses comandos devem ser executados sempre com os argumentos abaixo e que devem fazer exatamente o que esses 2 comandos fazem no shell bash:
* chmod <modo numérico> <arquivo no diretório atual>
* id -u
Não se preocupe em tratar os erros do chmod. O usuário nunca vai tentar mudar a permissão de algum arquivo que não existe ou usar algum valor no modo numérico incorreto. Ele também nunca vai tentar mudar a permissão de algum diretório ou link. Ele sempre mudará a permissão usando o modo numérico do chmod.
O shell tem que suportar a listagem de comandos que foram executados previamente, e a edição desses comandos para serem executados, por meio das funcionalidades das bibliotecas GNU readline e GNU history. No Debian ambas fazem parte do pacote libreadline-dev. Mais informações podem ser vistas na documentação da biblioteca em ftp://ftp.gnu.org/pub/gnu/readline/ . Não há necessidade de utilizar outras funcionalidades das bibliotecas além das requisitadas no inı́cio deste parágrafo.
O prompt do shell deve conter o diretório atual entre parênteses seguido de ‘:’ e de um espaço em branco, como no exemplo abaixo que mostra o shell pronto para rodar o comando id -u que imprimirá o UID do usuário:
```sh
(/home/mac/): id -u
```

### Simulador de Processos ### 
O simulador de processos deve receber como entrada um arquivo de trace, em texto puro, que possui várias linhas como a seguinte:
```sh
t0 nome dt deadline
```
t0 é o instante de tempo em segundos que o processo chega no sistema, nome é uma string sem espaços em branco que identifica o processo, dt é o quanto de tempo real da CPU deve ser simulado para aquele processo e deadline é o instante de tempo antes do qual aquele processo precisa terminar.
t0, dt e deadline são números reais.
Cada linha do arquivo de entrada representa portanto um processo, que deverá ser simulado no simulador como uma única thread. Cada thread precisa ser um loop que realize qualquer operação que consuma tempo real da CPU. Não há uma predefinição de qual deve ser essa operação.
Assim, se o simulador receber como entrada um arquivo que contenha apenas a linha:
```sh
1 processo0 10 11
```
é de se esperar, num cenário ideal, que no instante de tempo 1 segundo uma thread seja criada para representar o processo0 e que no instante de tempo 11 segundos o processo0 termine de executar.
O simulador deve finalizar sua execução assim que todos os processos terminarem de ser simulados.
O simulador será mais interessante de ser executado com traces que permitam mais de um processo ao mesmo tempo competindo pela CPU (ou pelas CPUs em casos onde o computador tenha mais de 1 unidade de processamento). Nessas situações o escalonador de processos implementado no simulador
terá um papel fundamental e provavelmente levará a diferentes resultados. 
Diversos escalonadores de processos existem. Neste EP o simulador deve implementar os seguintes escalonadores:
1. First-Come First-Served
2. Shortest Remaining Time Next
3. Escalonamento com múltiplas filas
A invocação do simulador no ep1sh deve receber como primeiro parâmetro obrigatório o número representando cada escalonador, conforme a listagem acima, como segundo parâmetro obrigatório o nome do arquivo de trace e como terceiro parâmetro obrigatório o nome de um arquivo que será criado pelo simulador com 1 linha para cada processo e mais 1 linha extra no final. Cada linha por processo deverá ter o seguinte formato:
```sh
nome tf tr
```
Onde nome é o identificador do processo, tf é o instante de tempo quando o processo terminou sua execução e tr é o tempo “de relógio” que o processo levou para executar, ou seja, tf-t0.
A linha extra deve conter um único número que informará a quantidade de mudanças de contexto que ocorreram durante a simulação. Ou seja, a quantidade de vezes que as CPUs deixaram de rodar um processo para rodar outro.
O simulador deve receber ainda como quarto parâmetro opcional, o caracter d. Quando esse parâmetro for usado, o simulador deverá exibir os seguintes eventos, assim que eles acontecerem, na saı́da de erro (stderr):
* chegada de um processo no sistema, informando a linha do trace daquele processo
* uso da CPU por um processo, informando qual o processo que começou a usar a CPU e qual CPU ele está usando
* liberação da CPU por um processo, informando qual o processo que deixou de usar a CPU e qual CPU ele está liberando
* finalização da execução do processo, informando a linha que será escrita no arquivo de saı́da
* quantidade de mudanças de contexto
O formato de exibição dessas informações é livre.

## Execução ##

Primeiramente basta utilizar o comando "make" para compilar os arquivos executáveis.
O executável ep1sh é o shell conforma o item 1.1 do enunciado do EP1 
Já o executável ep1 é a parte 1.2 do enunciado, que corresponde ao escalonador de processos.
A chamada dos programas deve ser feita conforme especificada no enunciado (rode ep1sh através do seu terminal e ep1 a partir do terminal ep1sh)


