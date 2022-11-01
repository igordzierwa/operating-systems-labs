#ifndef HEADER_H
#define HEADER_H

//Ograniczenia
#define MAX_CLIENTS 3
#define MAX_MSG_LENGTH 50
#define MAX_KEY 10

//rodzaje komunikatów
static const int END = 0;
static const int DISCONNECT = 1;
static const int TIME = 2;
static const int CONNECT = 3;
static const int QUEUEFULL = 4;

const char* PATHNAME = "HOME";
const char* QNAME = "/QUEUE1";


#endif
