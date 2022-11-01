 #include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <mqueue.h>
#include <fcntl.h>
#include "header.h"

int main(void){
  char name[MAX_MSG_LENGTH];
  //wysylanie wiadomosci na kolejke serwera, otrzymanie wiadomosci do swojej kolejki
  char readBuffer[MAX_MSG_LENGTH];
  char writeBuffer[MAX_MSG_LENGTH];
  memset(readBuffer, 0, MAX_MSG_LENGTH);
  memset(writeBuffer, 0, MAX_MSG_LENGTH);
  memset(name, '\0', MAX_MSG_LENGTH);

  int pid = getpid();
  sprintf(name, "/%d", pid);

  int serverQueueDesc = mq_open(QNAME, O_WRONLY); //tylko zapis do kolejki serwera
  if (serverQueueDesc == -1){
		printf("Error at opening server queue.\n");
		exit(1);
	}

  int clientQueueDesc;
  struct mq_attr clientQ;
  clientQ.mq_flags = 0; //sygnalizator kolejki
  clientQ.mq_maxmsg = 4; //maksymalna liczba komunikatow w kolejce
  clientQ.mq_msgsize = MAX_MSG_LENGTH; //makymalny rozmiar jednego komunikatu
  clientQ.mq_curmsgs = 0; //aktualna liczba komunikatow w kolejce
  clientQueueDesc = mq_open(name, O_CREAT | O_RDONLY, 0644, &clientQ);
  if(clientQueueDesc !=-1){
    printf("Client queue has been created.\n");
  }

	//wysylamy wiadomosc odnosnie polaczenia z serwerem (nie mamy jeszcze odpowiedzi)
  sprintf(writeBuffer, "%d#%d#%d", CONNECT, pid, clientQueueDesc);
  int sendFlag = mq_send(serverQueueDesc, writeBuffer, MAX_MSG_LENGTH, 0);
	if (sendFlag == -1) {
		printf("Client queue has been created\n");
	}

	int receivedFlag = mq_receive(clientQueueDesc, readBuffer, MAX_MSG_LENGTH, NULL);
	if (receivedFlag == -1) {
		printf("Error after receiving message with CONNECTION from server\n");
	}

  int type = atoi(strtok(readBuffer, "#"));
  if (type == QUEUEFULL) {
    printf("Queue is full.\n");
  } else if (type == CONNECT) {
		printf("Connected with server.\n");
	} else {
		printf("Can't connect with server.\n");
	}

  size_t size;
  char* message = NULL;
  char* line = NULL;
  printf("COMMANDS:\n0.END SERVER\n1.DISCONNECT CLIENT\n2.TIME\n");

  while(1) {
		//kazdoroazowe "oczyszczanie" bajtow pamieci komunikatow
    memset(writeBuffer, 0, MAX_MSG_LENGTH);
    memset(readBuffer, 0, MAX_MSG_LENGTH);
    int readCheck = getline(&line, &size, stdin);
    if(readCheck != -1) {

      type = atoi(line);
			printf("Type: %d\n", type);

      if(type == END){
        printf("Ending of running program.\n");
				sprintf(writeBuffer, "%d#%d", END, pid);
        mq_send(serverQueueDesc, writeBuffer, MAX_MSG_LENGTH, 0);
        break;
      }
      else if(type == DISCONNECT){
        printf("Client with PID: %d is disconnecting.\n", pid);
				sprintf(writeBuffer, "%d#%d", DISCONNECT, pid);
        mq_send(serverQueueDesc, writeBuffer, MAX_MSG_LENGTH, 0);
        break;
      }
      else if(type == TIME){
        printf("What's the time\n");
        sprintf(writeBuffer, "%d#%d", TIME, pid);
        mq_send(serverQueueDesc, writeBuffer, MAX_MSG_LENGTH, 0);
      }
      else{
        printf("Wront option, choose again\n");
        continue;
      }

			mq_receive(clientQueueDesc, readBuffer, MAX_MSG_LENGTH, NULL);
			message = strtok(readBuffer, "#"); //odczytanie typu
			message = strtok(NULL, "#"); //odczytanie id
			message = strtok(NULL, "#"); //odczytanie wiadomosci

      printf("%s\n", message);
    }
  }

  if (mq_close(clientQueueDesc) == 0) {
    mq_unlink(name);
    printf("Queue successfuly removed\n");
  } else {
    printf("Can't remove queue\n");
  }

  return 0;
}
