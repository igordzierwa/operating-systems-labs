#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>

const int argsLimit = 15;
int currentLine = 0;

void convertWhiteSpace (char * buff) { //funkcja potrzebna żeby rozdzielić łańcuch poprzez delimetery
  int i = 0;
  while(buff[i] != '\0') {
    if (isspace(buff[i]) != 0) {
      buff[i] = ' ';
    }
    i++;
  }
}

void readLineBatchFile (char * command) {
  convertWhiteSpace(command);

  char* firstArg = strtok(command, " "); //funkcja jako pierwsze wywołanie zwraca wskaźnik do słowa, najpierw pobieramy wskaźnik do 1 słowa, bo musimy podać w 1 argumencie nazwę łańcucha, później NULL, po to to jest
  if (firstArg == NULL) {
    return;
  }

  char* args[argsLimit + 1]; //bo w ostatniej komórce jest znak nowej linii
  args[0] = firstArg;
  int actualArg = 1;

  while((args[actualArg]=strtok(NULL, " "))!=NULL){
    actualArg++;

    if (actualArg > argsLimit) {
      printf("Too many arguments, actual arguments limit is: %d\n", argsLimit);
      exit(1);
    }
  }

  int newProcess = fork();
  if (newProcess < 0) {
    printf("Fork function failed\n");
    exit(1);
  }

  else if (newProcess == 0) {
    int newExec;
    newExec = execvp(firstArg, args); //analogicznie firstArg jest początkiem polecenia w ramach nazwy pliku
    if (newExec == -1) {
      exit(1);
    }
  }

  else {
    int status;
    wait(&status);

    if (status != 0) {
      printf("Error: Command - %s in line %d\n", firstArg, currentLine);
      exit(1);
    }
  }
  printf("\n\n");
}

int main (int argc, char** argv) {
  if (argc != 2) {
    printf("Wrong number of arguments!\n");
  }

  FILE* file = fopen(argv[1], "r");
  char* command;
  size_t size = 0;
  size_t read;

  while ((read = getline(&command, &size, file)) != -1) {
    currentLine ++;
    readLineBatchFile(command);
  }

  return 0;
}
