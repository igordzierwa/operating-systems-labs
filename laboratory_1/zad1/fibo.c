#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define BILLION 1000000000.0

int main () {
  struct timespec start, end;
  double final_time;

  int first = 0;
  int second = 1;

  int i,j,next;
  volatile long n = 1000000000; //zmienna ulotna - Dla zmiennej typu volatile kompilator wyłącza optymalizację

  clock_gettime(CLOCK_REALTIME, &start);
  for (i = 0; i < 10; i++) {
    for (j = 0; j < n; j++) {
      if (j <= 1) {
        next = j;
      } else {
        next = first + second;
        first = second;
        second = next;
      }
    }
  }
  clock_gettime(CLOCK_REALTIME, &end);

  final_time = (end.tv_sec - start.tv_sec) +
               (end.tv_nsec - start.tv_nsec) / BILLION;

  printf("%f s\n",final_time);

  return 0;
}
