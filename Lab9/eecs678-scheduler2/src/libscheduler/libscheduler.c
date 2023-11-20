/** @file libscheduler.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libscheduler.h"
#include "../libpriqueue/libpriqueue.h"

/**
  Stores information making up a job to be scheduled including any statistics.

  You may need to define some global variables or a struct to store your job queue elements. 
*/
typedef struct _job_t
{
  int job_id;
  int arrival_time;
  int running_time;
  int priority;
  int remaining_time;
  int start_time;
} job_t;

void assign_job_to_core(job_t *job, int core, int time);
int find_preemptive_core(job_t *job);
int max(int a, int b);

int (*compare)(const void *, const void *);
float wait_time, turnaround_time, response_time;
int pre, total_cores, num_jobs, current_time;

job_t **core_used;
priqueue_t job_queue;

int fcfs_compare(const void *a, const void *b)
{
  job_t *job_a = (job_t *)a;
  job_t *job_b = (job_t *)b;
  if (job_a->job_id == job_b->job_id)
  {
    return 0;
  }
  return job_a->arrival_time - job_b->arrival_time;
}

int sjf_compare(const void *a, const void *b)
{
  job_t *job_a = (job_t *)a;
  job_t *job_b = (job_t *)b;

  if (job_a->job_id == job_b->job_id) 
  {
    return 0;
  }

  int diff = job_a->remaining_time - job_b->remaining_time;
  return diff == 0 ? job_a->arrival_time - job_b->arrival_time : diff;
}

int pri_compare(const void *a, const void *b)
{
  job_t *job_a = (job_t *)a;
  job_t *job_b = (job_t *)b;
  if (job_a->job_id == job_b->job_id)
  {
    return 0;
  }
  int diff = job_a->priority - job_b->priority;
  if (diff == 0)
  {
    return job_a->arrival_time - job_b->arrival_time;
  }
  else
  {
    return diff;
  }
}

int rr_compare(const void *a, const void *b)
{
  job_t *job_a = (job_t *)a;
  job_t *job_b = (job_t *)b;
  if (job_a->job_id == job_b->job_id)
  {
    return 0;
  }
  
  return -1;
}

/**
  Initalizes the scheduler.
 
  Assumptions:
    - You may assume this will be the first scheduler function called.
    - You may assume this function will be called once once.
    - You may assume that cores is a positive, non-zero number.
    - You may assume that scheme is a valid scheduling scheme.

  @param cores the number of cores that is available by the scheduler. These cores will be known as core(id=0), core(id=1), ..., core(id=cores-1).
  @param scheme  the scheduling scheme that should be used. This value will be one of the six enum values of scheme_t
*/
void scheduler_start_up(int cores_count, scheme_t scheme)
{
  wait_time = turnaround_time = response_time = 0.0;
  num_jobs = current_time = 0;
  total_cores = cores_count;
  core_used = calloc(cores_count, sizeof(job_t *));

  switch (scheme)
  {
  case FCFS:
    compare = fcfs_compare;
    pre = 0;
    break;
  case SJF:
    compare = sjf_compare;
    pre = 0;
    break;
  case PSJF:
    compare = sjf_compare;
    pre = 1;
    break;
  case PRI:
    compare = pri_compare;
    pre = 0;
    break;
  case PPRI:
    compare = pri_compare;
    pre = 1;
    break;
  case RR:
    compare = rr_compare;
    pre = 0;
    break;
  }
  priqueue_init(&job_queue, compare);
}


/**
  Called when a new job arrives.
 
  If multiple cores are idle, the job should be assigned to the core with the
  lowest id.
  If the job arriving should be scheduled to run during the next
  time cycle, return the zero-based index of the core the job should be
  scheduled on. If another job is already running on the core specified,
  this will preempt the currently running job.
  Assumption:
    - You may assume that every job wil have a unique arrival time.

  @param job_number a globally unique identification number of the job arriving.
  @param time the current time of the simulator.
  @param running_time the total number of time units this job will run before it will be finished.
  @param priority the priority of the job. (The lower the value, the higher the priority.)
  @return index of core job should be scheduled on
  @return -1 if no scheduling changes should be made. 
 
 */
int scheduler_new_job(int job_number, int time, int running_time, int priority)
{
  decrease_remaining_time(time);

  job_t *job = malloc(sizeof(job_t));
  *job = (job_t){job_number, time, running_time, priority, running_time, -1};

  int core = check_idle_core();
  if (core != -1)
  {
    assign_job_to_core(job, core, time);
    return core;
  }

  if (pre && (core = find_preemptive_core(job)) != -1)
  {
    assign_job_to_core(job, core, time);
    return core;
  }

  priqueue_offer(&job_queue, job);
  return -1;
}

void update_metrics(job_t *job, int time)
{
  num_jobs++;
  wait_time += time - job->arrival_time - job->running_time;
  turnaround_time += time - job->arrival_time;
  response_time += job->start_time - job->arrival_time;
}

