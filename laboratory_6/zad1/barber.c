#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h> //zawiera definicję systemowej struktury danych o nazwie shmid_ds, która tworzona jest podczas tworzenia segmentu pamięci współdzielonej
#include <sys/ipc.h> //funkcje operujące na pamięci współdzielonej
#include <sys/types.h> //funkcje operujące na pamięci współdzielonej
#include <sys/sem.h> //zdefiniowane w nim są operacje na semaforach
#include <time.h>
#include <signal.h>
#include <unistd.h>

int N; //liczba miejsc w poczekalni (nie liczymy fotela do strzyżenia)
int *dataShared; //dataShared[0] - fotel,
                 //dataShared[1:N] - miejsca w poczekalni,
                 //dataShared[N+1] - ilość miejsc w poczekalni
                 //dataShared[N+2] - ilość zajętych miejsc,
                 //dataShared[N+3] - flaga czy barber śpi, czy też nie (0 - obudzony, 1 śpi)
                 //dataShared[N+4] - flaga porozuzmienia z Klientem (otrzymania informacji zwrotnej)

int semaphoreID;
int sharedMemID;

void timerResponse() {
  long timer;
  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);
  timer = t.tv_nsec / 1000; //zamiana nano na mikro
  printf("TIME: %ld ms\n", timer);
}

