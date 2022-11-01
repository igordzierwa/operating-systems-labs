#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h> //do funkcji sleep
#include <sys/types.h>
#include <signal.h>

int flag = 1;

//SIGINT - terminalowe przerwanie (CTRL + C)
void serivceINTsig(int sig) {
    printf("Program has received signal no.%d - SIGINT\n",sig);
    printf("End of program\n");
    //Funkcja kończy działanie aplikacji po zwolnieniu zasobów globalnych.
    exit(1);
}

//SIGTSTP - terminalowe zatrzymanie (CTRL + Z lub CTRL + Y)
void serviceTSTPsig(int sig) {
  printf("Program has received signal no.%d - SIGTSP\n",sig);

  if (flag) {
    printf("Waiting for CTRL+Z - \"continue\" or CTRL+C - \"end of the program\". \n");
    flag = 0;
  } else {
    printf("Program has been resumed\n");
    flag = 1;
  }
}

int main(void) {
  int hours, minutes;
  flag = 1;

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

  //arytmetyczny typ czasu
  time_t now;
  //uzyskanie aktualnego czasu, funkcja time() zwraca aktualny czas systemu jako
  //wartość time_t
  time(&now);

  while(1) {
    if(flag) {
      //funkcja localtime konwertuje wartośc time_t do czasu kalendarzowego i zwraca
      struct tm *local = localtime(&now);
      hours = local -> tm_hour;
      minutes = local -> tm_min;
      printf("%02d : %02d\n", hours, minutes);
      sleep(1);
    } else pause();
  }

  return 0;
}
