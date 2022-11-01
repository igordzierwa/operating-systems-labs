CC = gcc
CFLAGS = -Wall

all: program

program:
	$(CC) $(FLAGS) -o program.o main.c -pthread
	./program.o config.txt

clean:
	rm *.o
