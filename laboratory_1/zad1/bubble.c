#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define BILLION 1000000000.0

void bubbleSort(int arr[], int n)
{
   int i, j;
   for (i = 0; i < n-1; i++)
       for (j = 0; j < n-i-1; j++)
           if (arr[j] > arr[j+1]) {
             int temp = arr[i];
             arr[i] = arr[i+1];
             arr[i+1] = temp;
           }
}

void printArray(int arr[], int size)
{
    int i;
    for (i=0; i < size; i++)
        printf("%d ", arr[i]);
    printf("\n");
}

int main () {
  struct timespec start, end;
  double final_time;

  int arr[100000];
  //wybrana najmniej korzystna sytuacja, gdy dane posortowane są w ciągu malejącym
  for(int i = 0; i<100000; i++) {
    arr[i] = 100000-i;
  }

  clock_gettime(CLOCK_REALTIME, &start);
  bubbleSort(arr, 100000);
  clock_gettime(CLOCK_REALTIME, &end);

  final_time = (end.tv_sec - start.tv_sec) +
               (end.tv_nsec - start.tv_nsec) / BILLION;

  printf("%f s\n",final_time);

  return 0;
}
