#include <stdio.h>
#include <stdlib.h>
#include <pthread.h> //biblioteka potrzebna do obsługi wątków
#include <signal.h> //biblioteka do obsługi sygnałów
#include <unistd.h> //biblioteka zawiera funkcję sleep
#include <string.h>  //biblioteka zawiera funkcje do obsługi napisów
#include <sys/types.h>
#include <semaphore.h> //bilbioteka zawiera funkcje dotyczące obsługi semaforów
#include <sys/stat.h>
#include <fcntl.h>

//**ZMIENNE**//
char **bufferArray; //tablica wskaźników do stringów o różnej długości
FILE *fileToRead;

int lastWriteLine = 0; //pozycja ostatnio wypisanej linii
int lastReadLine = 0; //pozycja ostatnio przeczytanej linii
int bufferCounter = 0; //liczba przeczytanych linii
//zmienne lastWriteLine i lastReadLine są umieszczane w buforze zgodnie z zasadą - i = (i+1)%n (cykliczne wstawianie do bufora)

//**PARAMETRY DO KONFIGURACJI**//
int P = 0; //zmienna dotycząca producentów
int K = 0; //zmienna dotycząca konsumentów
int N = 0; //zmienna dotycząca rozmiaru tablicy wskaźników do stringów
char *fileToReadName = NULL; //zmienna dotycząca nazwy pliku
int L = 0; //zmienna zawierająca wartośc, która będzie porównywana z długością pobranego napisu
int raportMode = 0; //zmienna dotycząca trybu wyszukania (w zależności od zmiennej L)
int printMode = 0; //zmienna dotycząca trybu wypisywania informacji (uproszczony lub opisowy)
int nk = 0; //liczba sekund, po której wątki się kończą (działają w pętli nieskończonej)

//semafory
sem_t semBuffer;
sem_t semFullBuffer;
sem_t semEmptyBuffer;

void configureSettings(char *config) {
  FILE * file = fopen(config, "r");
  char* temp;
  size_t size;

  if (file == NULL) {
    printf("Can't read configure file!\n");
    exit(1);
  }

  getline(&temp, &size, file);
  P = atoi(temp);
  if (P < 0) {
    printf("Wrong number of producers\n");
    exit(1);
  }

  getline(&temp, &size, file);
  K = atoi(temp);
  if (K < 0) {
    printf("Wrong number of consumers\n");
    exit(1);
  }

  getline(&temp, &size, file);
  N = atoi(temp);
  if (N < 0) {
    printf("Wrong value of buffer array size\n");
    exit(1);
  }
  bufferArray = malloc(sizeof(char*) * N);
  for (int i = 0; i < N; i++) {
    bufferArray[i] = NULL;
  }

  getline(&fileToReadName, &size, file);
  fileToReadName[strlen(fileToReadName) - 1] = '\0'; //zamiana znaku nowej linii na koniec łańcucha
  fileToRead = fopen(fileToReadName, "r");
  if (fileToReadName == NULL) {
    printf("Can't find file to read\n");
    exit(1);
  }

  getline(&temp, &size, file);
  L = atoi(temp);
  if (L < 0) {
    printf("Wrong value of variable used to compare length of line\n");
    exit(1);
  }

  getline(&temp, &size, file);
  raportMode = atoi(temp);
  if (raportMode > 1 || raportMode < -1) {
    printf("Wrong value of search mode\n");
    exit(1);
  }

  getline(&temp, &size, file);
  printMode = atoi(temp);
  if (printMode > 1 || printMode < 0) {
    printf("Wrong value of print mode\n");
    exit(1);
  }

  getline(&temp, &size, file);
  nk = atoi(temp);
  if (nk < 0) {
    printf("Wrong value of nk\n");
    exit(1);
  }

  fclose(file);
}

void sigHandler(int sig) {
  printf("Received SIGINT -- ENDING PROGRAM\n");
  exit(1);
}

void cleaner() {
  fclose(fileToRead); //zamknięcie pliku
  free(bufferArray); //zwolnienie pamięci ulokowanej
  sem_destroy(&semBuffer);
  sem_destroy(&semEmptyBuffer);
  sem_destroy(&semFullBuffer);
}

