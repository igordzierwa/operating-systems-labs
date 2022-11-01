#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

int to_child_signal;
int to_parent_signal;
int ending_signal;

pid_t child_pid;


int to_child_signals_number;
int to_parent_signals_number;
int ch_received_signals;


int flag;


//Function to handle CHILD
void ch_sig_tc(int signum) {

    ch_received_signals++;
    kill(getppid(), to_parent_signal);
}



void ch_sig_end(int signum) {

    fprintf(stdout, "Dziecko: odebrałem od ojca %d sygnałów\n", ch_received_signals);
    exit(1);
}


//FUNCTION to handle PARENT
void sig_int(int signum) {

    printf("Received signal SIGINT\n");

    kill(child_pid, ending_signal);

    exit(1);
}



void sig_tp(int signum) {

    flag = 1;
    to_parent_signals_number++;
}


//HANDLERS
void set_parent_handlers() {


    struct sigaction s_act;
    s_act.sa_handler = sig_int;
    sigfillset(&s_act.sa_mask);
    if(sigaction(SIGINT, &s_act, NULL) == -1) {

        perror("Can not set SIGINThandler");
        exit(1);
    }


    struct sigaction s_act2;
    s_act2.sa_handler = sig_tp;
    sigfillset(&s_act2.sa_mask);
    if(sigaction(to_parent_signal, &s_act2, NULL) == -1) {

        perror("Can not set to_parent_signal handler");
        exit(1);
    }
}



void set_child_handlers() {


    struct sigaction s_act;
    s_act.sa_handler = ch_sig_end;
    sigfillset(&s_act.sa_mask);
    if(sigaction(ending_signal, &s_act, NULL) == -1) {

        perror("Can not set ending_signal handler");
        exit(1);
    }

    struct sigaction s_act2;
    s_act2.sa_handler = ch_sig_tc;
    sigfillset(&s_act2.sa_mask);
    sigdelset(&s_act2.sa_mask, ending_signal);
    if(sigaction(to_child_signal, &s_act2, NULL) == -1) {

        perror("Can not set to_child_signal handler");
        exit(1);
    }
}


void child_proc_fun() {

    ch_received_signals = 0;
    set_child_handlers();

    sigset_t child_mask;
    sigfillset(&child_mask);
    sigdelset(&child_mask, to_child_signal);
    sigdelset(&child_mask, ending_signal);

    while(1)
        sigsuspend(&child_mask);
}



void create_child() {

    sigset_t new_mask, prev_mask;
    sigfillset(&new_mask);

    if(sigprocmask(SIG_SETMASK, &new_mask, &prev_mask) == -1) {

        perror("Can not set sigmask");
        exit(1);
    }

    pid_t id = fork();

    if(id == 0)
        child_proc_fun();

    child_pid = id;

    if(sigprocmask(SIG_SETMASK, &prev_mask, NULL) == -1) {

        perror("Can not set sigmask");
        exit(1);
    }
}

int main(int argc, char** argv) {

    if(argc != 3) {
        printf("Wrong number of arguments\n");
        exit(1);
    }

    int signals_to_send = atoi(argv[1]);
    int option = atoi(argv[2]);

    to_child_signals_number = 0;
    to_parent_signals_number = 0;

    if(option == 1) {

        to_child_signal = SIGUSR1;
        to_parent_signal = SIGUSR1;
        ending_signal = SIGUSR2;


        create_child();
        set_parent_handlers();

        for(int i = 0; i < signals_to_send; i++) {
            kill(child_pid, to_child_signal);
            to_child_signals_number++;
        }

        kill(child_pid, ending_signal);
    }

    else if(option == 2) {

        to_child_signal = SIGUSR1;
        to_parent_signal = SIGUSR1;
        ending_signal = SIGUSR2;

        create_child();
        set_parent_handlers();

        sigset_t new_mask, prev_mask;
        sigemptyset(&new_mask);
        if(sigprocmask(SIG_BLOCK, &new_mask, &prev_mask) == -1) {

            perror("Can not set sigmask");
            exit(1);
        }

        for(int i = 0; i < signals_to_send; i++) {

            flag = 0;

            kill(child_pid, to_child_signal);
            to_child_signals_number++;

            while(flag == 0)
                sigsuspend(&prev_mask);
        }

        kill(child_pid, ending_signal);
    }

    // else if(option == 3) {
    //
    //     to_child_signal = SIGRTMIN;
    //     to_parent_signal = SIGRTMIN;
    //     ending_signal = SIGRTMIN+1;
    //
    //     create_child();
    //     set_parent_handlers();
    //
    //     for(int i = 0; i < signals_to_send; i++) {
    //         kill(child_pid, to_child_signal);
    //         to_child_signals_number++;
    //     }
    //
    //     kill(child_pid, ending_signal);
    // }

    else {
        printf("wrong argument format\n");
    }

    printf("Ojciec: wysłałem do dziedka %d sygnałów\n", to_child_signals_number);
    printf("Ojciec: odebrałem od dziecka %d sygnałów\n", to_parent_signals_number);
}
