#ifndef INCLUDE_MM_H_
#define INCLUDE_MM_H_

#include "type.h"

#define TLB_ENTRY_NUMBER 64
#define PTE_NUMBER 64

typedef enum {
    VALID,
    INVALID,
}flag_t;
typedef struct pte
{
	uint32_t ppn;
	uint32_t vpn;
    flag_t flag;
}pte_t;
typedef struct page_table
{
	pte_t pte[PTE_NUMBER];
	//pid_t pid;
}page_table_t;


extern page_table_t pagetable[16];

extern void TLBexception_handler_entry(void);
extern void TLBexception_handler_begin(void);
extern void TLBexception_handler_end(void);

extern void handle_TLB(void);
extern void tlb_fill(uint32_t cp0_entryhi,uint32_t cp0_entrylo0,uint32_t cp0_entrylo1);
void do_init_pagetable();
void do_TLB_Refill(uint32_t cp0_badvaddr);
void do_page_fault();

#endif
