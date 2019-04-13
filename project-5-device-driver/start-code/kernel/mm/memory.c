#include "mm.h"
#include "sched.h"

#define PAGE_SIZE 4096
#define PTABLE_NUM 16
page_table_t pagetable[PTABLE_NUM];
uint32_t ppage_num = 0;
//TODO:Finish memory management functions here refer to mm.h and add any functions you need.
void do_init_pagetable()
{
    uint32_t vpn = 0;
    uint32_t ppn = 4096;
    int i,j;
	
    for (i = 0; i < PTABLE_NUM; i++)
    {
    	//pagetable[i].pid = i;
    	for(j = 0; j < PTE_NUMBER;j++)
        {
    		pagetable[i].pte[j].vpn = vpn;
    		pagetable[i].pte[j].ppn = ppn;
    		pagetable[i].pte[j].flag = INVALID;
    		vpn ++;
    		ppn ++;
        }
        vpn = 0;
    }
    ppage_num = ppn;
   
}
void do_TLB_Refill(uint32_t cp0_badvaddr)
{
	int i = 0;
	pid_t pid = current_running->pid;
	uint32_t vpn = (cp0_badvaddr & 0xfffff000)>>12;
	uint32_t cp0_entryhi;
	uint32_t cp0_entrylo0;
	uint32_t cp0_entrylo1;

    cp0_entryhi = (((vpn/2)<<13)&0xffffe000)|(pid & 0x000000ff);
    (current_running->user_context).cp0_entryhi = cp0_entryhi;
	for(;i < PTE_NUMBER; i++){
        if(pagetable[pid].pte[i].vpn == vpn & (i%2 == 0)){
            cp0_entrylo0 = ((pagetable[pid].pte[i].ppn) <<6)|(0x00000016);
            cp0_entrylo1 = ((pagetable[pid].pte[i+1].ppn)<<6)|(0x00000016);
        	tlb_fill(cp0_entryhi,cp0_entrylo0,cp0_entrylo1);
        	return;
        } 
        if (pagetable[pid].pte[i].vpn == vpn & (i%2 == 1))
        {
            cp0_entrylo0 = ((pagetable[pid].pte[i].ppn) <<6)|(0x00000016);
            cp0_entrylo1 = ((pagetable[pid].pte[i-1].ppn)<<6)|(0x00000016);
        	tlb_fill(cp0_entryhi,cp0_entrylo0,cp0_entrylo1);
        	return;
        }
	}

    
	cp0_entrylo0 = (ppage_num <<6)|(0x00000016);
	ppage_num++;
    cp0_entrylo1 = ((ppage_num) <<6)|(0x00000016);
    ppage_num++;
    tlb_fill(cp0_entryhi,cp0_entrylo0,cp0_entrylo1);
    //tlb_fill(cp0_entryhi,cp0_entrylo0,cp0_entrylo1);
    return;
}
