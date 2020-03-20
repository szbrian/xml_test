CC = gcc
FLAG = -g
LIB = -lxml

all: test clean

test: xmlproc.o
	$(CC) -o $@ $^ $(LIB)

clean:
	rm *.o

.c.o:
	$(CC) -c $(FLAG) *.c
