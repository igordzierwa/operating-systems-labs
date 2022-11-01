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
      else if (S_ISDIR(bufferStat.st_mode)) {
        searchDir(path);
      }

      path[pathEndIndex] = '\0';
    }
  }
}

int nftwFunction(const char * path, const struct stat * bufferStat, int x, struct FTW* ftw) {
  if (x == FTW_F) {
    printAboutFile(path, bufferStat);
  }
  return 0;
}

int main(int argc, char** argv) {
  if (argc != 5) {
    printf("Wrong number of arguments!\n");
    exit(1);
  }

  char * path = malloc(1000 * sizeof(char));

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

  if (strcmp(argv[4], "standard") == 0) {
    printf("Searching with standard function\n");
    printf("\n");
    searchDir(path);
  } else if (strcmp(argv[4], "nftw") == 0) {
    printf("Searching with nftw function\n");
    printf("\n");
    nftw(path, nftwFunction, 10, FTW_PHYS); //FTW_PHYS  - do not follow symbolic links
  } else {
    printf("Choose standard or nftw function\n)");
    printf("\n");
    exit(1);
  }

  return 0;
}
