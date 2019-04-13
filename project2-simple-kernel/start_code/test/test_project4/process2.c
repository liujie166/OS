#include "sched.h"
#include "stdio.h"
#include "syscall.h"
#include "time.h"
#include "screen.h"
#include "test4.h"

#define RW_TIMES 1
int rand()
{	
	int current_time = get_timer();
	return current_time % 100000;
}

/*static void disable_interrupt()
{
    uint32_t cp0_status = get_cp0_status();
    cp0_status &= 0xfffffffe;
    set_cp0_status(cp0_status);
}

static void enable_interrupt()
{
    uint32_t cp0_status = get_cp0_status();
    cp0_status |= 0x01;
    set_cp0_status(cp0_status);
}

static char read_uart_ch(void)
{
    char ch = 0;
    unsigned char *read_port = (unsigned char *)(0xbfe48000 + 0x00);
    unsigned char *stat_port = (unsigned char *)(0xbfe48000 + 0x05);

    while ((*stat_port & 0x01))
    {
        ch = *read_port;
    }
    return ch;
}
*/
void scanf(int *mem)
{
	//TODO:Use read_uart_ch() to complete scanf(), read input as a hex number.
	//Extending function parameters to (const char *fmt, ...) as printf is recommended but not required.
	char input_ch=0;
	int input_int[8];
    int i=7;
	int addr = 0;
	int addr_i =1;

    while(1){
    	if(input_ch!=0)
    	    printf("%c",input_ch);
    	if(input_ch=='x')
    		break;
    	input_ch=read_uart_ch();
    }
	while(1){
		input_ch =read_uart_ch();
        printf("%c",input_ch);
		if(input_ch=='\r')
		{
			break;
		}
		if(input_ch>='0'&&input_ch<='9'){
			input_int[i] = input_ch - '0';
			i--;
		}
		if(input_ch>='a'&&input_ch<='f'){
            input_int[i] = input_ch - 'a'+10;
            i--;
		}
		if(input_ch>='A'&&input_ch<='F'){
			input_int[i] = input_ch - 'A'+10;
			i--;
		}
		
	}
	for(i=0;i<8;i++){
		addr += addr_i*input_int[i];
		addr_i= addr_i*16;
    }

    //printk("\n");
    *mem = addr;
    //printk("0x%x",*mem);
	return;
}

void rw_task1(void)
{
	int mem1, mem2 = 0;
	int curs = 0;
	int memory[RW_TIMES];
	int i = 0;
	

	for(i = 0; i < RW_TIMES; i++)
	{
		sys_move_cursor(1, curs+i);
		mutex_lock_acquire(&screen_lock);
		scanf(&mem1);
		mutex_lock_release(&screen_lock);
		memory[i] = mem2 = rand();
		*(int *)mem1 = mem2;
		//printk("shdakdkahdak");

		sys_move_cursor(12,curs+i);
		printf("Write: 0x%x, %d", mem1, mem2);
	}
	curs = RW_TIMES;
	for(i = 0; i < RW_TIMES; i++)
	{
		sys_move_cursor(1, curs+i);
		mutex_lock_acquire(&screen_lock);
		scanf(&mem1);
		mutex_lock_release(&screen_lock);
		memory[i+RW_TIMES] = *(int *)mem1;

		sys_move_cursor(12,curs+i);
		if(memory[i+RW_TIMES] == memory[i])
			printf("Read succeed: %d", memory[i+RW_TIMES]);
		else
			printf("Read error: %d", memory[i+RW_TIMES]);
	}
	//enable_interrupt();
    while(1);
	//Only input address.
	//Achieving input r/w command is recommended but not required.
}
