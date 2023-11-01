#ifndef BQ_H
#define BQ_H

#include "deque.h"
#include "task.h"

IMPLEMENT_DEQUE_STRUCT(background_job_queue_t, Task);
PROTOTYPE_DEQUE(background_job_queue_t, Task);

#endif