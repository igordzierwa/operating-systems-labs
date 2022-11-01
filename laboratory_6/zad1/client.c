#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h> //zawiera definicję systemowej struktury danych o nazwie shmid_ds, która tworzona jest podczas tworzenia segmentu pamięci współdzielonej
#include <sys/ipc.h> //funkcje operujące na pamięci współdzielonej
#include <sys/types.h> //funkcje operujące na pamięci współdzielonej
#include <sys/sem.h> //zdefiniowane w nim są operacje na semaforach
#include <sys/wait.h> //do funkcji wait
#include <time.h>
#include <signal.h>
#include <unistd.h>

int N; //liczba miejsc w poczekalni (nie liczymy fotela do strzyżenia)
int *dataShared; //dataShared[0] - fotel,
                 //dataShared[1:N] - miejsca w poczekalni,
                 //dataShared[N+1] - ilość miejsc w poczekalni
                 //dataShared[N+2] - ilość zajętych miejsc,
                 //dataShared[N+3] - flaga czy barber śpi, czy też nie
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


void init() {
  key_t key = ftok(getenv("HOME"), 10);
  if (key == -1) {
    printf("Can't create valid key, does the file given by the pathname exists ?\n");
    exit(1);
  }

  semaphoreID = semget(key, 0, 0); //nsems = 0 - pobranie identyfikatora istniejącego już zbioru semaforów
  if (semaphoreID == -1) {
    printf("Can't create semaphores\n");
    exit(1);
  }

  sharedMemID = shmget(key, 0, 0); // by uzyskać identyfikator istniejącego już segmentu pamięci wspólnej — argument size powinien mieć wartość 0
  if (sharedMemID == -1) {
    printf("Can't create shared memory\n");
    exit(1);
  }

  dataShared = (int *)shmat(sharedMemID, NULL, 0); //dołączenia segmentu pamięci wspólnej do przestrzeni adresowej procesu, w celu udostępnienia segmentu pamięci współdzielonej w procesie.
  if (dataShared == (int *)(-1)) {
    printf("Problem with connecting to shared memordy\n");
    exit(1);
  }

  for (int i = 0;; i++) {
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


int checkWaitingRoom(struct sembuf *semaphoresOperations) {
  semaphoresOperations[0].sem_num = 1; //semafor dający dostęp do informacji, czy golibroda śpi
  semaphoresOperations[0].sem_op = -1; //zmniejszenie wartości semafora
  semop(semaphoreID, semaphoresOperations, 1); //zwolnienie dostępu do stanu golibrody

  if (dataShared[N+2] < dataShared[N+1]) { //jest miejsce w poczekalni
    pid_t clientPID = getpid();
    dataShared[dataShared[N+2] + 1] = clientPID; //zajęcie ostatniego wolnego miejsca
    dataShared[N+2]++;
    printf("I've taken the seat nr: %d in waiting room - ID: %d\n", dataShared[N+2], clientPID);
    timerResponse();

    semaphoresOperations[0].sem_num = 0; //semafor dający dostęp do kolejki
    semaphoresOperations[0].sem_op = -1; //zmiejszenie wartości semafora
    semop(semaphoreID, semaphoresOperations, 1); //zwolnienie dostępu do kolejki

    while (dataShared[0] != clientPID) {} //klient czeka na swoją kolej
    dataShared[N+4] = 1; //porozumienie z golibrodą
    return 0;
  } else { //brak miejsca w poczekalni
    semaphoresOperations[0].sem_num = 0;
    semaphoresOperations[0].sem_op = -1;
    semop(semaphoreID, semaphoresOperations, 1);
    printf("No free seats in waiting room\n");
    exitSalon();
    return -1;
  }
}


int checkSleepingBarber(struct sembuf *semaphoresOperations) {
  semaphoresOperations[0].sem_num = 1; //semafor dający dostęp do informacji, czy golibroda śpi
  semaphoresOperations[0].sem_op = 0; //if sem_op is zero (0), then the calling process will sleep() until the semaphore's value is 0
  semaphoresOperations[1].sem_num = 1;
  semaphoresOperations[1].sem_op = 1; //zwiększenie wartości semafora (zwolnienie zasobu)
  semaphoresOperations[2].sem_num = 0; //if sem_op is zero (0), then the calling process will sleep() until the semaphore's value is 0
  semaphoresOperations[2].sem_op = 0;
  semaphoresOperations[3].sem_num = 0;
  semaphoresOperations[3].sem_op = 1;
  semop(semaphoreID, semaphoresOperations, 4); //uzyskanie dostępu do stanu golibrody i poczekalni

  printf("I'm checking if barber is sleeping - ID: %d\n", getpid());
  timerResponse();

  return dataShared[N+3];
}

void wakeupSleeepingBarber(struct sembuf *semaphoresOperations) {
  semaphoresOperations[0].sem_num = 2; //semafor dający dos†ep do informacji o strzyżeniu (do fotela)
  semaphoresOperations[0].sem_op = 0; //if sem_op is zero (0), then the calling process will sleep() until the semaphore's value is 0
  semaphoresOperations[1].sem_num = 2;
  semaphoresOperations[1].sem_op = 1;
  semop(semaphoreID, semaphoresOperations, 2); //uzyskanie dostępu do informacji o strzyżeniu (do fotela)

  printf("I'm waking up barber - ID: %d\n", getpid());

  dataShared[N+3] = 0; //flaga stanu na obudzony
  timerResponse();

  semaphoresOperations[0].sem_num = 0; //semafor dający dostęp do kolejki
  semaphoresOperations[0].sem_op = -1;
  semaphoresOperations[1].sem_num = 1; //semafor dający dostęp do informacji, czy golibroda śpi
  semaphoresOperations[1].sem_op = -1;
  semop(semaphoreID, semaphoresOperations, 2); //oddanie dostępu do poczekalni i stanu golibrody
}


void haircut(struct sembuf *semaphoresOperations, int flagQueue) {
  pid_t clientPID = getpid();
  printf("I'm sitting on chair for hair cutting - ID %d\n", clientPID);

  if (flagQueue == 0) { //nie ma nikogo
    dataShared[0] = clientPID;
    semaphoresOperations[0].sem_num = 2;
    semaphoresOperations[0].sem_op = -1;
    timerResponse();
    semop(semaphoreID, semaphoresOperations, 1); //zwolnienie dostępu do informacji o strzyżeniu (do fotela)
  } else {
    timerResponse();
  }

  while(dataShared[0] != 0) {} //oczekiwanie na ostrzyzenie
  dataShared[N+4] = 1; //wysłanie potwierdzenia do golibrody
  printf("My haircut is done - ID: %d\n", clientPID);
  timerResponse();
}


void enterSalon(struct sembuf *semaphoresOperations) {
  if (checkSleepingBarber(semaphoresOperations) == 1) { //jesli barber spi, czyli kolejka jest pusta
    wakeupSleeepingBarber(semaphoresOperations);
    haircut(semaphoresOperations, 0);
    exitSalon();
  } else {
    if (checkWaitingRoom(semaphoresOperations) == 0) {
      haircut(semaphoresOperations, 1); //poczeka na swoją kolej - pętla while
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

  struct sembuf *semaphoresOperations = (struct sembuf *)malloc(4*sizeof(struct sembuf)); //tablica struktur odpowiedzialnych za wykonywanie operacji na semaforach (są 4, bo jeden jest do sprawdzania wartosci semafora)
  for (int i = 0; i < 4; ++i) {
    semaphoresOperations[i].sem_flg = 0;
  }

  for (int i = 0; i < numberOfClients; i++) {
    pid_t childPID = fork();
    if (childPID == 0) {
      for (int j = 0; j < numberOfCuts; j++) {
        enterSalon(semaphoresOperations);
      }
      if (shmdt(dataShared) == -1) {
        printf("Can't detach shared memory\n");
      }
      exit(0);
    }
    continue;
  }

  for (int i = 0; i < numberOfClients; i++) { //oczekiwanie aż wszystkie procesy się zakończą
    wait(NULL);
  }

  if (shmdt(dataShared) == -1) {
    printf("Can't detach shared memory\n");
  }

  return 0;
}
