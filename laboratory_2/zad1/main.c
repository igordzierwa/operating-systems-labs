#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/times.h>

double calculateTime(clock_t t1, clock_t t2) { //dobrze
  return (double) (t2-t1) / sysconf(_SC_CLK_TCK);
}


void randomData(char text[], int blockSize) {
  char set[53] = "abcdefghijklmnopqrstuvwxyz";
  for (int i = 0; i < blockSize; i++) {
    text[i] = set[rand()%25];
  }
}

void generate(const char * filePath, int recordsNumber, int recordLength) {
  FILE * file = fopen(filePath, "w");
  char buffer[recordLength];
  char newLine = '\n';

  if (file == NULL) {
    printf("Can't open file!\n");
    exit(1);
  }

  for (int i = 0; i < recordsNumber; i++) {
    randomData(buffer, recordLength);
    fwrite(buffer, 1, recordLength, file);
    if (i != recordsNumber - 1) {
      fwrite(&newLine, 1, 1, file);
    }
  }

  fclose(file);
}

char* getRecordSys(int file, int recordLength, int recordsNumber) {
  lseek(file, (recordLength + 1)*recordsNumber, SEEK_SET);
  char * buffor = (char*) calloc(recordLength, sizeof(char));
  read(file, buffor, recordLength);
  return buffor;
}

void insertRecordSys(int file, int recordLength, int recordsNumber, char* buffor) {
  lseek(file, (recordLength + 1)*recordsNumber, SEEK_SET);
  write(file, buffor, recordLength);
}

char* getRecordLib(FILE * file, int recordLength, int recordsNumber) {
  char * buffor = (char*) calloc(recordLength, sizeof(char));

  fseek(file, (recordLength + 1)*recordsNumber, SEEK_SET); //+1 bo znak końca lini '/n'
  fread(buffor, sizeof(buffor[0]), recordLength, file);

  return buffor;
}

void insertRecordLib(FILE * file, int recordLength, int recordsNumber, char* buffor) {
  fseek(file, (recordLength+1)*recordsNumber, SEEK_SET);
  fwrite(buffor, sizeof(buffor[0]), recordLength, file);
}

void copyLib(const char * filePath1, const char * filePath2, int recordsNumber, int recordLength) { //dobrze
  FILE * fromCopy = fopen(filePath1, "r");
  if (fromCopy == NULL) {
    printf("Can't open file!\n");
    exit(1);
  }

  FILE * toCopy = fopen(filePath2, "w");
  if (toCopy == NULL) {
    printf("Can't open file!\n");
    exit(1);
  }

  char buffer[recordLength + 1];

  for (int i = 0; i < recordsNumber; i++) {
    fread(buffer, 1, recordLength + 1, fromCopy);
    if (i == recordsNumber - 1) {
      fwrite(buffer, 1, recordLength, toCopy);
    } else {
      fwrite(buffer, 1, recordLength + 1, toCopy);
    }
  }

  fclose(fromCopy);
  fclose(toCopy);
}

void copySys(const char * filePath1, const char * filePath2, int recordsNumber, int recordLength) { //dobrze
  int fromCopy = open(filePath1, O_RDONLY);
  if (fromCopy == -1) {
    printf("Can't open file!\n");
    exit(1);
  }

  int toCopy = open(filePath2, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
  if (toCopy == -1) {
    printf("Can't open file\n");
    exit(1);
  }

  char buffer[recordLength + 1];

  for (int i = 0; i < recordsNumber; i++) {
    read(fromCopy, buffer, recordLength + 1);
    if (i == recordsNumber - 1) {
      write(toCopy, buffer, recordLength);
    } else {
      write(toCopy, buffer, recordLength + 1);
    }
  }

  close(fromCopy);
  close(toCopy);
}

void sortLib(const char * filePath, int recordsNumber, int recordLength) { //źle
  FILE * file = fopen(filePath, "r+");
  if (file == NULL) {
    printf("Can't open file\n");
    exit(1);
  }

  for (int i = 1; i < recordsNumber; i++) {
    int j = i;
    char * x = getRecordLib(file, recordLength, i);
    while((j > 0) && (getRecordLib(file, recordLength, j-1)[0] > x[0])) {
      insertRecordLib(file, recordLength, j, getRecordLib(file, recordLength, j-1));
      j--;
    }
    insertRecordLib(file, recordLength, j, x);
  }

  fclose(file);
}

void sortSys(const char * filePath, int recordsNumber, int recordLength) { //źle
  int file = open(filePath, O_RDWR);
  if (file == -1) {
    printf("Can't open file\n");
    exit(1);
  }

  for (int i = 1; i < recordsNumber; i++) {
    int j = i;
    char * x = getRecordSys(file, recordLength, i);
    while((j > 0) && (getRecordSys(file, recordLength, j-1)[0] > x[0])) {
      insertRecordSys(file, recordLength, j, getRecordSys(file, recordLength, j-1));
      j--;
    }
    insertRecordSys(file, recordLength, j, x);
  }

  close(file);
}


int main (int argc, char** argv) { //dobrze
  srand(time(NULL));

  if (argc < 2) {
    printf("To small amount of arguments\n");
    exit(1);
  }

  struct tms start, end;
  times(&start);

  if (strcmp(argv[1], "generate") == 0) {
    if (argc != 5) {
      printf("Wrong arguments!\n");
      exit(1);
    }
    char * filePath = argv[2];
    int recordsNumber = (int) strtol(argv[3], NULL, 10);
    int recordLength = (int) strtol(argv[4], NULL, 10);
    generate(filePath, recordsNumber, recordLength);
  }
  else if (strcmp(argv[1], "sort") == 0) {
    if (argc != 6) {
      printf("Wrong arguments!\n");
      exit(1);
    }
    char * filePath = argv[2];
    int recordsNumber = (int) strtol(argv[3], NULL, 10);
    int recordLength = (int) strtol(argv[4], NULL, 10);
    char * type = argv[5];

    if (strcmp(type, "sys") == 0) {
      sortSys(filePath, recordsNumber, recordLength);
    } else if (strcmp(type, "lib") == 0) {
      sortLib(filePath, recordsNumber, recordLength);
    } else {
      printf("Wrong type!\n");
      exit(1);
    }
  }
  else if (strcmp(argv[1], "copy") == 0) {
    if (argc != 7) {
      printf("Wrong arguments!\n");
      exit(1);
    }
    char * fromCopy = argv[2];
    char * toCopy = argv[3];
    int recordsNumber = (int) strtol(argv[4], NULL, 10);
    int recordLength = (int) strtol(argv[5], NULL, 10);
    char * type = argv[6];

    if (strcmp(type, "sys") == 0) {
      copySys(fromCopy, toCopy, recordsNumber, recordLength);
    } else if (strcmp(type, "lib") == 0) {
      copyLib(fromCopy, toCopy, recordsNumber, recordLength);
    } else {
      printf("Wrong type!\n");
      exit(1);
    }
  }

  times(&end);

  printf("User time: %0.2fs\n", calculateTime(start.tms_utime, end.tms_utime));
  printf("System time: %0.2fs\n", calculateTime(start.tms_stime, end.tms_stime));

  return 0;
}
