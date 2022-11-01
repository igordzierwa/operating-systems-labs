//RACZEJ TO KURWA NIE DZIAŁA

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

int TRIGGERSignal; //sygnał wysyłany z procesu macierzystego do potomka
int RESPONSESignal; //sygnał wysłany z potomka do procesu macierzystego
int ENDINGSignal; //sygnał kończący

int childPID;
int Type; //sposób wysyłania sygnałów (trzy różne sposoby)
int l; //liczba sygnałów
int flag;

int counterTriggerSignals; //liczba wysłanych sygnałów do potomka
int receivedChildSignals; //liczba sygnałów odebranych przez potomka
int counterResponseSignals; //liczba odebranych sygnałów przez rodzica wysłanych od potomka

//funkcja będąca reakcją procesu potomnego na otrzymany sygnał
void childTriggerReaction(int sig) {
  receivedChildSignals++;
  kill(getppid(), RESPONSESignal);
}

//funkcja będąca reakcją procesu potomnego na otrzymany sygnał kończący
void childEndingReaction(int sig) {
  printf("Child process received: %d signals from parent process\n", receivedChildSignals);
  exit(1);
}

//funkcja będąca reakcją procesu macierzystego na odebranie sygnału od potomka
void parentResponseReaction(int sig) {
  flag = 1;
  counterResponseSignals ++;
}

//funkcja będąca reakcją procesu macierzystego na wysłanie sygnału kończącego
void parentEndingReaction(int sig) {
  printf("Received SIGINT signal\n");

  kill(childPID, ENDINGSignal);

  printf("Parent process sent: %d trigger signals to child\n", counterTriggerSignals);
  printf("Parent process received: %d response signals from child\n", counterResponseSignals);
  exit(1);
}

//Funkcja obsługi sygnałów przez proces rodzica
void childProcessHandlers() {
  //obsługa sygnału wysłanego przez rodzica
  struct sigaction triggerAction;
  triggerAction.sa_handler = childTriggerReaction;
  sigfillset(&triggerAction.sa_mask); //ustawienie maski na każdy sygnał (wszystkie pozostałe sygnały muszą być blokowane)
  if (sigaction(TRIGGERSignal, &triggerAction, NULL) == -1) {
    printf("Can't set trigger reaction in child process\n");
    exit(1);
  }

  //obsluga sygnału kończącego
  struct sigaction endingAction;
  endingAction.sa_handler = childEndingReaction;
  sigfillset(&endingAction.sa_mask);
  sigdelset(&endingAction.sa_mask, ENDINGSignal);
  if (sigaction(ENDINGSignal, &endingAction, NULL) == -1) {
    printf("Can't set ending reaction in child proces\n");
    exit(1);
  }
}

//Funkcja obsługi syngałów przez proces potomka
void parentProcessHandlers() {
  //obsługa sygnału odebranego od potomka
  struct sigaction responseAction;
  responseAction.sa_handler = parentResponseReaction;
  sigfillset(&responseAction.sa_mask);
  if (sigaction(RESPONSESignal, &responseAction, NULL) == -1) {
    printf("Can't set response reaction in parent process\n");
    exit(1);
  }

  //obsluga sygnału kończącego SIGINT
  struct sigaction endingAction;
  endingAction.sa_handler = parentEndingReaction;
  sigfillset(&endingAction.sa_mask);
  sigdelset(&endingAction.sa_mask, SIGINT);
  if (sigaction(SIGINT, &endingAction, NULL) == -1) {
    printf("Can't set ending reaction in parent process\n");
    exit(1);
  }
}

//funkcja, która jest wykonywana przez każdy proces potomny
void childFunction() {
  receivedChildSignals = 0;
  childProcessHandlers();

  //Odblokowanie sygnału TRIGGERSignal oraz ENDINGSignal (reszta dalej zablokowana)
  sigset_t childMask;
  sigfillset(&childMask);
  sigdelset(&childMask, TRIGGERSignal);
  sigdelset(&childMask, ENDINGSignal);

  //tymczasowe zastąpienie bieżącej maski przez childMask
  while(1) {
    sigsuspend(&childMask);
  }
}

//stworzenie potomka oraz zapisanie jego ID
void createChild() {
  sigset_t childMask;
  sigset_t parentMask;

  //ustawienie maski sygnałów początkowo blokującej wszystkie sygnały
  sigfillset(&childMask);
  if (sigprocmask(SIG_SETMASK, &childMask, &parentMask) == -1) {
    printf("Can't set sigmask for child process\n");
    exit(1);
  }

  int id = fork();

  if (id == 0) {
    childFunction();
  }

  childPID = id;
  if (sigprocmask(SIG_SETMASK, &parentMask, NULL) == -1) {
    printf("Can't set sigmask for parent process\n");
    exit(1);
  }
}

int main(int argc, char** argv) {
  if(argc < 3) {
    printf("Wrong number of arguments\n");
    exit(1);
  }

  //Funkcja jako argument pobiera liczbę w postaci ciągu znaków ASCII, a następnie zwraca jej wartość w formacie int.
  l = atoi(argv[1]);
  Type = atoi(argv[2]);

  counterTriggerSignals = 0;
  counterResponseSignals = 0;

  if (Type == 1)
  {
    TRIGGERSignal = SIGUSR1;
    RESPONSESignal = SIGUSR1;
    ENDINGSignal = SIGUSR2;

    createChild();
    parentProcessHandlers();

    for (int i = 0; i < l; i++) {
      counterTriggerSignals ++;
      kill(childPID, TRIGGERSignal);
    }
    kill(childPID, ENDINGSignal);
  }

  else if (Type == 2)
  {
    TRIGGERSignal = SIGUSR1;
    RESPONSESignal = SIGUSR1;
    ENDINGSignal = SIGUSR2;

    createChild();
    parentProcessHandlers();

    sigset_t responseMask, prevMask;
    sigemptyset(&responseMask);
    sigaddset(&responseMask, RESPONSESignal);
    if (sigprocmask(SIG_BLOCK, &responseMask, &prevMask) == -1) {
      printf("Can't set sigmask\n");
      exit(1);
    }

    for(int i = 0; i < l; i++) {
      flag = 0;
      kill(childPID, TRIGGERSignal);
      counterTriggerSignals++;

      while(flag == 0) {
        sigsuspend(&prevMask); //prevMask jest pustym zbiorem, więc odbierając dowolny sygnał jest wybudzany zmieniwszy flag na 1
      }
    }
    kill(childPID, ENDINGSignal);
  }
  //  else if (Type == 3) {
  //   TRIGGERSignal = SIGRTMIN;
  //   RESPONSESignal = SIGRTMIN;
  //   ENDINGSignal = SIGRTMAX;
  //
  //   createChild();
  //   parentProcessHandlers();
  //
  //   for(int i = 0; i < l; i++) {
  //         kill(childPID, TRIGGERSignal);
  //         counterTriggerSignals++;
  //   }
  //   kill(childPID, ENDINGSignal);
  // }

  else {
     printf("Wrong argument format\n");
  }

  printf("Parent process sent: %d trigger signals to child\n", counterTriggerSignals);
  printf("Parent process received: %d response signals from child\n", counterResponseSignals);

  return 0;
}
