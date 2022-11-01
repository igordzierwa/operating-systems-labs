CC = gcc
CFLAGS = -Wall

all: clientProgramPosix serverProgramPosix

clientProgramPosix:
	$(CC) $(FLAGS) -o client.o clientPos.c -lrt

serverProgramPosix:
	$(CC) $(FLAGS) -o server.o serverPos.c -lrt

clean:
	rm *.o
