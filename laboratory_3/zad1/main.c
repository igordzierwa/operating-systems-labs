#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <ftw.h>

struct tm argDate;
char compOperator;

char dateComparsion(struct tm* firstDate, struct tm* secondDate) {
  if((firstDate->tm_year < secondDate->tm_year) || (firstDate->tm_year == secondDate->tm_year && firstDate->tm_mon < secondDate->tm_mon) ||
    (firstDate->tm_year == secondDate->tm_year && firstDate->tm_mon == secondDate->tm_mon && firstDate->tm_mday < secondDate->tm_mday)) {
      return '<';
  }
  else if((firstDate->tm_year > secondDate->tm_year) || (firstDate->tm_year == secondDate->tm_year && firstDate->tm_mon > secondDate->tm_mon) ||
    (firstDate->tm_year == secondDate->tm_year && firstDate->tm_mon == secondDate->tm_mon && firstDate->tm_mday > secondDate->tm_mday)) {
      return '>';
  }
  else {
    return '=';
  }
}

void printRights(const struct stat* buffer) {
  //porównywanie stałej za pomocą bitowego AND '&'
  printf("Rights: ");
  printf((buffer->st_mode & S_IRUSR) ? "r" : "-");
  printf((buffer->st_mode & S_IWUSR) ? "w" : "-");
  printf((buffer->st_mode & S_IXUSR) ? "x" : "-");

  printf((buffer->st_mode & S_IRGRP) ? "r" : "-");
  printf((buffer->st_mode & S_IWGRP) ? "w" : "-");
  printf((buffer->st_mode & S_IXGRP) ? "x" : "-");

  printf((buffer->st_mode & S_IROTH) ? "r" : "-");
  printf((buffer->st_mode & S_IWOTH) ? "w" : "-");
  printf((buffer->st_mode & S_IXUSR) ? "x" : "-");

  printf("\n");
}

void printAboutFile(const char * path, const struct stat * buffer) {
  struct tm* localDate;
  localDate = localtime(&(buffer->st_mtime));

  if (dateComparsion(localDate, &argDate) == compOperator) {
    printf("Path: %s\n", path);

    char datePrint[100];
    strftime(datePrint, 100, "%d/%m/%Y  %T", localtime(&(buffer->st_mtime)));
    printf("Date: %s\n", datePrint);

    printRights(buffer);

    printf("Size: %lld bytes\n", buffer->st_size);

    printf("\n");
  }
}

void searchDir(char * path) {
  strcat(path, "/");

  int pathEndIndex = strlen(path);
  path[pathEndIndex] = '\0';

  DIR * currentDirectory; //struktura reprezentująca strumień katalogowy
  currentDirectory = opendir(path);
  if (currentDirectory == NULL) {
    printf("Can't open directory!\n");
    exit(1);
  }

  struct dirent* directoryFile; //struktura wskazująca
  struct stat bufferStat;

  while((directoryFile = readdir(currentDirectory)) != NULL) {
    if (strcmp(directoryFile->d_name, ".") != 0 && strcmp(directoryFile->d_name, "..") != 0) {
      strcat(path, directoryFile->d_name);

      lstat(path, &bufferStat);
      if (S_ISREG(bufferStat.st_mode)) {
        printAboutFile(path, &bufferStat);
      }
      else if (S_ISDIR(bufferStat.st_mode)) { //przeszukiwanie podkatalogu
        int newProcess;
        newProcess = fork(); //utworzenie procesu

        if (newProcess < 0) {
          printf("Fork function failed\n");
          exit(1);

        } else if (newProcess == 0) {

          printf("Child process %d is working in subdirectory: %s\n\n", getpid(), path);
          searchDir(path);
          return; //w tym miejscu proces zostaje zakończony - funkcja main procesu wykona instrukcje return

        } else { //w procesie macierzystym zwracany jest PID procesu potomnego
          printf("Ending child process %d\n\n", getpid());
          wait(NULL); //proces macierzysty czeka na zakończenie procesu potomnego
        }

        searchDir(path);
      }

      path[pathEndIndex] = '\0';
    }
  }
}

int main(int argc, char** argv) {
  if (argc != 4) {
    printf("Wrong number of arguments!\n");
    exit(1);
  }

  char * path = malloc(1000 * sizeof(char));

  //ścieżka względna do bezwzględnej
  if(argv[1][0] == '/') {
    strcpy(path, argv[1]);
  } else {
    getcwd(path, 1000);
    strcat(path, "/");
    strcat(path, argv[1]);
  }

  printf("Path: %s\n", path);

  if (strcmp(argv[2], "<") == 0) {
    compOperator = '<';
  } else if (strcmp(argv[2], ">") == 0) {
    compOperator = '>';
  } else if (strcmp(argv[2], "=") == 0) {
    compOperator = '=';
  } else {
    printf("Wrong operator\n");
    exit(1);
  }

  printf("Operator: %c\n",compOperator);
  strptime(argv[3], "%d %m %Y", &argDate);

  printf("Searching with standard function and multiple process\n");
  printf("\n");
  searchDir(path);


  return 0;
}
