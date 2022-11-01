#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>


const int limit = 5; //ograniczenie gorne, ilość połączonych komend w pojedynczym poleceniu
const int secondLimit = 50;
const int argsLimit = 15;

int currentLine = 0;

int finalPipePID;

char commandLine[limit][secondLimit]; //tablica dwuwymiarowa, każda linia poleceń aż do znaku "|" znajduje się w jednym wierszu
int commandLineCounter;


void clear() {
  for (int i = 0; i < limit; i++) {
    for (int j = 0; j < secondLimit; j++) {
      commandLine[i][j] = '\0';
    }
  }
}

//funkcja potrzebna żeby rozdzielić łańcuch poprzez delimetery
void convertWhiteSpace(char * buff) {
  int i = 0;
  while(buff[i] != '\0') {
    if (isspace(buff[i]) != 0) { //konweruje znak biały na spację (ustandaryzowanie)
      buff[i] = ' ';
    }
    i++;
  }
}

//funkcja do rozdzielenia poleceń względem znaku "\"
void convertLine(char * buff) {
  commandLineCounter = 0;
  int position = 0;
  int flag = 1;
  int j = 0;
  for(int i = 0; i < limit && flag == 1; i++) {
    while(buff[j] != '|') {
      commandLine[commandLineCounter][position] = buff[j];
      position++;
      j++;
      if(buff[j+1] == '\0') {
        flag = 0;
        break;
      }
    }
    commandLineCounter++;
    position = 0;
    j++; //przejście dalej w przypadku napotkania '|'
  }
}

void readLineBatchFile (char * command) {
  clear();
  convertLine(command);

  //konwertowanie znaków białych danego wiersza
  for(int i = 0; i < commandLineCounter; i++) {
    convertWhiteSpace(commandLine[i]);
  }

  char* firstArg[limit];
  char* args[limit][argsLimit];

  for (int i = 0; i < commandLineCounter; i++) {
    //funkcja jako pierwsze wywołanie zwraca wskaźnik do słowa, najpierw pobieramy wskaźnik do 1 słowa,
    //bo musimy podać w 1 argumencie nazwę łańcucha, później NULL, po to to jest
    firstArg[i] = strtok(commandLine[i], " ");

    if (*firstArg == NULL) {
      return;
    }

    args[i][0] = firstArg[i];
    int actualArg = 1;

    //rozdzielanie argumentów danego wiersza
    while((args[i][actualArg]=strtok(NULL, " ")) != NULL) {
      actualArg++;
      if (actualArg > argsLimit) {
        printf("Too many arguments, actual arguments limit is: %d\n",argsLimit);
        exit(1);
      }
    }
  }

  int fields[commandLineCounter - 1][2]; //-1 - mając potok, jeden proces czyta, drugi odbiera (np. arg1 | arg2 | arg3 | arg4 -> arg1->arg2, arg2->arg3, arg3->arg4)
  for (int i = 0; i < commandLineCounter - 1; i++) {
    pipe(fields[i]);
  }

  for (int i = 0; i < commandLineCounter; i++) {
    int newProcess = fork();
    if (i == commandLineCounter - 1) {
        finalPipePID = newProcess;
    }
    if (newProcess < 0) {
      printf("Fork function failed\n");
      exit(1);
    }
    else if (newProcess == 0) {
      //potoki za wyjątkiem pierwszego i ostatniego, obsługujące odczyt danych (pierwszy potok nie odbiera danych, ostatni wykonuje więcej operacji)
      //i-1 bo odebranie danych jest z poprzedniego potoku
      if (i != 0 && i != commandLineCounter-1) {
        //zwolnienie deskryptora zapisu danych
        close(fields[i - 1][1]);
        dup2(fields[i - 1][0], STDIN_FILENO);
      }

      //przypadek ostatniego potoku, który tylko odczytuje dane oraz zwalnia deskryptory zapisu danych pozostałych potoków
      //i-1 bo odebranie danych jest z poprzedniego potoku
      if (i == commandLineCounter - 1) {
        dup2(fields[i - 1][0], STDIN_FILENO);
        for (int i = 0; i < commandLineCounter - 1; i++) {
          close(fields[i][0]);
          close(fields[i][1]);
        }
      }

      //potoki za wyjątkiem ostatniego, obsługujące wysyłanie danych
      if (i != commandLineCounter - 1) {
        //zwolenie deskryptora odczytu danych
        close(fields[i][0]);
        dup2(fields[i][1], STDOUT_FILENO);
      }

      int newExec;
      newExec = execvp(firstArg[i], args[i]);
      if (newExec == -1) {
        exit(1);
      }
    }
  }

  for (int i = 0; i < commandLineCounter - 1; i++) {
    close(fields[i][0]);
    close(fields[i][1]);
  }

  waitpid(finalPipePID, NULL, 0); //zawiesza wykonywanie bieżącego procesu dopóki potomek określony przez pid nie zakończy działania
}

int main (int argc, char** argv) {
  if (argc != 2) {
    printf("Wrong number of arguments!\n");
    exit(1);
  }

  FILE* file = fopen(argv[1], "r");
  char* command;
  size_t size = 0;
  size_t read;

  while ((read = getline(&command, &size, file)) != -1) {
    currentLine ++;
    printf("Line %d\n",currentLine);
    readLineBatchFile(command);
    printf("\n");
  }
  return 0;
}
