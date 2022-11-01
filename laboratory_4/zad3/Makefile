CC = gcc
CFLAGS = -Wall

all: test

test:
	$(CC) $(CFLAGS) -o program.o main.c
	./program.o test.txt

clean:
	rm -r *.o
