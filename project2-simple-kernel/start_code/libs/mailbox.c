#include "string.h"
#include "mailbox.h"

#define MAX_NUM_BOX 32

static mailbox_t mboxs[MAX_NUM_BOX];

void mbox_init()
{

	int i;

	for(i=0;i<MAX_NUM_BOX;i++){
		mboxs[i].status = 0;
		mboxs[i].num = 0;
		strcpy(mboxs[i].name,"hello_os");
		//do_condition_init(&(mboxs[i].con));
		do_mutex_lock_init(&(mboxs[i].lock));
		do_semaphore_init(&(mboxs[i].empty),3);
		do_semaphore_init(&(mboxs[i].full),0);
	}
	
}

mailbox_t *mbox_open(char *name)
{
	int i;
	
	for(i=0;i<MAX_NUM_BOX;i++){
		if((strcmp(mboxs[i].name, name)==0)&&(mboxs[i].status==1)){
			return &mboxs[i];
		}
	}
	for(i=0;i<MAX_NUM_BOX;i++){
		if(mboxs[i].status==0){
			mutex_lock_acquire(&(mboxs[i].lock));
			strcpy(mboxs[i].name, name);
			mboxs[i].status=1;
			mutex_lock_release(&(mboxs[i].lock));
			return &mboxs[i];
		}
	}
}

void mbox_close(mailbox_t *mailbox)
{
	//mutex_lock_acquire(&(mailbox->lock));
	//condition_broadcast(&(mailbox->con));
	//mutex_lock_release(&(mailbox->lock));
}

void mbox_send(mailbox_t *mailbox, void *msg, int msg_length)
{
	//condition_broadcast(&(mailbox->con));
	semaphore_down(&(mailbox->empty));
	mutex_lock_acquire(&(mailbox->lock));
	//while(mailbox->num == 3){
	//	condition_wait(&(mailbox->lock),&(mailbox->con));
	//}
	//mutex_lock_acquire(&(mailbox->lock));
	mailbox->msg[(mailbox->num)++] = *(msg_t*)msg;
	mutex_lock_release(&(mailbox->lock));
	semaphore_up(&(mailbox->full));
	return;

}

void mbox_recv(mailbox_t *mailbox, void *msg, int msg_length)
{
	semaphore_down(&(mailbox->full));
	//condition_broadcast(&(mailbox->con));
	mutex_lock_acquire(&(mailbox->lock));
	//while(mailbox->num == 0){
		//condition_wait(&(mailbox->lock),&(mailbox->con));
	//}
	//mutex_lock_acquire(&(mailbox->lock));
	*(msg_t*)msg = mailbox->msg[--(mailbox->num)];
	mutex_lock_release(&(mailbox->lock));
	semaphore_up(&(mailbox->empty));
	return;
}