/**
  Called when a job has completed execution.
 
  The core_id, job_number and time parameters are provided for convenience. You may be able to calculate the values with your own data structure.
  If any job should be scheduled to run on the core free'd up by the
  finished job, return the job_number of the job that should be scheduled to
  run on core core_id.
 
  @param core_id the zero-based index of the core where the job was located.
  @param job_number a globally unique identification number of the job.
  @param time the current time of the simulator.
  @return job_number of the job that should be scheduled to run on core core_id
  @return -1 if core should remain idle.
 */
int scheduler_job_finished(int core_id, int job_number, int time)
{
    decrease_remaining_time(time);

    job_t *finished_job = core_used[core_id];
    if (finished_job != NULL) {
        update_metrics(finished_job, time);
        free(finished_job);
        core_used[core_id] = NULL;
    }

    if (priqueue_size(&job_queue) > 0) {
        job_t *next_job = priqueue_poll(&job_queue);

        if (next_job->start_time == -1) {
            next_job->start_time = time;
        }

        core_used[core_id] = next_job;
        return next_job->job_id;
    }

    return -1;
}



/**
  When the scheme is set to RR, called when the quantum timer has expired
  on a core.
 
  If any job should be scheduled to run on the core free'd up by
  the quantum expiration, return the job_number of the job that should be
  scheduled to run on core core_id.

  @param core_id the zero-based index of the core where the quantum has expired.
  @param time the current time of the simulator. 
  @return job_number of the job that should be scheduled on core cord_id
  @return -1 if core should remain idle
 */
int scheduler_quantum_expired(int core_id, int time)
{
    decrease_remaining_time(time);
    job_t *expired_job = core_used[core_id];

    if (expired_job == NULL) {
        return -1;
    }

    core_used[core_id] = NULL; 

    if (priqueue_size(&job_queue) > 0) {
        priqueue_offer(&job_queue, expired_job);

        job_t *next_job = priqueue_poll(&job_queue);
        if (next_job->start_time == -1) {
            next_job->start_time = time;
        }
        core_used[core_id] = next_job;

        return next_job->job_id;
    } else {
        core_used[core_id] = expired_job;
        return expired_job->job_id;
    }
}


/**
  Returns the average waiting time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average waiting time of all jobs scheduled.
 */
float scheduler_average_waiting_time()
{
  return num_jobs > 0 ? wait_time / num_jobs : 0;
}


/**
  Returns the average turnaround time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average turnaround time of all jobs scheduled.
 */
float scheduler_average_turnaround_time()
{
	return num_jobs > 0 ? turnaround_time / num_jobs : 0;
}


/**
  Returns the average response time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average response time of all jobs scheduled.
 */
float scheduler_average_response_time()
{
	return compare != rr_compare && !pre ? wait_time / num_jobs : response_time / num_jobs;
}


/**
  Free any memory associated with your scheduler.
 
  Assumption:
    - This function will be the last function called in your library.
*/
void scheduler_clean_up()
{
  priqueue_destroy(&job_queue);
  free(core_used);
}


/**
  This function may print out any debugging information you choose. This
  function will be called by the simulator after every call the simulator
  makes to your scheduler.
  In our provided output, we have implemented this function to list the jobs in the order they are to be scheduled. Furthermore, we have also listed the current state of the job (either running on a given core or idle). For example, if we have a non-preemptive algorithm and job(id=4) has began running, job(id=2) arrives with a higher priority, and job(id=1) arrives with a lower priority, the output in our sample output will be:

    2(-1) 4(0) 1(-1)  
  
  This function is not required and will not be graded. You may leave it
  blank if you do not find it useful.
 */
void scheduler_show_queue()
{
  int size = priqueue_size(&job_queue);

  if (size == 0)
  {
    printf("The queue is empty.\n");
    return;
  }

  printf("Current Job Queue Status:\n");
  for (int i = 0; i < size; i++)
  {
    job_t *temp = (job_t *)priqueue_at(&job_queue, i);
    if (temp != NULL) {
      printf("Index: %d, Job Number: %d, Arrival Time: %d, Remaining Time: %d, Priority: %d\n", 
             i, temp->job_id, temp->arrival_time, temp->remaining_time, temp->priority);
    }
  }
}

int check_idle_core()
{
  for (int i = 0; i < total_cores; i++)
  {
    if (core_used[i] == NULL)
    {
      return i;
    }
  }
  return -1;
}


void decrease_remaining_time(int time)
{
  int diff_time = time - current_time;
  for (int i = 0; i < total_cores; i++)
  {
    if (core_used[i])
    {
      core_used[i]->remaining_time = max(0, core_used[i]->remaining_time - diff_time);
    }
  }
  current_time = time;
}

int find_preemptive_core(job_t *job)
{
  int core = -1;
  for (int i = 0; i < total_cores; i++)
  {
    if (core_used[i] && compare(job, core_used[i]) < 0)
    {
      core = i;
      break;
    }
  }
  return core;
}

void assign_job_to_core(job_t *job, int core, int time)
{
  if (core_used[core])
  {
    priqueue_offer(&job_queue, core_used[core]);
  }
  job->start_time = time;
  core_used[core] = job;
}

int max(int a, int b)
{
  return (a > b) ? a : b;
}