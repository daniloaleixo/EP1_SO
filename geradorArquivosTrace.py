import sys
import os
import random
import time

n_arquivos_trace = 30
processos_gerados_por_segundo = 2
n_processos_por_arquivo = 10
longevidade_do_proc = 5
proporcao_deadline = 2
count = 0
tempo = 0

random.seed(time.time())



if len(sys.argv) < 2:
	print "Modo de uso\n\n"
	print "geradorArquivosTrace <diretorio dos arquivos de trace>\n"
	print "-n\tnumero de arquivos trace\n"
	print "-N\tnumero de processos de cada arquivo de trace"
	print "-p\tprocessos gerados por segundo"
	print "-l\tquanto tempo um processo pode durar no maximo"
	print "-d\tproporcao deadline em relacao ao dt do processo"
	print "\n"
	exit(0)
	
directory = sys.argv[1]

# pega os argumentos de entrada
if len(sys.argv) > 2:
	tamanho = len(sys.argv) - 1
	while(tamanho > 1):
		if sys.argv[tamanho - 1] == '-n':
			n_arquivos_trace = sys.argv[tamanho]
			tamanho = tamanho - 2
		if sys.argv[tamanho - 1] == '-p':
			processos_gerados_por_segundo = sys.argv[tamanho]
			tamanho = tamanho - 2
		if sys.argv[tamanho - 1] == '-N':
			n_processos_por_arquivo = sys.argv[tamanho]
			tamanho = tamanho - 2
		if sys.argv[tamanho - 1] == '-l':
			longevidade_do_proc = sys.argv[tamanho]
			tamanho = tamanho - 2
		if sys.argv[tamanho - 1] == '-d':
			proporcao_deadline = sys.argv[tamanho]
			tamanho = tamanho - 2

# se nao tem o dirretorio cria
if not os.path.exists(directory):
	os.makedirs(directory)

#gera os aqruivos de trace
for arq_i in range(0, int(n_arquivos_trace)):
	tempo = 0
	file = open(directory + "arquivoTrace" + str(arq_i) + ".txt", "w")
	for proc_i in range(0, int(n_processos_por_arquivo)):
		if proc_i != 0 and proc_i % int(processos_gerados_por_segundo) == 0:
			tempo = tempo + 1
		# gera dt e deadline aleatorios
		dt = random.randrange(1, int(longevidade_do_proc) + 1)
		deadline = random.randrange(tempo + dt, tempo + dt * int(proporcao_deadline) + 1)
		file.write(str(tempo) + " processo" + str(proc_i) + " " + str(dt) + " " + str(deadline) + "\n")
		
	file.close()
	