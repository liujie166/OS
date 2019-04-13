#include "lock.h"
#include "time.h"
#include "stdio.h"
#include "sched.h"
#include "queue.h"
#include "screen.h"

pcb_t pcb[NUM_MAX_TASK];

/* current running task PCB */
pcb_t *current_running;

/* global process id */
pid_t process_id = 0;
int running_pid[NUM_MAX_TASK];
static void check_sleeping()
{
    pcb_t* sleep_task;
    sleep_task=queue_dequeue(&block_queue);
    if(get_timer()-sleep_task->begin_time<sleep_task->sleep_time)
        queue_push(&block_queue,sleep_task);
    else{
        sleep_task->sleep_time = 0;
        sleep_task->status = TASK_READY;
        sleep_task->ready_queue =&h_ready_queue;
        queue_push(&h_ready_queue,sleep_task);
    }
}

void scheduler(void)
{
    // TODO schedule
    // Modify the current_running pointer.

    //check sleep task and maybe wake up
    if(queue_is_empty(&block_queue)==0){
       check_sleeping(); 
    }

    //push task into block queue
    if(current_running->status==TASK_BLOCKED){
        //if sleep time is not zero,push into block queue of sleep task
        if(current_running->sleep_time!=0){
        queue_push(&block_queue,current_running);
        current_running->begin_time=get_timer();
        }
        //if task fail to get lock,push into block queue of mutex 
        else if(current_running->sleep_time==0){
        queue_push(current_running->block_queue,current_running);
        }
    }
    //push running task into the queue of different priority 
    if(current_running->status==TASK_RUNNING){
        current_running->status = TASK_READY;
        if(current_running->priority == HIGH){
            current_running->priority == MEDIUM;
            current_running->ready_queue = &m_ready_queue;
            queue_push(&m_ready_queue,current_running);
        }
        if(current_running->priority == MEDIUM){
            current_running->priority == LOW;
            current_running->ready_queue = &l_ready_queue;
            queue_push(&l_ready_queue,current_running);
        }
        if(current_running->priority == LOW){
            current_running->ready_queue = &l_ready_queue;
            queue_push(&l_ready_queue,current_running);
        }
    }
    //select a higher priority task and running
    while(1){
        if(queue_is_empty(&h_ready_queue)==0){
            current_running=queue_dequeue(&h_ready_queue);
            current_running->status = TASK_RUNNING;
            break;
        }
        else if(queue_is_empty(&m_ready_queue)==0){
            current_running=queue_dequeue(&m_ready_queue);
            current_running->status = TASK_RUNNING;
            break;
        }
        else if(queue_is_empty(&l_ready_queue)==0){
            current_running=queue_dequeue(&l_ready_queue);
            current_running->status = TASK_RUNNING;
            break;
        } 
    }
}

void do_sleep(uint32_t sleep_time)
{
    // TODO sleep(seconds)
    current_running->block_queue = &block_queue;
    current_running->status = TASK_BLOCKED; 
    current_running->sleep_time = sleep_time;

    //like irq_timer();
    save_cursor();
    screen_reflush();
    get_ticks();
    reset_com_and_cnt();
    do_scheduler();
    restore_cursor();
}

void do_block(queue_t *queue)
{
    // block the current_running task into the queue

    current_running->status = TASK_BLOCKED; 
    current_running->block_queue = queue;

    //like irq_timer();
    save_cursor();
    screen_reflush();
    get_ticks();
    reset_com_and_cnt();
    do_scheduler();
    restore_cursor();

}
void do_unblock_one(queue_t *queue)
{
    // unblock the head task from the queue

    pcb_t* unblock_task;
    if(queue_is_empty(queue)==0){
        unblock_task = queue_dequeue(queue);
        unblock_task->status = TASK_READY; 
        unblock_task->block_queue = NULL;
        unblock_task->ready_queue = &h_ready_queue;
        queue_push(&h_ready_queue,unblock_task);
    }

}

