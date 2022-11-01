#include <stdio.h>
#include <stdlib.h>

void hello_pl(void) {
   printf("Witaj w bibliotece zawierajacej proste operacje na tablicy!\n");
   printf("\n");
}

int checkingArraySorted(int arr[], int n)
{
    if (n == 0 || n == 1) {
      return 1;
    }

    for (int i = 1; i < n; i++) {
      if (arr[i - 1] > arr[i])
          return 0;
    }

    return 1;
}

void swap(int *xp, int *yp) {
    int temp = *xp;
    *xp = *yp;
    *yp = temp;
}

void bubbleSort(int arr[], int n) {
  int sorted = checkingArraySorted(arr, n);
  if (sorted) {
    printf("Nie przystępujemy do sortowania tablicy, tablica jest posortowana\n");
    return;
  } else {
    printf("Tablica nie jest posortowana - sortujemy babelkowo:\n");
    int i, j;
    for (i = 0; i < n-1; i++) {
      for (j = 0; j < n-i-1; j++)
      if (arr[j] > arr[j+1]) {
        swap(&arr[j], &arr[j+1]);
      }
    }
  }
}

void printArray(int arr[], int size) {
  int i;
  printf("Wypisanie elementow tablicy:\n");
  for (i=0; i < size; i++)
      printf("%d ", arr[i]);
  printf("\n");
}
