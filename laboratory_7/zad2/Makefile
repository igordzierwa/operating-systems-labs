CC = gcc
CFLAGS = -Wall

all: program

program:
	$(CC) $(FLAGS) -o main.o main.c -pthread
	./main.o config.txt

clean:
	rm *.o
