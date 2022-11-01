#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <mqueue.h>
#include <fcntl.h>
#include <time.h>
#include "header.h"

int clientsPIDS[MAX_CLIENTS];
int counter = 0;

void clientConnect(char* msg){
  char* token = strtok(msg, "#"); //odczytujemy typ

  token = strtok(NULL, "#"); //odczytujemy pid clienta
  int pidClient = atoi(token);

  token = strtok(NULL, "#"); //odczytujemy deskryptor
  int clientQueueDesc = atoi(token);

  char messageComunicat [MAX_MSG_LENGTH];
  int pid = getpid();

  if(counter == MAX_CLIENTS){
    printf("Can't connect more clients.\n");
    sprintf(messageComunicat, "%d#%d", QUEUEFULL, pid);
  } else{
    printf("Client connection, with PID: %d.\n", pidClient);

    clientsPIDS[counter] = pidClient;
    sprintf(messageComunicat, "%d#%d#%d", CONNECT, pid, counter);
    counter++;
  }

  //dodanie komunikatu do kolejki klienta jako wiadomosc zwrotna
  char name[MAX_MSG_LENGTH];
  memset(name, '\0', MAX_MSG_LENGTH);
  sprintf(name, "/%d", pidClient);
  clientQueueDesc = mq_open(name, O_WRONLY);
  mq_send(clientQueueDesc, messageComunicat, MAX_MSG_LENGTH, 0);
  mq_close(clientQueueDesc);
}

void clientDisconnect (int pid){
  printf("Disconecting client.\n");
  int index = -1;

  for (int i = 0; i < counter; i++){
    if(clientsPIDS[i] == pid){
      index = i;
    }
  }

  if(index == -1){
    printf("Client with this PID does not exsist.\n");
  } else{
    for(int j = index; j < counter-1; j++){
      clientsPIDS[j] = clientsPIDS[j+1];
    }
    counter--;
    printf("Client PID: %d has been disconnected\n", pid);
  }
}

void clientTime(char* msg){
  printf("Time.\n");
  int clientQ = -1;

  char* token = strtok(msg, "#"); //odczytujemy typ
	token = strtok(NULL, "#");
  int pidClient = atoi(token);

  for (int i = 0; i < counter; i++){
    if(clientsPIDS[i] == pidClient){
      clientQ = 1;
    }
  }
  if (clientQ == -1) {
    printf("Client with this PID does not exsist\n");
  }

  char messageComunicat [MAX_MSG_LENGTH];
    time_t t;
    struct tm * pointerTm;
    time(&t);
    pointerTm = localtime(&t);

    sprintf(messageComunicat, "%d#%d#Time :%d:%d:%d",TIME, pidClient, pointerTm->tm_hour, pointerTm->tm_min, pointerTm->tm_sec);

    //dodanie komunikatu do kolejki klienta jako wiadomosc zwrotna
    char name[MAX_MSG_LENGTH];
    memset(name, '\0', MAX_MSG_LENGTH);
    sprintf(name, "/%d", pidClient);
    int openedClientQueue = mq_open(name, O_WRONLY);
    mq_send(openedClientQueue, messageComunicat, MAX_MSG_LENGTH, 0);
    mq_close(openedClientQueue);
}

int main(void){

  int serverQueueDesc;
  struct mq_attr serverQ;
  serverQ.mq_flags = 0; //sygnalizator kolejki
	serverQ.mq_maxmsg = 10; //maksymalna liczba komunikatow w kolejce (do kazdego po 2, ktorych maksymalnie jest 3)
  serverQ.mq_msgsize = MAX_MSG_LENGTH; //makymalny rozmiar jednego komunikatu
  serverQ.mq_curmsgs = 0; //aktualna liczba komunikatow w kolejce

  serverQueueDesc = mq_open(QNAME, O_CREAT | O_RDONLY, 0644, &serverQ);
  if(serverQueueDesc != -1){
    printf("Server queue has been created\n");
  } else{
    printf("Error with creating server queue\n");
    exit(1);
  }

  char buffer[MAX_MSG_LENGTH];
  char buffer2[MAX_MSG_LENGTH];
  memset(buffer, 0, MAX_MSG_LENGTH);
  int type;
  int messageFlag; //czy otrzymanny zostal komunikat od klienta

  while (1) {
    messageFlag = mq_receive(serverQueueDesc, buffer, MAX_MSG_LENGTH, NULL);

    if (messageFlag == -1) {
      continue;
    }

    for (int i = 0; i < MAX_MSG_LENGTH; i++){
      buffer2[i] = buffer[i];
    }

    char* token = strtok(buffer, "#");
    type = atoi(token);
    int pid = atoi(strtok(NULL, "#"));

    if (type == CONNECT) {
      printf("Message from Client: %d, who has been already connected.\n", pid);
    } else {
      printf("Message from Client: %d, who has chosen option: %s\n", pid, token);
    }
    if(type == CONNECT){
      clientConnect(buffer2);
    }
    if(type == END){
      printf("Ending of running program.\n");
      break;
    }
    else if(type == DISCONNECT){
      clientDisconnect(pid);
    }
    else if(type == TIME){
      clientTime(buffer2);
    }
  }

  if (mq_close(serverQueueDesc) == 0) {
    mq_unlink(QNAME);
    printf("Queue successfuly removed\n");
  } else {
    printf("Can't remove queue\n");
  }

  return 0;
}
