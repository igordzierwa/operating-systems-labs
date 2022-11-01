#ifndef HEADER_H
#define HEADER_H

//rodzaje komunikatów
static const int END = 0;
static const int CONNECT = 3;
static const int DISCONNECT = 1;
static const int TIME = 2;
static const int QUEUEFULL = 4;

//Ograniczenia
#define MAX_MSG_LENGTH 20 //ograniczona długośc komunikatu
#define MAX_KEY 10 //ograniczenie dotyczące małej ilości kluczy
#define MAX_CLIENTS 3 //ograniczenie do rozmiaru tablicy klientów, którzy mogą zgłosić się do serwera

//struktura budowy komunikatu
struct messagebuf {
  pid_t pid;
  int type;
  char txt_msg [MAX_MSG_LENGTH];
};

//ścieżka do generowania kluczów kolejek
const char *PATHNAME = "HOME";

#endif
