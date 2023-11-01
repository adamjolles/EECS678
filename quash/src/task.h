#ifndef TASK_H
#define TASK_H

#include "queue.h"

#define MAX_PIPES 10
typedef int job_id_t;

typedef struct Task
{
    job_process_queue_t process_queue;
    int pipes[MAX_PIPES][2];
    bool is_background;
    job_id_t t_id;
    char *cmd;
} Task;
Task new_Task();

void push_process_front_to_job(Task *job, pid_t pid);

void destroy_task(Task *job);
void destroy_task_callback(Task job);

#endif