#include "barrier.h"

void do_barrier_init(barrier_t *barrier, int goal)
{
	queue_init(&(barrier->block_queue));
	barrier->goal = goal;
	barrier->count = 0;
	return;
}

void do_barrier_wait(barrier_t *barrier)
{
	(barrier->count)++;
	while(barrier->count<barrier->goal){
		do_block(&(barrier->block_queue));
	}
		do_unblock_all(&(barrier->block_queue));

}