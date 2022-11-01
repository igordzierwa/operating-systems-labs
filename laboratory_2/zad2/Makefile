CC = gcc
CFLAGS = -Wall

all: program test1StandardFunction test2NftwFunction clean

program:
	$(CC) $(CFLAGS) -o program.o main.c

test1StandardFunction:
	./program.o /Users/igordzierwa/Desktop/DzierwaIgor/cw02/zad2/example ">" "29 09 2019" standard
	./program.o example ">" "29 09 2019" standard

test2NftwFunction:
	./program.o /Users/igordzierwa/Desktop/DzierwaIgor/cw02/zad2/example ">" "29 09 2019" nftw
	./program.o example ">" "29 09 2019" nftw

clean:
	rm -f *.o
