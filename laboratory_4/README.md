## Sygnały i potoki
Rodzaje sygnałów: ```SIGINT, SIGQUIT, SIGKILL, SIGTSTP, SIGSTOP, SIGTERM, SIGSEGV, SIGHUP, SIGALARM, SIGCHLD, SIGUSR1, SIGUSR2```

Sygnały czasu rzeczywistego: ```SIGRTMIN, SIGRTMIN+n, SIGRTMAX```

Przydatne polecenia Unix: ```kill, ps```

Przydatne funkcje systemowe: ```kill, raise, sigqueue, signal, sigaction, sigemptyset, sigfillset, sigaddset, sigdelset, sigismember, sigprocmask, sigpending, pause, sigsuspend```

Funkcje do potoków: ```pipe, dup```

## Zadanie 1
- Napisz program wypisujący w pętli nieskończonej aktualną godzinę Po odebraniu sygnału SIGTSTP (CTRL+Z) program zatrzymuje się, wypisując komunikat 
"Oczekuję na CTRL+Z - kontynuacja albo CTR+C - zakonczenie programu". Po ponownym wysłaniu SIGTSTP program powraca do pierwotnego wypisywania.
Program powinien również obsługiwać sygnał SIGINT. Po jego odebraniu program wypisuje komunikat "Odebrano sygnał SIGINT" i kończy działanie. W kodzie programu, do przechwycenia sygnałów użyj zarówno funkcji signal, 
jak i sigaction (np. SIGINT odbierz za pomocą signal, a SIGTSTP za pomocą sigaction).
- Zrealizuj powyższe zadanie, tworząc program potomny, który będzie wywoływał jedną z funkcji z rodziny exec skrypt shellowy zawierający zapętlone systemowe polecenie date. 
Proces macierzysty będzie przychwytywał powyższe sygnały i przekazywał je do procesu potomnego, tj po otrzymaniu SIGTSTP kończy proces potomka, a jeśli ten został wcześniej zakończony, 
tworzy nowy, wznawiając działanie skryptu, a po otrzymaniu SIGINT kończy działanie potomka (jeśli ten jeszcze pracuje) oraz programu.

## Zadanie 2
Napisz program który tworzy proces potomny i wysyła do niego L sygnałów SIGUSR1, a następnie sygnał zakończenia wysyłania SIGUSR2. 
Potomek po otrzymaniu sygnałów SIGUSR1 od rodzica zaczyna je odsyłać do procesu macierzystego, a po otrzymaniu SIGUSR2 kończy pracę.

Proces macierzysty w zależności od argumentu Type (1,2,3) programu wysyła sygnały na trzy różne sposoby:
- SIGUSR1, SIGUSR2 za pomocą funkcji kill
- SIGUSR1, SIGUSR2 za pomocą funkcji kill, z tym, że proces macierzysty wysyła kolejny sygnał dopiero po otrzymaniu potwierdzenia odebrania poprzedniego
- wybrane 2 sygnały czasu rzeczywistego za pomocą kill

Program powinien wypisywać informacje o:
- liczbie wysłanych sygnałów do potomka
- liczbie odebranych sygnałów przez potomka
- liczbie odebranych sygnałów od potomka

Program kończy działanie po zakończeniu pracy potomka albo po otrzymaniu sygnału SIGINT (w tym wypadku od razu wysyła do potomka sygnał SIGUSR2, aby ten zakończył pracę. Wszystkie pozostałe sygnały są blokowane w procesie potomnym).

L i Type są argumentami programu.

## Zadanie 3

Należy rozszerzyć interpreter poleceń z zadania 2 w zestawie 3 (Procesy) tak, by obsługiwał operator pipe - "|". Interpreter czyta kolejne linie z podanego pliku, każda linia ma format

```prog1 arg1 ... argn1 | prog2 arg1 ... argn2 | ... | progN arg1 ... argnN```

Dla takiej linii interpreter powinien uruchomić wszystkie N poleceń w osobnych procesach, zapewniając przy użyciu potoków nienazwanych oraz funkcji ```dup2```, 
by wyjście standardowe procesu k było przekierowane do wejścia standardowego procesu (k+1). 
Można założyć ograniczenie górne na ilość obsługiwanych argumentów oraz ilość połączonych komend w pojedynczym poleceniu (co najmniej 5). 
Po uruchomieniu ciągu programów składających się na pojedczyne polecenie (linijkę) interpreter powinien oczekiwać na zakończenie wszystkich tych programów.

Uwaga: należy użyć ```pipe/fork/exec```, nie ```popen```
