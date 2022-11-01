CC = gcc
CFLAGS = -Wall

all: test_loopfunction test_bubble test_fibo test_recc

bubble: bubble.c
	$(CC) $(CFLAGS) bubble.c -o bubble.o

fibo: fibo.c
	$(CC) $(CFLAGS) fibo.c -o fibo_test.o

loopfunction: loopfunction.c
	$(CC) $(CFLAGS) loopfunction.c -o loopfunction.o

recc: recc.c
	$(CC) $(CFLAGS) recc.c -o recc.o

test_loopfunction:
	$(CC) $(CFLAGS) loopfunction.c -o loopfunction.o
	./loopfunction.o
	$(CC) $(CFLAGS) loopfunction.c -o loopfunction_o.o -O
	./loopfunction_o.o
	$(CC) $(CFLAGS) loopfunction.c -o loopfunction_o1.o -O1
	./loopfunction_o1.o
	$(CC) $(CFLAGS) loopfunction.c -o loopfunction_o2.o -O2
	./loopfunction_o2.o
	$(CC) $(CFLAGS) loopfunction.c -o loopfunction_o3.o -O3
	./loopfunction_o3.o

test_bubble:
	$(CC) $(CFLAGS) bubble.c -o bubble_test.o
	./bubble_test.o
	$(CC) $(CFLAGS) bubble.c -o bubble_test_o.o -O
	./bubble_test_o.o
	$(CC) $(CFLAGS) bubble.c -o bubble_test_o1.o -O1
	./bubble_test_o1.o
	$(CC) $(CFLAGS) bubble.c -o bubble_test_o2.o -O2
	./bubble_test_o2.o
	$(CC) $(CFLAGS) bubble.c -o bubble_test_o3.o -O3
	./bubble_test_o3.o

test_fibo:
	$(CC) $(CFLAGS) fibo.c -o fibo_test.o
	./fibo_test.o
	$(CC) $(CFLAGS) fibo.c -o fibo_test_o.o -O
	./fibo_test_o.o
	$(CC) $(CFLAGS) fibo.c -o fibo_test_o1.o -O1
	./fibo_test_o1.o
	$(CC) $(CFLAGS) fibo.c -o fibo_test_o2.o -O2
	./fibo_test_o2.o
	$(CC) $(CFLAGS) fibo.c -o fibo_test_o3.o -O3
	./fibo_test_o3.o

test_recc:
	$(CC) $(CFLAGS) recc.c -o recc.o
	./recc.o
	$(CC) $(CFLAGS) recc.c -o recc_o.o -O
	./recc_o.o
	$(CC) $(CFLAGS) recc.c -o recc_o1.o -O1
	./recc_o1.o
	$(CC) $(CFLAGS) recc.c -o recc_o2.o -O2
	./recc_o2.o
	$(CC) $(CFLAGS) recc.c -o recc_o3.o -O3
	./recc_o3.o

clean:
	-rm -rf *.o
