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

int main (void) {
  char* path = getenv(PATHNAME);
  if (path == NULL) {
    printf("environment variable was not found\n");
    exit(1);
  }

  int key = ftok(path, MAX_KEY);
  int serverQueue = msgget(key, 0644 | IPC_CREAT);
  int clientQueue = msgget(IPC_PRIVATE, 0644); //this special value is used for key, the system call ignores everything but the least significant 9 bits of msgflg and creates a new message queue (on success).

  if (serverQueue == -1 || clientQueue == -1) {
    printf("Can't get access to queue //serverQueue: %d//clientQueue: %d//.\n", serverQueue, clientQueue);
  } else {
    printf("Acces to queue.\n");
  }

  //wysyłanie wiadomości na kolejkę serwera, otrzymanie wiadomości do swojej kolejki
  struct messagebuf messageConnection;
  struct messagebuf messageOption;

  messageConnection.pid = getpid();
  messageConnection.type = CONNECT;
  sprintf(messageConnection.txt_msg, "%d", clientQueue);

  //wysyłamy wiadomość odnośnie połączenia z serwerem (nie mamy jeszcze odpowiedzi)
  int sendFlag = msgsnd(serverQueue, &messageConnection, MAX_MSG_LENGTH, 0);
  if (sendFlag == -1) {
    printf("Error after sending message with CONNECTION to server\n");
  }

  //odbieramy wiadomość z serwera odnośnie połączenia
  int receivedFlag = msgrcv(clientQueue, &messageConnection, MAX_MSG_LENGTH, 0, 0);
  if (receivedFlag == -1) {
    printf("Error after receiving message with CONNECTION from server\n");
  }

  if (messageConnection.type == CONNECT) {
    printf("Connected with server.\n");
  } else {
    printf("Can't connect with server\n");
  }

  int type;
  char* line = NULL;
  size_t size = 0;
  printf("COMMANDS:\n0.END SERVER\n1.DISCONNECT CLIENT\n2.TIME\n");

  while(1) {
    int readCheck = getline(&line, &size, stdin);
    if (readCheck != -1) {
      type = atoi(line);
      printf("Type: %d\n", type);

      messageOption.pid = getpid();
      if (type == END) {
        messageOption.type = END;
        msgsnd(serverQueue, &messageOption, MAX_MSG_LENGTH, 0);
        break;
      } else if (type == DISCONNECT) {
        messageOption.type = DISCONNECT;
        msgsnd(serverQueue, &messageOption, MAX_MSG_LENGTH, 0);
        printf("Disconnected with server\n");
        break;
      } else if (type == TIME) {
        messageOption.type = TIME;
        msgsnd(serverQueue, &messageOption, MAX_MSG_LENGTH, 0);
      } else {
        printf("Wrong option, choose again\n");
        continue;
      }

      msgrcv(clientQueue, &messageOption, MAX_MSG_LENGTH, 0, 0);
      if (type == TIME) {
        printf("The return message from server:\n");
        printf("%s\n", messageOption.txt_msg);
      }
    }
  }

  int checkRemove = msgctl(clientQueue, IPC_RMID, NULL);
  if (checkRemove == -1) {
    printf("Can't remove queue\n");
    exit(1);
  } else {
    printf("Queue successfuly removed.\n");
  }

  return 0;
}
