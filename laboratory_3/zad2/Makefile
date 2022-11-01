CC = gcc
CFLAGS = -Wall

all: program test

program:
	$(CC) $(CFLAGS) -o program.o main.c

test:
	./program.o file.txt

clean:
	rm -f *.o
	rm -r r t