void do_unblock_all(queue_t *queue)
{
    // unblock all task in the queue
    pcb_t* unblock_task;
    while(queue_is_empty(queue)==0){
        unblock_task = queue_dequeue(queue);
        unblock_task->status = TASK_READY;
        unblock_task->block_queue = NULL;
        unblock_task->ready_queue = &h_ready_queue;
        queue_push(&h_ready_queue,unblock_task); 
    }
}

void process_show()
{
    int i,j;
    for(i=0,j=0;i<NUM_MAX_TASK;i++){
        if((pcb[i].status == TASK_RUNNING)||(pcb[i].status == TASK_READY)||(pcb[i].status == TASK_BLOCKED)){
            running_pid[j] = pcb[i].pid;
            j++;
        }
    }
    running_pid[j] = -1;
}

void process_spawn(task_info_t* task_info)
{
    int i;
        for(i=0;pcb[i].status!=TASK_EXITED&&pcb[i].status!=TASK_CREATE;i++);
            queue_init(&(pcb[i].wait_queue));
            pcb[i].sleep_time = 0;
            pcb[i].lock1 = NULL;
            pcb[i].lock2 = NULL;
            pcb[i].kernel_context.cp0_status = initial_cp0_status; 
            pcb[i].user_context.regs[29] = pcb[i].user_stack_top;    
            pcb[i].kernel_context.regs[29] = pcb[i].kernel_stack_top;
            pcb[i].kernel_context.regs[31] = (uint32_t)task_start;
            pcb[i].status = TASK_READY;
            pcb[i].type = task_info->type;
            pcb[i].user_context.cp0_epc = (uint32_t)task_info->entry_point;
            pcb[i].priority = HIGH;
            pcb[i].ready_queue = &h_ready_queue;
            pcb[i].pid = process_id++;
            //pcb[i].user_context.cp0_entryhi = pcb[i].user_stack_top | (0x000000ff & pcb[i].pid);
            queue_push(&h_ready_queue,&pcb[i]);
            return;

}
void process_exit()
{
    current_running->status = TASK_EXITED;
    do_unblock_one(&(current_running->wait_queue));
    
    if(current_running->lock1!=NULL)
        do_mutex_lock_release(current_running->lock1);
    if(current_running->lock2!=NULL)
        do_mutex_lock_release(current_running->lock2);

    screen_reflush();
    get_ticks();
    reset_com_and_cnt();
    do_scheduler();

}
void process_kill(pid_t pid)
{
    int i=0;
    while(i<NUM_MAX_TASK) {
        if(pcb[i].pid==pid)
            break;
        i++;
    }

    do_unblock_all(&(pcb[i].wait_queue));

    if(pcb[i].status == TASK_READY) {
        queue_remove(pcb[i].ready_queue,&pcb[i]);
    }
    if(pcb[i].status == TASK_BLOCKED) {
        queue_remove(pcb[i].block_queue,&pcb[i]);
    }
    pcb[i].status = TASK_EXITED;
    
    

    if(pcb[i].lock1!=NULL)
        do_mutex_lock_release(pcb[i].lock1);
    if(pcb[i].lock2!=NULL)
        do_mutex_lock_release(pcb[i].lock2);
}
void wait_exit(pid_t pid)
{
    int i=0;
    while(i<NUM_MAX_TASK) {
        if(pcb[i].pid==pid)
            break;
        i++;
    }
    do_block(&(pcb[i].wait_queue));
}

void let_wait(pid_t pid)
{
    int i=0;
    while(i<NUM_MAX_TASK) {
        if(pcb[i].pid==pid)
            break;
        i++;
    }
    queue_remove(pcb[i].ready_queue,&pcb[i]);
    queue_push(&(current_running->wait_queue),&pcb[i]);
}
void get_pid(pid_t* mypid)
{
    *mypid = current_running->pid;
}