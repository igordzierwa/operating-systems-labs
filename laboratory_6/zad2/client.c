#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <semaphore.h> //biblioteka do obsługi semaforów POSIX
#include <sys/mman.h> //biblioteka do obsługi pamięci współdzielonej
#include <sys/stat.h> //dodatkowe biblioteki do obsługi POSIX
#include <fcntl.h>

int N; //liczba miejsc w poczekalni (nie liczymy fotela do strzyżenia)
int *dataShared; //dataShared[0] - fotel,
                 //dataShared[1:N] - miejsca w poczekalni,
                 //dataShared[N+1] - ilość miejsc w poczekalni
                 //dataShared[N+2] - ilość zajętych miejsc,
                 //dataShared[N+3] - flaga czy barber śpi, czy też nie (0 - obudzony, 1 śpi)
                 //dataShared[N+4] - flaga porozuzmienia z Klientem (otrzymania informacji zwrotnej)

int fd; //deskryptor pamięci współdzielonej
sem_t* semaphore1; //semafor dający dostęp do kolejki
sem_t* semaphore2; //semafor dający dostęp do informacji, czy golibroda śpi
sem_t* semaphore3; //semafor dający dos†ep do informacji o strzyżeniu (do fotela)


void timerResponse() {
  long timer;
  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);
  timer = t.tv_nsec / 1000; //zamiana nano na mikro
  printf("TIME: %ld ms\n", timer);
}


void init() {
  //utworzenie zbioru 3 semaforów
  semaphore1 = sem_open("/semaphore1", O_RDWR); //dostęp do kolejki
  semaphore2 = sem_open("/semaphore2", O_RDWR); //dostęp do stanu golibrody
  semaphore3 = sem_open("/semaphore3", O_RDWR); //dostęp do fotela
  if (semaphore1 == SEM_FAILED || semaphore2 == SEM_FAILED || semaphore3 == SEM_FAILED) {
    printf("Can't access to semaphores\n");
    exit(1);
  }

  fd = shm_open("/sharedmemory", O_RDWR, 0);
  if (fd == -1) {
    printf("Can't access to shared memory\n");
    exit(1);
  }

  dataShared = (int*) mmap(NULL, (N+4)*sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); //NULL - jądro systemu dobiera samo odpowiedni adres
  if (dataShared == (int*)(-1)) {
    printf("Can't connect with shared data\n");
    exit(1);
  }

  for (int i = 0 ; ;i++) {
    if (dataShared[i] != 0) {
      N = dataShared[i];
      break;
    }
  }
}


void exitSalon() {
  printf("I'm going out of salon - ID: %d\n", getpid());
  timerResponse();
}


int checkWaitingRoom() {
  sem_post(semaphore2); //zwolnienie dostępu do stanu golibrody

  if (dataShared[N+2] < dataShared[N+1]) { //jest miejsce w poczekalni
    pid_t clientPID = getpid();
    dataShared[dataShared[N+2] + 1] = clientPID; //zajęcie ostatniego wolnego miejsca;
    dataShared[N+2]++;
    printf("I've taken the seat nr: %d in waiting room - ID: %d\n", dataShared[N+2], clientPID);
    timerResponse();

    sem_post(semaphore1); //zwolnienie dostępu do kolejki

    while (dataShared[0] != clientPID) {} //klient czeka na swoją kolej
    dataShared[N+4] = 1; //porozumenie z golibrodą
    return 0;
  } else { //brak miejsca w poczekalni
    sem_post(semaphore1); //zwolnienie dostępu do kolejki
    printf("No free seats in waiting room\n");
    exitSalon();
    return -1;
  }
}


int checkSleepingBarber() {
  sem_wait(semaphore2); //uzyskanie dostępu do stanu golibrody
  sem_wait(semaphore1); //uzyskanie dostępu do kolejki

  printf("I'm checking if barber is sleeping - ID: %d\n", getpid());
  timerResponse();

  return dataShared[N+3];
}


void wakeupSleeepingBarber() {
  sem_wait(semaphore3); //uzyskanie dostępu do fotela
  printf("I'm waking up barber - ID: %d\n", getpid());

  dataShared[N+3] = 0; //flaga stanu na obudzony
  timerResponse();

  sem_post(semaphore2); //oddanie dostępu do barbera
  sem_post(semaphore1); //oddanie dostępu do kolejki
}

void haircut(int flagQueue) {
  pid_t clientPID = getpid();
  printf("I'm sitting on chair for hair cutting - ID %d\n", clientPID);

  if (flagQueue == 0) { //nie ma nikogo
    dataShared[0] = clientPID;
    timerResponse();
    sem_post(semaphore3); //oddanie dostępu do fotela
  } else {
    timerResponse();
  }

  while(dataShared[0] != 0) {} //oczekiwanie na ostrzyżenie
  dataShared[N+4] = 1; //wysłanie potwierdzenia do golibrody
  printf("My haircut is done - ID: %d\n", clientPID);
  timerResponse();
}


void enterSalon() {
  if (checkSleepingBarber() == 1) { //jesli barber spi, czyli kolejka jest pusta
    wakeupSleeepingBarber();
    haircut(0);
    exitSalon();
  } else {
    if (checkWaitingRoom() == 0) {
      haircut(1); //poczeka na swoją kolej - petla while
      exitSalon();
    }
  }
}


int main(int argc, char* argv[]) {
  if (argc != 3) {
    printf("Wrong number of arguments\n");
    exit(1);
  }

  int numberOfClients = atoi(argv[1]); //liczba klientów do stworzenia (procesy potomne)
  int numberOfCuts = atoi(argv[2]); //liczba strzyżeń każdego klienta (liczba pętli)


  init();

  for (int i = 0; i < numberOfClients; i++) {
    pid_t childPID = fork();
    if (childPID == 0) {
      for (int j = 0; j < numberOfCuts; j++) {
        enterSalon();
      }
      if (munmap(dataShared, (N+4)*sizeof(int)) == -1) {
        printf("Can't detach shared memory\n");
      }
      exit(0);
    }
    continue;
  }

  for (int i = 0; i < numberOfClients; i++) { //oczekiwanie aż wszystkie procesy się zakończą
    wait(NULL);
  }

  if (munmap(dataShared, (N+4)*sizeof(int)) == -1) {
    printf("Can't detach shared memory\n");
  }

  return 0;
}
