CC = gcc
CFLAGS = -Wall
all: task_withoutFork task_withFork

task_withoutFork:
	$(CC) $(CFLAGS) -o task1.o main1a.c

task_withFork:
	$(CC) $(CFLAGS) -o task2.o main1b.c

clean:
	rm *.o
