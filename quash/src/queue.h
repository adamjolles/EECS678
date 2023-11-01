#ifndef QUEUE_H
#define QUEUE_H
#include "deque.h"
IMPLEMENT_DEQUE_STRUCT(job_process_queue_t, pid_t);
PROTOTYPE_DEQUE(job_process_queue_t, pid_t);
#endif