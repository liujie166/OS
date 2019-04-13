#include "lock.h"
#include "sched.h"
#include "syscall.h"


void spin_lock_init(spin_lock_t *lock)
{
    lock->status = UNLOCKED;
}

void spin_lock_acquire(spin_lock_t *lock)
{
    while (LOCKED == lock->status)
    {
    };
    lock->status = LOCKED;
}

void spin_lock_release(spin_lock_t *lock)
{
    lock->status = UNLOCKED;
}

void do_mutex_lock_init(mutex_lock_t *lock)
{
	lock->status = UNLOCKED;
	queue_init(&(lock->block_queue));
}

void do_mutex_lock_acquire(mutex_lock_t *lock)
{
	while(LOCKED == lock->status)
	{
		do_block(&(lock->block_queue));
	}
		lock->status = LOCKED;
		if(current_running->lock1==NULL)
			current_running->lock1 = lock;
		else
			current_running->lock2 = lock;
}

void do_mutex_lock_release(mutex_lock_t *lock)
{
	
	lock->status = UNLOCKED;
	do_unblock_all(&(lock->block_queue));
}
