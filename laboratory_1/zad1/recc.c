#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define BILLION 1000000000.0

int fibo(volatile int n) {
  if (n == 0) {
       return 0;
   }
   if (n == 1) {
       return n;
   }

   return fibo(n - 2) + fibo(n - 1);
}

int main () {
  struct timespec start, end;
  double final_time;

  int score;

  clock_gettime(CLOCK_REALTIME, &start);
  score = fibo(46);
  clock_gettime(CLOCK_REALTIME, &end);

  final_time = (end.tv_sec - start.tv_sec) +
               (end.tv_nsec - start.tv_nsec) / BILLION;

  printf("%f s\n",final_time);

  return 0;
}
