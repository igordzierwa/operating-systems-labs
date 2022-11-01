#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <dlfcn.h>
#include "array_operations.h"

#define BILLION 1000000000.0

int main () {
  struct timespec start, end;
  double final_time;

  void  *library;
  const char *blad;
  void (*functioHello)(void);
  void (*functionBubble)(int*, int);

  clock_gettime(CLOCK_REALTIME, &start);

  library = dlopen("lib_array_operations_shared.so", RTLD_LAZY);
  blad = dlerror();
  printf("\n    Otwarcie biblioteki \"lib_array_operations_shared\", rezultat -%s- \n", blad);
  if(blad) return(1);

  dlerror();
  functioHello = dlsym(library, "hello_pl");
  blad = dlerror();
  if(blad) return(1);
  (*functioHello)();

  int array[100000];
  for(int i = 0; i < 100000; i++) {
    array[i] = 100000 - i;
  }

  int array_size = sizeof (array)/sizeof(int);
  functionBubble = dlsym(library, "bubbleSort");
  blad = dlerror();
  if(blad) return(1);
  (*functionBubble)(array, array_size);

  dlclose(library);

  clock_gettime(CLOCK_REALTIME, &end);

  final_time = (end.tv_sec - start.tv_sec) +
               (end.tv_nsec - start.tv_nsec) / BILLION;

  printf("%f s\n",final_time);

  return 0;
}
