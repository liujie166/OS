#include "irq.h"
#include "time.h"
#include "sched.h"
#include "string.h"
#include "screen.h"

static void irq_timer()
{
    // TODO clock interrupt handler.
    // scheduler, time counter in here to do, emmmmmm maybe.
    //save cursor and print
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
    if(cause&(0x00008000)){
    	irq_timer();
	}
}

void other_exception_handler()
{
    // TODO other exception handler
    //sorry , I don't want to do anything -_-!
}
