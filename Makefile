CC = gcc
CFLAGS = -Wall -g -O0
LFLAGS = -lreadline -lpthread
OUT = ep1sh ep1
OBJS = ep1.o StringOps.o

all: $(OUT)

clean:
	rm -f $(OUT) $(OBJS) ep1sh.o ep1sh

ep1: StringOps.o
	$(CC) ep1.c StringOps.o $(LFLAGS) -o ep1

ep1sh: StringOps.o
	$(CC) ep1sh.c StringOps.o $(LFLAGS) -o ep1sh

StringOps.o: StringOps.c
	$(CC) StringOps.c -c -o StringOps.o