void signalHandler(int sig) {
  printf("RECEIVED SIGNAL: %d \n--/*ZAMYKANIE SALONU*/--\n", sig);

  if (shmdt(dataShared) == -1) {
    printf("Can't detach shared memory!\n");
  }

  if (shmctl(sharedMemID, IPC_RMID, 0) == -1) {
    printf("Can't remove shared memory!\n");
  }

  if (semctl(semaphoreID, 0, IPC_RMID) == -1) {
    printf("Can't remove set of semaphores\n");
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
  key_t key = ftok(getenv("HOME"), 10); //utworzenie klucza [drugi argument - 8 najmniej znaczących bitów proj_id (które muszą stanowić oktet niezerowy)]
  if (key == -1) {
    printf("Can't create valid key, does the file given by the pathname exists ?\n");
    exit(1);
  }

  semaphoreID = semget(key, 3, 0600 | IPC_CREAT); //utworzenie zbioru 3 semaforów
  if (semaphoreID == -1) {
    printf("Can't create semaphores\n");
    exit(1);
  }

  union semun semaphoresInit;
  semaphoresInit.array = (unsigned short*)malloc(3*sizeof(unsigned short));
  semaphoresInit.array[0] = 0; //semafor dający dostęp do kolejki
  semaphoresInit.array[1] = 0; //semafor dający dostęp do informacji, czy golibroda śpi
  semaphoresInit.array[2] = 0; //semafor dający dos†ep do informacji o strzyżeniu (do fotela)
  semctl(semaphoreID, 0, SETALL, semaphoresInit); //SETALL ustawia wszystkie wartości zgodnie z zawartością unii semun

  sharedMemID = shmget(key, (N+4)*sizeof(int), 0600 | IPC_CREAT); //Utworzenie segmentu pamięci współdzielonej o wielkości (N+4)*(int)
  if (sharedMemID == -1) {
    printf("Can't create shared memory\n");
    exit(1);
  }

  dataShared = (int *)shmat(sharedMemID, NULL, 0); //dołączenia segmentu pamięci wspólnej do przestrzeni adresowej procesu, w celu udostępnienia segmentu pamięci współdzielonej w procesie.
  if (dataShared == (int *)(-1)) {
    printf("Problem with connecting to shared memordy\n");
    exit(1);
  }

  for (int i = 0; i <= (N+4); i++) dataShared[i] = 0; //inicjalizacja danych zmiennej dataShared, która jest pamięcią wspólną
  dataShared[N+1] = N;
}


int checkWaitingRoom(struct sembuf *semaphoresOperations) {
  semaphoresOperations[0].sem_num = 1; //semafor dający dostęp czy golibroda śpi
  semaphoresOperations[0].sem_op = 0; //if sem_op is zero (0), then the calling process will sleep() until the semaphore's value is 0
  semaphoresOperations[1].sem_num = 1;
  semaphoresOperations[1].sem_op = 1; //zwiększenie wartości semafora
  semaphoresOperations[2].sem_num = 0; //semafor dający dostęp do kolejki
  semaphoresOperations[2].sem_op = 0; //if sem_op is zero (0), then the calling process will sleep() until the semaphore's value is 0
  semaphoresOperations[3].sem_num = 0;
  semaphoresOperations[3].sem_op = 1; //zwiększenie wartości semafora
  semop(semaphoreID, semaphoresOperations, 4); //uzyskanie dostępu do danych kolejki i stanu golibrody

  printf("Checking if someone is in waiting room\n");
  if (dataShared[1] != 0) { //jeśli ktoś jest w poczekalni
    printf("I'm inviting client with PID: %d to cut his hair\n", dataShared[1]);
    for (int i = 0; i < N; i++) { //jedno miejsce się zwalnia, więc kolejka przesuwa się o jedno miejsce
      dataShared[i] = dataShared[i + 1];
    }

    dataShared[N] = 0;
    dataShared[N+2]--; //zmniejszenie ilości zajętych miejsc
    timerResponse();
    while (dataShared[N+4] != 1) {} //czekanie na potwierdzenie od klienta, na którego przyszła kolej
    return 1;
  }
  //nikogo nie ma w poczekalni
  return 0;
}


void barberSleeping(struct sembuf *semaphoresOperations) {
  semaphoresOperations[0].sem_num = 0; //semafor dający dostęp do kolejki
  semaphoresOperations[0].sem_op = -1;  //zmniejszenie wartości semafora
  semop(semaphoreID, semaphoresOperations, 1); //zwolnienie dostępu do poczekalni

  printf("I'm going to sleep...\n");
  timerResponse();
  dataShared[N+3] = 1; //ustawienie flagi spania

  semaphoresOperations[0].sem_num = 1; //semafor dający dostęp do informacji czy golibroda śpi
  semaphoresOperations[0].sem_op = -1;  //zmniejszenie wartości semafora
  semop(semaphoreID, semaphoresOperations, 1); //zwolnienie dostępu do stanu golibrody

  while (dataShared[N+3] == 1) {} //zasypia

  printf("Wake up - I'm not sleeping anymore\n");
  timerResponse();
}


void cuttingHair(struct sembuf *semaphoresOperations) {
  semaphoresOperations[0].sem_num = 2; //semafor dający dostęp do informacji o strzyzeniu (do fotela)
  semaphoresOperations[0].sem_op = 0; //if sem_op is zero (0), then the calling process will sleep() until the semaphore's value is 0
  semaphoresOperations[1].sem_num = 2;
  semaphoresOperations[1].sem_op = 1; //zwiększenie wartości semafora
  semop(semaphoreID, semaphoresOperations, 2); //uzyskanie dostępu do informacji o strzyżeniu (do danych fotela dataShared[0])

  printf("I'm going to cut client ID: %d\n", dataShared[0]);
  timerResponse();
  printf("I've cut client ID: %d\n", dataShared[0]);


  semaphoresOperations[0].sem_num = 2;
  semaphoresOperations[0].sem_op = -1;
  dataShared[N+4] = 0; //flaga porozumienia z klientem
  dataShared[0] = 0; //koniec strzyżenia
  timerResponse();
  semop(semaphoreID, semaphoresOperations, 1); //oddanie dostępu do informacji o strzyzeniu

  while (dataShared[N+4] != 1) {} //czekamy na informację zwrotną od klienta
  dataShared[N+4] = 0;
}

void openHairSalon (struct sembuf *semaphoresOperations) {
  while (1) {
    if (checkWaitingRoom(semaphoresOperations) == 1) {
      semaphoresOperations[0].sem_num = 0; //semafor dający dostęp do kolejki
      semaphoresOperations[0].sem_op = -1; //próba zajęcia zasobu
      semaphoresOperations[1].sem_num = 1; //semafor dający dostęp czy golibroda śpi
      semaphoresOperations[1].sem_op = -1;
      semop(semaphoreID, semaphoresOperations, 2); //zwolnienie dostępu do informacji do kolejki i stanu golibrody
      cuttingHair(semaphoresOperations);
    } else {
      barberSleeping(semaphoresOperations);
      cuttingHair(semaphoresOperations);
    }
  }
}


int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Wrong numbe of arguments!\n");
    exit(1);
  }

  signalsInit();
  N = atoi(argv[1]);
  init();

  struct sembuf *semaphoresOperations = (struct sembuf *)malloc(4*sizeof(struct sembuf)); //tablica struktur odpowiedzialnych za wykonywanie operacji na semaforach (są 4, bo jeden jest do sprawdzania wartosci semafora)
  for (int i = 0; i < 4; ++i) {
    semaphoresOperations[i].sem_flg = 0;
  }

  openHairSalon(semaphoresOperations);

  return 0;
}
