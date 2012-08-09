CC=gcc
CFLAGS=-Wall

all: a2.o
	$(CC) $(CFLAGS) a2.o test.c -o test

a2: a2.c

clean:
	rm -rf *.o
	rm -rf test
