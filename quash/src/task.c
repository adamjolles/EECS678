#include <unistd.h>
#include "task.h"

Task new_Task()
{
    Task job;
    job.is_background = false;
    job.process_queue = new_job_process_queue_t(0);
    for (int i = 0; i < MAX_PIPES; i++)
    {
        job.pipes[i][0] = -1;
        job.pipes[i][1] = -1;
    }
    return job;
}

void push_process_front_to_job(Task *job, pid_t pid)
{
    push_front_job_process_queue_t(&(job->process_queue), pid);
}
void destroy_task(Task *task)
{
    destroy_job_process_queue_t(&(task->process_queue));
    if (task->is_background)
    {
        free(task->cmd);
    }
    for (int i = 0; i < MAX_PIPES; i++)
    {
        if (task->pipes[i][0] != -1)
            close(task->pipes[i][0]);
        if (task->pipes[i][1] != -1)
            close(task->pipes[i][1]);
    }
}

void destroy_job_callback(Task job)
{
    destroy_job_process_queue_t(&(job.process_queue));
    if (job.is_background)
    {
        free(job.cmd);
    }
    for (int i = 0; i < MAX_PIPES; i++)
    {
        if (job.pipes[i][0] != -1)
            close(job.pipes[i][0]);
        if (job.pipes[i][1] != -1)
            close(job.pipes[i][1]);
    }
}
