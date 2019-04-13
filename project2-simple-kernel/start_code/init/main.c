/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 *               Author : Han Shukai (email : hanshukai@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *         The kernel's entry, where most of the initialization work is done.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this 
 * software and associated documentation files (the "Software"), to deal in the Software 
 * without restriction, including without limitation the rights to use, copy, modify, 
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit 
 * persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE. 
 * 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * */

#include "irq.h"
#include "test.h"
#include "stdio.h"
#include "sched.h"
#include "screen.h"
#include "common.h"
#include "syscall.h"
#include "string.h"
#include "sem.h"
#include "cond.h"
#include "mailbox.h"
#include "mm.h"
#include "mac.h"
#include "fs.h"
queue_t l_ready_queue;
queue_t m_ready_queue;
queue_t h_ready_queue;
queue_t block_queue;
queue_t irq_block_queue;
mutex_lock_t screen_lock;
uint32_t initial_cp0_status = 0x00008001;

static void init_pcb()
{
	int i;
	//init stack top
	uint32_t user_stack_top=0xa1f00000;
	uint32_t kernel_stack_top=0xa0f00000;
	//init queue
	queue_init(&l_ready_queue);
	queue_init(&m_ready_queue);
	queue_init(&h_ready_queue);
	queue_init(&block_queue);
	queue_init(&irq_block_queue);
    do_mutex_lock_init(&screen_lock);

	//init an idling task
	pcb[0].pid = process_id++;
	pcb[0].status = TASK_RUNNING;

	pcb[0].sleep_time = 0;
	pcb[0].kernel_context.regs[29] = kernel_stack_top;
	pcb[0].user_context.regs[29] = user_stack_top;
	//pcb[0].user_context.cp0_entryhi = user_stack_top | (0x000000ff & pcb[0].pid);
	//set_entryhi(pcb[0].pid);
	user_stack_top+=0x00010000;
	kernel_stack_top+=0x00010000;
	//init test task
	for(i=1;i<NUM_MAX_TASK;i++){
		queue_init(&(pcb[i].wait_queue));
		pcb[i].status = TASK_CREATE;
	    pcb[i].user_stack_top = user_stack_top;
	    pcb[i].kernel_stack_top = kernel_stack_top;   
	    user_stack_top = user_stack_top+0x00010000;
	    kernel_stack_top = kernel_stack_top+0x00010000;
	}
	    //memset(&(pcb[1].kernel_context),0,sizeof(regs_context_t));
        //memset(&(pcb[1].user_context),0,sizeof(regs_context_t));
        pcb[1].kernel_context.cp0_status = initial_cp0_status;    
	    pcb[1].sleep_time = 0;
		pcb[1].kernel_context.regs[29] = pcb[1].kernel_stack_top;
		pcb[1].user_context.regs[29] = pcb[1].user_stack_top;
		pcb[1].kernel_context.regs[31] = (uint32_t)task_start;
		pcb[1].status = TASK_READY;
        pcb[1].type = USER_PROCESS;
        pcb[1].user_context.cp0_epc = (uint32_t)test_shell;
        pcb[1].priority = HIGH;
        pcb[1].pid = process_id++;
	    //pcb[1].user_context.cp0_entryhi = user_stack_top | (0x000000ff & pcb[1].pid);

        //set_entryhi();
        queue_push(&h_ready_queue,&pcb[1]);
        pcb[1].ready_queue = &h_ready_queue;
        current_running = &pcb[0];
        //mbox_init();
}

void exception_helper(uint32_t cause,uint32_t badva)
{
	uint32_t cause_excode = cause & 0x7c;
	
	if(cause_excode == 0x00){
		//vt100_move_cursor(0,0);
		//printk("int");
		handle_int();
	}
	else if(cause_excode == 0x20){
		//vt100_move_cursor(0,0);
		//printk("sys");
		handle_syscall();
	}
	else{
		vt100_move_cursor(0,0);
		printk("cp0_cause:0x%x,cp0_badvaddr:0x%x\n",cause,badva);
		//printk("cause is 0x%x\n pc is 0x%x\n",cause,badva);
		handle_other();
	}
}

static void init_exception()
{
	uint32_t cp0_status;
	cp0_status = get_cp0_status()|0x0000fc00;

	// 1. Get CP0_STATUS
	// 2. Disable all interrupt
	// 3. Copy the level 2 exception handling code to 0x80000180
    memcpy((void*)0x80000000,TLBexception_handler_entry,TLBexception_handler_end-TLBexception_handler_begin);
	memcpy((void*)0x80000180,exception_handler_entry,exception_handler_end-exception_handler_begin);
	// 4. reset CP0_COMPARE & CP0_COUNT register
	reset_com_and_cnt();
	set_cp0_status(cp0_status);
}

static void init_syscall(void)
{
	// init system call table.
	syscall[SYSCALL_SLEEP]=&do_sleep;
	syscall[SYSCALL_BLOCK]=&do_block;
	syscall[SYSCALL_UNBLOCK_ONE]=&do_unblock_one;
	syscall[SYSCALL_UNBLOCK_ALL]=&do_unblock_all;
	syscall[SYSCALL_WRITE]=&screen_write;
	syscall[SYSCALL_REFLUSH]=&screen_reflush;
	syscall[SYSCALL_CURSOR]=&screen_move_cursor;
	syscall[SYSCALL_MUTEX_LOCK_INIT]=&do_mutex_lock_init;
	syscall[SYSCALL_MUTEX_LOCK_ACQUIRE]=&do_mutex_lock_acquire;
	syscall[SYSCALL_MUTEX_LOCK_RELEASE]=&do_mutex_lock_release;
	syscall[SYSCALL_CLEAR]=&screen_clear;
	syscall[SYSCALL_PS]=&process_show;
	syscall[SYSCALL_EXIT]=&process_exit;
	syscall[SYSCALL_SPAWN]=&process_spawn;
	syscall[SYSCALL_WAITPID]=&wait_exit;
	syscall[SYSCALL_KILL]=&process_kill;
	syscall[SYSCALL_GETPID]=&get_pid;
	syscall[SYSCALL_SEMAPHORE_INIT]=&do_semaphore_init;
	syscall[SYSCALL_SEMAPHORE_UP]=&do_semaphore_up;
	syscall[SYSCALL_SEMAPHORE_DOWN]=&do_semaphore_down;
	syscall[SYSCALL_CONDITION_INIT]=&do_condition_init;
	syscall[SYSCALL_CONDITION_WAIT]=&do_condition_wait;
	syscall[SYSCALL_CONDITION_SIGNAL]=&do_condition_signal;
	syscall[SYSCALL_CONDITION_BROADCAST]=&do_condition_broadcast;
	syscall[SYSCALL_BARRIER_INIT]=&do_barrier_init;
	syscall[SYSCALL_BARRIER_WAIT]=&do_barrier_wait;
	syscall[SYSCALL_WAIT]=&let_wait;
	syscall[SYSCALL_INIT_MAC]=&do_init_mac;
	syscall[SYSCALL_NET_SEND]=&do_net_send;
	syscall[SYSCALL_NET_RECV]=&do_net_recv;
	syscall[SYSCALL_WAIT_RECV_PACKAGE]=&do_wait_recv_package;
	syscall[SYSCALL_INIT_FS]=&do_init_fs;
	syscall[SYSCALL_FS_STATE]=&show_fs_state;
	syscall[SYSCALL_MKDIR]=&do_mkdir;
	syscall[SYSCALL_LS]=&do_ls;
	syscall[SYSCALL_RMDIR]=&do_rmdir;
	syscall[SYSCALL_CD]=&do_cd;
	syscall[SYSCALL_TOUCH]=&do_touch;
	syscall[SYSCALL_CAT] = &do_cat;
	syscall[SYSCALL_FOPEN] = &do_openfile;
	syscall[SYSCALL_FREAD] = &do_readfile;
	syscall[SYSCALL_FWRITE] = &do_writefile;
	syscall[SYSCALL_FCLOSE] = &do_closefile;
}
void init_pagetabe(){
	//o_init_pagetable();
}
// jump from bootloader.
// The beginning of everything >_< ~~~~~~~~~~~~~~
void __attribute__((section(".entry_function"))) _start(void)
{
	// Close the cache, no longer refresh the cache 
	// when making the exception vector entry copy
	printk("MINI_OS_1.0!\n");
	asm_start();

	// init interrupt (^_^)
	init_exception();
	printk("> [INIT] Interrupt processing initialization succeeded.\n");

	// init system call table (0_0)
	init_syscall();
	printk("> [INIT] System call initialized successfully.\n");

	// init Process Control Block (-_-!)
	init_pcb();
	printk("> [INIT] PCB initialization succeeded.\n");

	// init screen (QAQ)
	init_screen();
	//printk("> [INIT] SCREEN initialization succeeded.\n");
    //init TLB
    init_pagetabe();
	// TODO Enable interrupt
	enable_interrupt();

	while (1)
	{
		// (QAQQQQQQQQQQQ)
		// If you do non-preemptive scheduling, you need to use it to surrender control
		//do_scheduler();
	};
	return;
}
void save_cursor()
{
	current_running->cursor_x = screen_cursor_x;
	current_running->cursor_y = screen_cursor_y;
	return;
}
void restore_cursor()
{
	screen_cursor_x = current_running->cursor_x;
	screen_cursor_y = current_running->cursor_y;
	return;
}
