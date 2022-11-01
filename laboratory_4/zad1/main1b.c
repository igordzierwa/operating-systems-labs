#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/types.h> //dla funkcji fork
#include <unistd.h> //dla funkcji fork


int flag = 1;
int childPID;

void createChild() {
  int child = fork();

  if (child == 0) {
    //przyjmuje listę argumentów ale, nie podajemy tutaj ścieżki do pliku, lecz samą jego nazwę
    execlp("./dataScript", NULL);
  } else {
    childPID = child;
  }
}

void serivceINTsig(int sig) {
    printf("Program has received signal no.%d - SIGINT\n",sig);
    printf("End of program\n");

    //jeśli proces potomny wykonuje działanie, o czym informuje flaga
    if(flag) {
      //SIGKILL - unicestwienie procesu, powoduje utratę wszystkich danych w nim (9)
      kill(childPID, SIGKILL);
    }
    //Funkcja kończy działanie aplikacji po zwolnieniu zasobów globalnych.
    exit(1);
}

//SIGTSTP - terminalowe zatrzymanie (CTRL + Z lub CTRL + Y)
void serviceTSTPsig(int sig) {
  printf("Program has received signal no.%d - SIGTSP\n",sig);

  if (flag) {
    printf("Waiting for CTRL+Z - \"continue\" or CTRL+C - \"end of the program\". \n");
    flag = 0;
    kill(childPID, SIGKILL);
  } else {
    createChild();
    printf("Program has been resumed\n");
    flag = 1;
  }
}

int main(void) {
  //zdefiniowana struktura definiuje sposób obsługi sygnału
  struct sigaction action;
  action.sa_handler = serviceTSTPsig;
  //sa_mask (maska sygnałów) – zawiera zbiór sygnałów, które mają być zablokowane na czas wykonania tej funkcji.
  sigemptyset(&action.sa_mask);
  //sa_flags - nadzoruje obsługę sygnału przez jądro
  action.sa_flags = 0;
  //NULL jako ostatni argument, sugerując że nie było zarejestrowanej poprzedniej akcji
  sigaction(SIGTSTP, &action, NULL);
  signal(SIGINT, serivceINTsig);

  createChild();

  //pętla nieskończona, żeby działał program, bo jeśli by jej nie było, to następuje skończenie programu i dalej wykonuje się
  //tylko skrypt z basha
  while(1) {
  }

  return 0;
}
