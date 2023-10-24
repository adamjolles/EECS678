#include <stdio.h>     /* standard I/O functions                         */
#include <stdlib.h>    /* exit                                           */
#include <string.h>    /* memset                                         */
#include <unistd.h>    /* standard unix functions, like getpid()         */
#include <signal.h>    /* signal name macros, and the signal() prototype */

/* first, define the Ctrl-C counter, initialize it with zero. */
int ctrl_c_count = 0;
int got_response = 0;
#define CTRL_C_THRESHOLD  5 

/* the Ctrl-C signal handler */
void catch_int(int sig_num)
{
  /* increase count, and check if threshold was reached */
  ctrl_c_count++;
  if (ctrl_c_count >= CTRL_C_THRESHOLD) {
    char answer[30];

    /* prompt the user to tell us if to really
     * exit or not */
    printf("\nReally exit? [Y/n]: ");
    fflush(stdout);
    alarm(3);
    fgets(answer, sizeof(answer), stdin);
    if (answer[0] == 'n' || answer[0] == 'N') {
      alarm(0);
      printf("\nContinuing...\n");
      fflush(stdout);
      got_response = 0;
      /* 
       * Reset Ctrl-C counter
       */
      ctrl_c_count = 0;
    }
    else {
      printf("\nExiting...\n");
      fflush(stdout);
      got_response = 1;
      exit(0);
    }
  }
}

/* the Ctrl-Z signal handler */
void catch_tstp(int sig_num)
{
  /* print the current Ctrl-C counter */
  printf("\n\nSo far, '%d' Ctrl-C presses were counted\n\n", ctrl_c_count);
  fflush(stdout);
}

/* STEP - 1 (20 points) */
/* Implement alarm handler - following catch_int and catch_tstp signal handlers */
/* If the user DOES NOT RESPOND before the alarm time elapses, the program should exit */
/* If the user RESPONDEDS before the alarm time elapses, the alarm should be cancelled */
//YOUR CODE

void catch_alarm(int sig_num)
{
  if (got_response == 0) {
    printf("\nUser did not respond in time. Exiting...\n");
    fflush(stdout);
    exit(0);
  } 
  else {
    printf("\nAlarm cancelled...\n");
    got_response = 0;
  }
}


int main(int argc, char* argv[])
{
  struct sigaction sa, sa_sigint, sa_sigalrm, sa_sigtstp;
  
  /* STEP - 2 (10 points) */
  /* clear the memory at sa - by filling up the memory location at sa with the value 0 till the size of sa, using the function memset */
  /* type "man memset" on the terminal and take reference from it */
  /* if the sa memory location is reset this way, then no garbage value can create undefined behavior with the signal handlers */
  //YOUR CODE

  memset(&sa_sigint, 0, sizeof(struct sigaction));
  memset(&sa_sigalrm, 0, sizeof(struct sigaction));
  memset(&sa_sigtstp, 0, sizeof(struct sigaction));
  sigset_t mask_set;  /* used to set a signal masking set. */

  /* STEP - 3 (10 points) */
  /* setup mask_set - fill up the mask_set with all the signals to block*/
  //YOUR CODE

  sigfillset(&mask_set);
  
  /* STEP - 4 (10 points) */
  /* ensure in the mask_set that the alarm signal does not get blocked while in another signal handler */
  //YOUR CODE

  sigdelset(&mask_set, SIGALRM);
  
  /* STEP - 5 (20 points) */
  /* set signal handlers for SIGINT, SIGTSTP and SIGALRM */
  //YOUR CODE

  sa_sigint.sa_mask = mask_set;
  sa_sigint.sa_handler = catch_int;
  sigaction(SIGINT, &sa_sigint, NULL);
  sa_sigtstp.sa_mask = mask_set;
  sa_sigtstp.sa_handler = catch_tstp;
  sigaction(SIGTSTP, &sa_sigtstp, NULL);
  sa_sigalrm.sa_mask = mask_set;
  sa_sigalrm.sa_handler = catch_alarm;
  sigaction(SIGALRM, &sa_sigalrm, NULL);
  
  /* STEP - 6 (10 points) */
  /* ensure that the program keeps running to receive the signals */
  //YOUR CODE

  while(1){
    pause();
  }

  return 0;
}
