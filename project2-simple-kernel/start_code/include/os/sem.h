#ifndef INCLUDE_SEM_H_
#define INCLUDE_SEM_H_

#include "queue.h"
#include "type.h"
#include "sched.h"
typedef struct semaphore
{
	//mutex_lock_t lock;
	int count;
	queue_t block_queue;
} semaphore_t;

void do_semaphore_init(semaphore_t *, int);
void do_semaphore_up(semaphore_t *);
void do_semaphore_down(semaphore_t *);

#endif