void *producentFunction() {
  char* line = NULL; //wiersz który jest aktualnie czytany
  size_t size;

  while(1) {
    //zajęcie zasobu - dekrementacja semaforów
    sem_wait(&semFullBuffer);
    sem_wait(&semBuffer);

    if (getline(&line, &size, fileToRead) <= 0) {//nic nie przeczytano, zwalniamy zasób
      sem_post(&semBuffer); //inkrementacja semafora
      break;
    }

    lastWriteLine = (lastWriteLine + 1) % N;
    bufferArray[lastWriteLine] = malloc(size * sizeof(char));
    strcpy(bufferArray[lastWriteLine], line); //skopiowanie tablicy line do tablicy bufferArray
    bufferCounter++;

    if (raportMode == 1) { //tryb opisowy producenta
      printf("PRODUCENT: Reading line is at %d place in buffer. Line size: %d. Actual size of buffer: %d\n\n",lastWriteLine, (int)strlen(line), bufferCounter);
    }

    //zwolnienie zasobu - inkrementacja semaforów
    sem_post(&semEmptyBuffer);
    sem_post(&semBuffer);
  }
}

void *consumerFunction() {
  char* line;

  while(1) {
    //zajęcie zasobu - dekrementacja semaforów
    sem_wait(&semEmptyBuffer);
    sem_wait(&semBuffer);

    lastReadLine = (lastReadLine + 1) % N;
    line = bufferArray[lastReadLine];
    bufferCounter --;

    printf("\nCONSUMER: Getting line is at %d place in buffer. Actual size of buffer: %d.\n",lastReadLine, bufferCounter);

    if (line[strlen(line) - 1] == '\n') {
      line[strlen(line) - 1] = '\0';
    }

    if (printMode == 0) {
      if (strlen(line) == L) {
        printf("Index of line: %d.\n", lastReadLine);
        printf("Line text: %s\n", line);
      }
    } else if (printMode == 1) {
      if (strlen(line) > L) {
        printf("Index of line: %d.\n", lastReadLine);
        printf("Line text: %s\n", line);
      }
    } else if (printMode == -1) {
      if (strlen(line) < L) {
        printf("Index of line: %d.\n", lastReadLine);
        printf("Line text: %s\n", line);
      }
    }

    free(line);
    //zwolnienie zasobu - inkrementacja semaforów
    sem_post(&semFullBuffer);
    sem_post(&semBuffer);
  }
}

int main(int argc, char*argv[]) {
  atexit(&cleaner); //podana jako parametr funkcja będzie wywołana w chwili, gdy program zakończy działanie.
  signal(SIGINT, sigHandler);

  if (argc != 2) {
    printf("Wrong number of arguments to run the program\n");
    exit(1);
  }

  char* config = argv[1];
  configureSettings(config);


  printf("P:%d K:%d; N:%d File:%s L:%d raportMode:%d printMode:%d nk:%d\n", P, K, N, fileToReadName, L, raportMode, printMode, nk);

  sem_init(&semBuffer, 0, 1);
  sem_init(&semFullBuffer, 0, N);
  sem_init(&semEmptyBuffer, 0, 0);

  pthread_t producents[P];
  pthread_t consumers[K];

  for (int i = 0; i < P; i++) {
    pthread_create(&producents[i], NULL, &producentFunction, NULL); //1.NULL - domyślne argumenty, 2.NULL - brak wartości z którą ma być wywołana funkcja w wątku
  }

  for (int i = 0; i < K; i++) {
    pthread_create(&consumers[i], NULL, &consumerFunction, NULL);
  }

  for(int i = 0; i < P; i++) {
    pthread_join(producents[i], NULL); //gdy wskazany jako parametr wątek nie zakończył się jeszcze, wątek bieżący jest wstrzymywany (kod powrotu == NULL)
  }

  if (nk > 0) {
    sleep(nk);
    printf("\nSuccess ending threads - after %d seconds\n", nk);
    _exit(0);
  }

  while(1) { //synchronizacja - sekcja krytyczna, sprawdzenie ilości przeczytanych linii
    sem_wait(&semBuffer); //dekrementacja semafora - zajmowanie zasobu
    if (bufferCounter == 0) break;
    sem_post(&semBuffer); //inkrementacja semafora - oddanie zasobu
  }


  return 0;
}
