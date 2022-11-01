CC = gcc
CFLAGS = -Wall

all: static_library shared_library dynamic_library

static_library:
	$(CC) $(CFLAGS) -c array_operations.c -o array_operations_static.o
	$(CC) $(CFLAGS) -c main.c -o main.o
	ar rcs lib_array_operations_static.a array_operations_static.o
	$(CC) $(CFLAGS) -o main_static.o main.o -L. -l_array_operations_static
	./main_static.o

shared_library:
	export DYLD_LIBRARY_PATH="."
	export LD_LIBRARY_PATH="."
	$(CC) $(CFLAGS) -fPIC -c array_operations.c -o array_operations_shared.o
	$(CC) $(CFLAGS) -shared -W1,-soname,lib_array_operations_shared.so.1 -o \
	lib_array_operations_shared.so.1.0.0 array_operations_shared.o -lc
	ln -s lib_array_operations_shared.so.1.0.0 lib_array_operations_shared.so.1
	ln -s lib_array_operations_shared.so.1.0.0 lib_array_operations_shared.so
	$(CC) $(CFLAGS) main.c -o main_shared.o -L. -l_array_operations_shared
	./main_shared.o

dynamic_library:
	$(CC) $(CFLAGS) -c main_dynamic.c
	$(CC) $(CFLAGS) -o main_dynamic_final.o main_dynamic.o -ldl
	./main_dynamic_final.o

clean:
	rm -rf *.o
	rm -rf *.a
	rm -rf *.so
	rm -rf *.0
	rm -rf *.1
