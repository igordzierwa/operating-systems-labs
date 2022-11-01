#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> //time, localtime, struct tm
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>  //ftok
#include <sys/msg.h> //msgget, msgrcv, msgsnd
#include <unistd.h> //sleep, getpid
#include "header.h"

int clientsQueue[MAX_CLIENTS];
int clientsPID [MAX_CLIENTS];
int counter = 0;

//funkcja służąca do usunięcia kolejki
void removeQueue(int queueID) {
  int check = msgctl(queueID, IPC_RMID, NULL) == 0; //Działanie funkcji sprowadza się do przekopiowania odpowiednich wartości od lub do użytkownika, lub skasowania kolejki
  if (check != -1) {
    printf("Server queue has been successfuly removed\n");
  } else {
    printf("Server queue has not been removed\n");
    exit(1);
  }
}

//funkcja obsługująca połączenie klienta
void clientConnect(struct messagebuf * msg) {
  if (counter == MAX_CLIENTS) {
    printf("Can't connect more clients\n");
    msg->type = QUEUEFULL;
  } else {
    printf("Client connection, with PID: %d.\n", msg->pid);
    int clientqueue = atoi(msg->txt_msg); //identyfikator kolejki danego klienta
    clientsQueue[counter] = clientqueue;
    clientsPID[counter] = msg->pid;
    sprintf(msg->txt_msg, "%d", counter); //wysyła sformatowane dane wyjściowe do łańcucha
    counter++;
    msgsnd(clientqueue, msg, MAX_MSG_LENGTH, 0);
  }
}

//funkcja obsługująca odłączenie klienta
void clientDisconnect(int pid) {
  printf("Disconnecting client.\n");
  int index = -1;

  for (int i = 0; i < counter; i++) {
    if (clientsPID[i] == pid) {
      index = i;
    }
  }

  if (index == -1) {
    printf("Client with this PID does not exsist\n");
  } else {
    for (int j = index; j < counter - 1; j++) {
      clientsQueue[j] = clientsQueue[j + 1];
      clientsPID[j] = clientsPID[j + 1];
    }
    counter --;
    printf("Client PID: %d has been disconnected\n", pid);
  }
}

//funkcja obsługująca usługę czasu (TIME)
void clientTime(struct messagebuf * msg) {
  printf("Time.\n");
  int clientQ = -1;

  for (int i = 0; i < counter; i++) {
    if (clientsPID[i] == msg->pid) {
      clientQ = clientsQueue[i];
    }
  }

  if (clientQ == -1) {
    printf("Client with this PID does not exsist\n");
  } else {
    time_t t;
    struct tm * pointerTm;
    time (&t);
    pointerTm = localtime(&t);

    sprintf(msg->txt_msg, "Time: %d:%d:%d", pointerTm->tm_hour, pointerTm->tm_min, pointerTm->tm_sec);
    msgsnd(clientQ, msg, MAX_MSG_LENGTH, 0);
  }
}

int main (void) {
  char* path = getenv(PATHNAME);
  if (path == NULL) {
    printf("environment variable was not found\n");
    exit(1);
  }

  int key = ftok(path, MAX_KEY); //
  int serverQueue = msgget(key, 0644 | IPC_CREAT); //utworzenie kolejki servera
  printf("Server queue has been created\n");

  struct messagebuf msg;

  int messageFlag; //czy otrzymany został komunikat z klienta

  while (1) {
    messageFlag = msgrcv(serverQueue, &msg, MAX_MSG_LENGTH, 0, 0); //pierwsze 0 -- typ komunikatu nie jest brany, funkcja pobiera pierwszy komunikat dowolnego typu
                                                                             //drugie 0 -- flaga specjalizująca zachowanie funkcji w warunkach nietypowych
    if (msg.type == CONNECT) {
      printf("Messege from Client: %d, who has been already connected.\n", msg.pid);
    } else {
      printf("Messege from Client: %d, who has chosen option: %d\n", msg.pid, msg.type);
    }

    if (messageFlag == -1) {
      continue;
    } else {
      if (msg.type == CONNECT) {
        clientConnect(&msg);
      } else if (msg.type == DISCONNECT) {
        clientDisconnect(msg.pid);
      } else if (msg.type == TIME) {
        clientTime(&msg);
      } else if (msg.type == END) {
        break;
      } else {
        printf("Wrong option\n");
      }
    }
  }

  removeQueue(serverQueue);

  return 0;
}
