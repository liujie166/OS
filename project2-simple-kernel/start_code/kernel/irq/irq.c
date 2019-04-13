#include "irq.h"
#include "time.h"
#include "sched.h"
#include "string.h"
#include "screen.h"
#include "mac.h"
static void irq_timer()
{
    // TODO clock interrupt handler.
    // scheduler, time counter in here to do, emmmmmm maybe.
    //save cursor and print
    /*if(0==queue_is_empty(&recv_block_queue)){
        vt100_move_cursor(2, 3);
        printk("> [RECV TASK] still waiting %dth package.\n",package_cnt+1);
        if(0x00000000==(r_desc[package_cnt].tdes0&0x80000000)){
            package_cnt++;
        }
        if(package_cnt==64){
            do_unblock_one(&recv_block_queue);
        }
    }*/
    save_cursor();
    screen_reflush();
    //get time_elapsed and rst compare&count
    get_ticks(); 
    reset_com_and_cnt();
    //switch kernel context and restore cursor
    do_scheduler();
    restore_cursor();
}
void interrupt_helper(uint32_t status, uint32_t cause)
{
    // TODO interrupt handler.
    // Leve3 exception Handler.
    // read CP0 register to analyze the type of interrupt.
    uint32_t int1_sr;
    if(cause&(0x00008000)){
    	irq_timer();
	}
    if(cause&(0x00000800)){
        int1_sr=read_register((uint32_t)0xbfd01058,(uint32_t)0x0);
        if(int1_sr&(0x00000008)){
            //printk("dakda\n");
            irq_mac();
            
        }
        
    }
}

void other_exception_handler()
{
    // TODO other exception handler
    //sorry , I don't want to do anything -_-!
}
