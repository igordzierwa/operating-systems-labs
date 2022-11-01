CC = gcc
CFLAGS = -Wall

all: program test1 clean

program:
	$(CC) $(CFLAGS) -o program.o main.c

test1:
	./program.o /Users/igordzierwa/Desktop/DzierwaIgor/cw03/zad1/example ">" "29 09 2019"
	./program.o example ">" "29 09 2019"

clean:
	rm -f *.o
