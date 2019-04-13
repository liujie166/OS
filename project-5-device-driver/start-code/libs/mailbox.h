#ifndef INCLUDE_MAIL_BOX_
#define INCLUDE_MAIL_BOX_

#include "queue.h"
#include "lock.h"
#include "sched.h"
#include "syscall.h"
#include "cond.h"
#include "sem.h"
typedef pid_t msg_t;
typedef struct mailbox
{
	int status;
	int num;
	char name[30];
	msg_t msg[10];
	mutex_lock_t lock;
	//condition_t con;
	semaphore_t empty;
	semaphore_t full;
	//queue_t block_queue;
} mailbox_t;


void mbox_init();
mailbox_t *mbox_open(char *);
void mbox_close(mailbox_t *);
void mbox_send(mailbox_t *, void *, int);
void mbox_recv(mailbox_t *, void *, int);

#endif