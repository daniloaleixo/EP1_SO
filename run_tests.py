import sys
import os
import random
import time
import subprocess

tipo_escalonamento = 1

if len(sys.argv) < 4:
	print "Modo de uso\n\n"
	print "geradorArquivosTrace <diretorio dos arquivos de trace> <diretorio arquivos de saida> <diretorio result>\n"
	print "-t\ttipo de escalonamento\n"
	print "\n"
	exit(0)
	
directory = sys.argv[1]
dir_saida = sys.argv[2]
dir_result = sys.argv[3]

# pega os argumentos de entrada
if len(sys.argv) > 4:
	tamanho = len(sys.argv) - 1
	while(tamanho > 3):
		if sys.argv[tamanho - 1] == '-t':
			tipo_escalonamento = sys.argv[tamanho]
			tamanho = tamanho - 2


#Executa os testes
arquivos = os.listdir(directory)
file_tempos = open("resultado_tempos.txt", "w")

for i in range(0,len(arquivos)):
	nome_arquivo_saida = dir_saida + "saida" + str(i) + ".txt"
	arquivo_saida = open(nome_arquivo_saida, "w")
	nome_arquivo_result = dir_result + "result" + str(i) + ".txt"
	arquivo_result = open(nome_arquivo_result, "w")

	text_cmd = "./ep1 " + str(tipo_escalonamento) + " " + str(directory) +	str(arquivos[i]) + " " + nome_arquivo_saida  + " > " + nome_arquivo_result
	print text_cmd

	start_time = time.time()
	cmd = os.system(text_cmd)
	file_tempos.write(str(time.time() - start_time) + "\n")

file_tempos.close()

results = os.listdir(dir_result)
text_cmd = "cat "
for arq in results:
	text_cmd = text_cmd + " " + dir_result + arq

text_cmd = text_cmd + " > resultados_deadline.txt"
print text_cmd
os.system(text_cmd)




