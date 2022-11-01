#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define BILLION 1000000000.0

double
powern (double d, unsigned n)
{
  double x = 1.0;
  unsigned j;

  for (j = 1; j <= n; j++)
    x *= d;

  return x;
}

int main() {
  struct timespec start, end;
  double final_time;

  double sum = 0.0;
  volatile int i;

  clock_gettime(CLOCK_REALTIME, &start);
  for (i = 1; i <= 2000000000; i++)
  {
    sum += powern (i, i % 5);
  }
  clock_gettime(CLOCK_REALTIME, &end);

  final_time = (end.tv_sec - start.tv_sec) +
               (end.tv_nsec - start.tv_nsec) / BILLION;

  printf("%f s\n",final_time);

  return 0;
}
