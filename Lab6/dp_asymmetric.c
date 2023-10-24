#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>

/*
 * Some handy constants. Number of philosophers and chopsticks lets us
 * parameterize the number of concurrent threads and shared
 * resources. The maximum thinking and eating periods let us tune
 * relative periods of holding or not holding a resource. The MAX_BUF
 * and column_width constants help with creating output that makes
 * sense..
 */
#define NUM_PHILS                     5
#define NUM_CHOPS             NUM_PHILS
#define MAX_PHIL_THINK_PERIOD      1000
#define MAX_PHIL_EAT_PERIOD         100
#define MAX_BUF                     256
#define STATS_WIDTH                  16
#define COLUMN_WIDTH                 18
#define ACCOUNTING_PERIOD             5
#define ITERATION_LIMIT              10

/*
 * Structure defining a philosopher and any state we need to know
 * about to print out interesting data, or implement different
 * solutions.
 */
typedef struct {
  int            id;           /* Int ID number assigned by
                                  set_table() */
  pthread_cond_t can_eat;      /* Condition var used in a WAITER SOLUTION */
  int            prog;         /* Progress during current main()
                                  accounting period */
  int            prog_total;   /* Total progress across all
                                  sessions  */
  pthread_t      thread;       /* Thread structure for this
                                  philosopher */
} philosopher;

/* GLOBALS */
philosopher Diners[NUM_PHILS];
int         Stop = 0;

/* Each chopstick is shared between two philosophers */
static pthread_mutex_t chopstick[NUM_CHOPS];

philosopher *left_phil(philosopher *p)
{
  return &Diners[(p->id == (NUM_PHILS - 1) ? 0 : (p->id) + 1)];
}

philosopher *right_phil(philosopher *p)
{
  return &Diners[(p->id == 0 ? (NUM_PHILS - 1) : (p->id) - 1)];
}

pthread_mutex_t *left_chop(philosopher *p)
{
  return &chopstick[p->id];
}

pthread_mutex_t *right_chop(philosopher *p)
{
  return &chopstick[(p->id == 0 ? NUM_CHOPS - 1 : (p->id) - 1)];
}

void think_one_thought()
{
  int i;
  i = 0;
  i++;
}

void eat_one_mouthful()
{
  int i;
  i = 0;
  i++;
}

static void *dp_thread(void *arg)
{
  int eat_rnd;
  int i;
  int id;
  philosopher *me;
  int think_rnd;

  me = (philosopher *)arg;
  id = me->id;

  while (!Stop)
  {
    think_rnd = (rand() % MAX_PHIL_THINK_PERIOD);
    eat_rnd = (rand() % MAX_PHIL_EAT_PERIOD);

    for (i = 0; i < think_rnd; i++)
    {
      think_one_thought();
    }

    // ASYMMETRY added to avoid deadlock
    if (id % 2 == 0)
    {
      pthread_mutex_lock(left_chop(me));
      pthread_mutex_lock(right_chop(me));
    }
    else
    {
      pthread_mutex_lock(right_chop(me));
      pthread_mutex_lock(left_chop(me));
    }

    for (i = 0; i < eat_rnd; i++)
    {
      eat_one_mouthful();
    }

    pthread_mutex_unlock(left_chop(me));
    pthread_mutex_unlock(right_chop(me));

    me->prog++;
    me->prog_total++;
  }

  return NULL;
}

void set_table()
{
  int i;

  for (i = 0; i < NUM_CHOPS; i++)
  {
    pthread_mutex_init(&chopstick[i], NULL);
  }

  for (i = 0; i < NUM_PHILS; i++)
  {
    Diners[i].prog = 0;
    Diners[i].prog_total = 0;
    Diners[i].id = i;
  }

  for (i = 0; i < NUM_PHILS; i++)
  {
    pthread_create(&(Diners[i].thread), NULL, dp_thread, &Diners[i]);
  }
}

void print_progress()
{
  int i;
  int j;
  char buf[MAX_BUF];

  for (i = 0; i < NUM_PHILS;)
  {
    for (j = 0; j < 4; j++)
    {
      if (i == NUM_PHILS)
      {
        printf("\n");
        goto out;
      }

      sprintf(buf, "%d/%d", Diners[i].prog, Diners[i].prog_total);
      printf("p%d=%*s   ", i, STATS_WIDTH, buf);
      i++;
    }

    if (i == NUM_PHILS)
    {
      printf("\n");
      break;
    }

    sprintf(buf, "%d/%d", Diners[i].prog, Diners[i].prog_total);
    printf("p%d=%*s\n", i, STATS_WIDTH, buf);
    i++;
  }

out:
/*
  * Add an extra new line for a blank between data for each
  * accounting period.
*/
  printf("\n");
}

int main(int argc, char **argv)
{
  int i;
  int deadlock;
  int iter;

  iter = 0;

  /*
   * Randomly seed the random number generator used to control how
   * long philosophers eat and think.
   */
  srand(time(NULL));

  /*
   * Set the table means create the chopsticks and the philosophers.
   * Print out a header for the periodic updates on Philosopher state.
   */
  set_table();
  printf("\n");
  printf("Dining Philosophers Update every %d seconds\n", ACCOUNTING_PERIOD);
  printf("-------------------------------------------\n");

  do {
    /*
     * Reset the philosophers eating progress to 0. If the
     * philosopher is making progress, the philosopher will
     * increment it.
     */
    for (i = 0; i < NUM_PHILS; i++)
    {
      Diners[i].prog = 0;
    }
 /*
  * Let the philosophers do some thinking and eating over a period
  * of ACCOUNTING_PERIOD seconds, which is a *long* time compared
  * to the time-scale of a philosopher thread, so *some* progress
  * should be made by each in this waiting time, unless deadlock
  * has occurred.
*/
    sleep(ACCOUNTING_PERIOD);
/*
  * Check for deadlock (i.e. none of the philosophers have
  * made progress in 5 seconds)
*/
    deadlock = 1;
    for (i = 0; i < NUM_PHILS; i++)
    {
      if (Diners[i].prog)
      {
        deadlock = 0;
      }
    }

    print_progress();
    iter++;
  } while (!deadlock && iter < ITERATION_LIMIT);
  Stop = 1;
  if (deadlock)
  {
    printf("Deadlock Detected\n");
  }
  else
  {
    printf("Finished without Deadlock\n");
  }
/*
  * Wait for philosophers to finish
*/
  for (i = 0; i < NUM_PHILS; i++)
  {
    pthread_join(Diners[i].thread, NULL);
  }

  return 0;
}
