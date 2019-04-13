#include "sem.h"
#include "stdio.h"

void do_semaphore_init(semaphore_t *s, int val)
{
	queue_init(&(s->block_queue));
	s->count = val;
	return;
}

void do_semaphore_up(semaphore_t *s)
{
	if(queue_is_empty(&(s->block_queue))==0)
		do_unblock_one(&(s->block_queue));
	s->count++;	
	return;
}

void do_semaphore_down(semaphore_t *s)
{
	
	while((s->count)<=0){
		do_block(&(s->block_queue));
	}
	s->count--;
	return;
}