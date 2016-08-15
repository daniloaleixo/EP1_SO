CC = gcc
CFLAGS = -Wall -g -O0
LFLAGS = -lreadline -lpthread
OUT = ep1 
IN = ep1.c
OBJS = ep1.o StringOps.o

all: $(OUT) ep1sh

clean:
	rm -f $(OUT) $(OBJS) ep1sh.o ep1sh
	

ep1sh: ep1sh.o StringOps.o
	$(CC) ep1sh.o StringOps.o $(LFLAGS) -o ep1sh

ep1sh.o: ep1sh.c
	$(CC) ep1sh.c -c -o ep1sh.o

$(OUT): $(OBJS)
	$(CC) $(OBJS) $(LFLAGS) -o $(OUT)

ep1.o: $(IN)
	$(CC) $(IN) -c -o ep1.o

StringOps.o: StringOps.c
	$(CC) StringOps.c -c -o StringOps.o
