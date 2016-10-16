CC = gcc
CFLAGS = -Wall -g -O0 -pedantic
LFLAGS = -lreadline -lpthread
OUT = ep1sh ep1
OBJS = ep1.o auxiliares.o

all: $(OUT)

clean:
	rm -f $(OUT) $(OBJS) ep1sh.o ep1sh

ep1: auxiliares.o
	$(CC) ep1.c auxiliares.o $(LFLAGS) -o ep1

ep1sh:
	$(CC) ep1sh.c $(LFLAGS) $(CFLAGS) -o ep1sh

auxiliares.o: auxiliares.c
	$(CC) auxiliares.c -c -o auxiliares.o
