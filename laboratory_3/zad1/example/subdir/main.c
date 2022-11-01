#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "array_operations.h"

#define BILLION 1000000000.0

int main () {
  struct timespec start, end;
  double final_time;

  clock_gettime(CLOCK_REALTIME, &start);

  hello_pl();

  int array[100000];
  for(int i = 0; i < 100000; i++) {
    array[i] = 100000 - i;
  }

  int array_size = sizeof (array)/sizeof(int);
  bubbleSort(array, array_size);

  clock_gettime(CLOCK_REALTIME, &end);

  final_time = (end.tv_sec - start.tv_sec) +
               (end.tv_nsec - start.tv_nsec) / BILLION;

  printf("%f s\n",final_time);

  return 0;
}
