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

//sprawdzić czy sem_unlink trzeba użyć, czy wyjebie program
void signalHandler(int sig) {
  printf("RECEIVED SIGNAL: %d \n--/*ZAMYKANIE SALONU*/--\n", sig);

  if (sem_close(semaphore1) == -1 || sem_close(semaphore2) == -1 || sem_close(semaphore3) == -1) {
    printf("Can't close semaphores\n");
  }

  if (munmap(dataShared, (N+4)*sizeof(int)) == -1) {
    printf("Can't detach shared memory\n");
  }

  if (shm_unlink("/sharedmemory") == -1) {
    printf("Can't remove shared memory!\n");
  }

  exit(0);
}


void signalsInit() {
  sigset_t setSignals;
  sigfillset(&setSignals);
  sigdelset(&setSignals, SIGTERM);
  sigdelset(&setSignals, SIGINT);
  sigprocmask(SIG_SETMASK, &setSignals, NULL);
  signal(SIGTERM, signalHandler);
  signal(SIGINT, signalHandler);
}

void init() {
  //utworzenie zbioru 3 semaforów
  semaphore1 = sem_open("/semaphore1", O_RDWR | O_CREAT, 0600, 1); //dostęp do kolejki
  semaphore2 = sem_open("/semaphore2", O_RDWR | O_CREAT, 0600, 1); //dostęp do stanu golibrody
  semaphore3 = sem_open("/semaphore3", O_RDWR | O_CREAT, 0600, 1); //dostęp do fotela
  if (semaphore1 == SEM_FAILED || semaphore2 == SEM_FAILED || semaphore3 == SEM_FAILED) {
    printf("Can;t create semaphores\n");
    exit(1);
  }

  fd = shm_open("/sharedmemory", O_RDWR | O_CREAT, 0600);
  if (fd == -1) {
    printf("Can't create shared memory\n");
    exit(1);
  }

  if (ftruncate(fd, (N+4)*sizeof(int)) == -1) {
    printf("Can't allocate space for shared memory\n");
    exit(1);
  }

  dataShared = (int*) mmap(NULL, (N+4)*sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); //NULL - jądro systemu dobiera samo odpowiedni adres
  if (dataShared == (int*)(-1)) {
    printf("Can't connect with shared data\n");
    exit(1);
  }

  for (int i = 0; i <= (N+4); i++) {
    dataShared[i] = 0;
  }
  dataShared[N+1] = N;
}

int checkWaitingRoom() {
  //dekrementacja semaforów - zajmowanie
  sem_wait(semaphore2); //uzyskanie dostępu do stanu golibrody
  sem_wait(semaphore1); //uzyskanie dostępu do kolejki

  printf("Checking if someone is in waiting room\n");
  if (dataShared[1] != 0) {
    printf("I'm inviting client with PID: %d to cut his hair\n", dataShared[1]);
    for (int i = 0; i < N; i++) {
      dataShared[i] = dataShared[i+1];
    }
    dataShared[N] = 0;
    dataShared[N+2]-- ; //zmniejszenie ilości zajętych miejsca
    timerResponse();
    while (dataShared[N+4] != 1) {} //oczekiwanie na potwierdzenie od klienta, na którego przyszła kolej
    return 1;
  }
  //nikogo nie main
  return 0;
}


void barberSleeping() {
  //inkrementacja semafora - zwolnienie dostępu
  sem_post(semaphore1); //zwolnienie dostęp do kolejki
  printf("I'm going to sleep...\n");
  timerResponse();

  dataShared[N+3] = 1; //ustawienie flagi spania
  sem_post(semaphore2); //zwolnienie dostępu do stanu golibrody

  while (dataShared[N+3] == 1) {} //zasypia

  printf("Wake up - I'm not sleeping anymore\n");
  timerResponse();
}


void cuttingHair() {
  sem_wait(semaphore3); //uzyskanie dostępu do fotela

  printf("I'm going to cut client ID: %d\n", dataShared[0]);
  timerResponse();
  printf("I've cut client ID: %d\n", dataShared[0]);

  dataShared[N+4] = 0;
  dataShared[0] = 0; //koniec strzyżenia
  timerResponse();
  sem_post(semaphore3); //zwolnienie dostępu do fotela

  while (dataShared[N+4] != 1) {} //czekamy na informację zwrotną od klienta
  dataShared[N+4] = 0;
}


void openHairSalon() {
  while (1) {
    if (checkWaitingRoom() == 1) {
      sem_post(semaphore1); //zwolnienie dostępu do kolejki
      sem_post(semaphore2); //zwolnienei dostępu do stanu golibrody
      cuttingHair();
    } else {
      barberSleeping();
      cuttingHair();
    }
  }
}


int main(int argc, char* argv[]) {
  if (argc != 2) {
    printf("Wrong number of arguments!\n");
    exit(1);
  }

  signalsInit();

  N = atoi(argv[1]);

  init();

  openHairSalon();

  return 0;
}
