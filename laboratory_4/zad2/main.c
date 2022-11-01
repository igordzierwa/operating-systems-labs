#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

int receivedChildSignals; //liczba sygnałów odebranych przez potomka = child_count
int counterTriggerSignals; //liczba sygnałów wysłanych do potomka = sent_count
int counterResponseSignals; //liczba odebranych sygnałów przez rodzica wysłanych od potomka = parent_count

int childPID;
int parentPID;
int type;
int l; //liczba sygnałów

//funkcja będąca reakcją rodzica na odbierane sygnały
void parentSignalReceiver(int sig) {
  counterResponseSignals++;

  if (sig == SIGUSR1) {
    printf("Parent received %d signals.\n", counterResponseSignals);
  }
  else if (sig == SIGINT) {
    printf("Parent process received SIGINT\n");
    if (type == 1 || type == 2) {
      kill(childPID, SIGUSR2);
      exit(1);
    }
    // else if (type == 3) {
    //   kill(childPID, SIGRTMAX);
    //   exit(1);
    // }
  }
}

//funkcja będąca reakcją dziecka na odbierane sygnały
void childSignalReceiver(int sig) {
  receivedChildSignals++;

  if (sig == SIGUSR1) {
    printf("Child process received: %d signals.\n", receivedChildSignals);
    kill(parentPID, SIGUSR1);
  }
  // else if (sig == SIGRTMIN) {
  //   printf("Child process received: %d signals.\n", receivedChildSignals);
  //   kill(parentPID, SIGRTMIN);
  // }
  else if (sig == SIGUSR2) {
    printf("Child process received: SIGUSR2.\n");
    exit(1);
  }
  // else if (sig == SIGRTMAX) {
  //   printf("Child process received: SIGRTMAX.\n");
  //   exit(1);
  // }
}

void parentFunction(int numbOfSignals) {
  struct sigaction action;
  action.sa_handler = parentSignalReceiver;
  sigfillset(&action.sa_mask);
  action.sa_flags = 0;

  if (type == 1 || type == 2) {
    if (sigaction(SIGINT, &action, NULL) < -1 ||
        sigaction(SIGUSR1, &action, NULL) < -1) {
          exit(1);
        }
  }
  // else if (type == 3) {
  //   if (sigaction(SIGINT, &action, NULL) < -1 ||
  //       sigaction(SIGRTMIN, &action, NULL) < -1) {
  //         exit(1);
  //       }
  // }

  for (int i = 0; i < numbOfSignals; i++) {
    counterTriggerSignals++;
    printf("Parent process: send %d signals\n", counterTriggerSignals);
    if (type == 1) {
      kill(childPID, SIGUSR1);
    }
    else if (type == 2) {
      kill(childPID, SIGUSR1);
      pause();//Zawiesza wywołujący proces aż do chwili otrzymania dowolnego sygnału.
      //Jeśli sygnał jest ignorowany przez proces, to funkcja pause też go ignoruje.
    }
    // else if (type == 3) {
    //   kill(childPID, SIGRTMIN);
    // }
  }
  if (type == 1 || type == 2) {
    kill(childPID, SIGUSR2);
  }
  // else if (type == 3) {
  //   kill(childPID, SIGRTMAX);
  // }

  //jeśli proces potomny dostanie SIGUSR2 szybciej niż przekaże spowrotem sygnały do rodzica
  //następuje wstrzymanie procesu do momentu otrzymania sygnału - w tym przypadku SIGINT
  while(counterResponseSignals < numbOfSignals) pause();
}

void childFunction() {
  struct sigaction action;
  action.sa_handler = childSignalReceiver;
  sigfillset(&action.sa_mask);
  action.sa_flags = 0;

  if (type == 1 || type == 2) {
    if (sigaction(SIGINT, &action, NULL) < -1 ||
        sigaction(SIGUSR1, &action, NULL) < -1 ||
        sigaction(SIGUSR2, &action, NULL) < -1) {
          exit(1);
    }
  }
  // else if (type == 3) {
  //   if (sigaction(SIGINT, &action, NULL) < -1 ||
  //       sigaction(SIGRTMIN, &action, NULL) < -1 ||
  //       sigaction(SIGRTMAX, &action, NULL) < -1) {
  //         exit(1);
  //   }
  // }

  while(1) {
    pause();
  }
}

int main(int argc, char** argv) {
  if (argc < 3) {
    printf("Wrong number of arguments\n");
    exit(1);
  }

  l = atoi(argv[1]);
  type = atoi(argv[2]);

  sigset_t newMask;
  sigset_t oldMask;
  sigfillset(&newMask);
  // Wszystkie pozostałe sygnały są blokowane w procesie potomnym - proces potomny dziedziczy
  //po rodzicu maski sygnałów
  sigdelset(&newMask, SIGINT);
  sigdelset(&newMask, SIGUSR1);
  sigdelset(&newMask, SIGUSR2);
  sigprocmask(SIG_SETMASK, &newMask, &oldMask);

  parentPID = getpid();
  receivedChildSignals = 0;
  counterTriggerSignals = 0;
  counterResponseSignals = 0;

  childPID = fork();
  if (childPID < 0) {
    printf("Can't create child process - ending.\n");
    exit(1);
  }
  else if (childPID == 0) {
    childFunction();
  }
  else {
    sleep(1);
    parentFunction(l);
  }

  return 0;
}
